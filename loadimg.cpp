#include <iostream>
#include <cstdio>
#include <vector>
#include <algorithm>
#include <string>
#include <cassert>
#include <cstdint>
#include <queue>
#include <limits>
#include <cfloat>
using namespace std;
#define STB_IMAGE_IMPLEMENTATION
#include "stb-image.h"

#define BF_TYPE 0x4D42             /* "MB" */
#define EPS (1.0/32)
#pragma pack(2)

struct  BITMAPFILEHEADER                      /**** BMP file header structure ****/
{
		unsigned short Type;           /* Magic number for file */
		unsigned int   Size;           /* Size of file */
		unsigned short Reserved1;      /* Reserved */
		unsigned short Reserved2;      /* ... */
		unsigned int   Offset;        /* Offset to bitmap data */
};
struct BITMAPINFOHEADER                       /**** BMP file info structure ****/
{
		unsigned int   Size;           /* Size of info header */
		int            Width;          /* Width of image */
		int            Height;         /* Height of image */
		unsigned short Planes;         /* Number of color planes */
		unsigned short BitCount;       /* Number of bits per pixel */
		unsigned int   Compression;    /* Type of compression to use */
		unsigned int   SizeImage;      /* Size of image data */
		int            XPelsPerMeter;  /* X pixels per meter */
		int            YPelsPerMeter;  /* Y pixels per meter */
		unsigned int   ClrUsed;        /* Number of colors used */
		unsigned int   ClrImportant;   /* Number of important colors */
};
#pragma pack()

void save_bmp(const char *filename,int w,int h,unsigned char *data,short bytesPerPixel=4){
	FILE *fp=fopen(filename,"wb");
	size_t hsize=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);
	size_t dsize=w*h*bytesPerPixel;
	BITMAPFILEHEADER file={BF_TYPE,(unsigned int)(hsize+dsize),0,0,(unsigned int)hsize};
	BITMAPINFOHEADER info={sizeof(BITMAPINFOHEADER),w,-h,0,(unsigned short)(8*bytesPerPixel),0,(unsigned int)dsize,1,1,0,0};
	fwrite(&file,sizeof(file),1,fp);
	fwrite(&info,sizeof(info),1,fp);
	fwrite(data,dsize,1,fp);
	fclose(fp);
}

class Color{
public:
	float r,g,b,a;
	Color(float _r=0,float _g=0,float _b=0,float _a=1.f){
		r=_r;g=_g;b=_b;a=_a;
	}
	Color(uint32_t bgra){
		decode(bgra);
	}
	void normalize(){
		if(r>1.f)r=1.f;if(r<0.f)r=0.f;
		if(g>1.f)g=1.f;if(g<0.f)g=0.f;
		if(b>1.f)b=1.f;if(b<0.f)b=0.f;
		if(a>1.f)a=1.f;if(a<0.f)a=0.f;
	}
	void print() const{
		printf("%d R, %d G, %d B, %d A\n",r,g,b,a);
	}
	void decode(uint32_t bgra){
		b=(bgra & 0x000000ff)/255.f;
		g=((bgra & 0x0000ff00)>>8)/255.f;
		r=((bgra & 0x00ff0000)>>16)/255.f;
		a=((bgra & 0xff000000)>>24)/255.f;
	}
	uint32_t encode(){
		normalize();
		uint32_t bgra=int(b*255)+(int(g*255)<<8)+(int(r*255)<<16)+(int(a*255)<<24);
		//printf("%x %f %x   ",int(g*255)<<8,g,bgra);
		return bgra;
	}
	Color operator *(const Color &_){
		return Color(r*_.r,g*_.g,b*_.b,a*_.a);
	}
	Color operator +(const Color &_){
		return Color(r+_.r,g+_.g,b+_.b,a+_.a);
	}
	Color operator *(const float &_){
		return Color(r*_,g*_,b*_,a*_);
	}
	bool operator ==(const Color &_)const{
		return (fabs(r-_.r)<=EPS && fabs(g-_.g)<=EPS && fabs(b-_.b)<=EPS && fabs(a-_.a)<=EPS);
	}
	bool operator <(const Color &_) const{
		if(*this==_)return false;
		if(r<_.r)return true;if(r>_.r)return false;
		if(g<_.g)return true;if(g>_.g)return false;
		if(b<_.b)return true;if(b>_.b)return false;
		if(a<_.a)return true;if(a>_.a)return false;
		return false;
	}
	Color& operator =(const Color &_){
		r=_.r;g=_.g;b=_.b;a=_.a;
		return *this;
	}
};
class Image{
public:
	int w,h;
	unsigned char *data;
	Image(){data=0;}
	~Image(){if(data)STBI_FREE(data);}
	void init(int _w,int _h){
		w=_w;
		h=_h;
		data=(unsigned char *)STBI_MALLOC(w*h*4);
		memset(data,0,w*h*4);
	}
	Color get(int x,int y){
		assert(x<w && y<h && x>=0 && y>=0);
		return Color(*(uint32_t*)(data+(y*w+x)*4));
	}
	void set(int x, int y, Color c){
		assert(x<w && y<h && x>=0 && y>=0);
		*(uint32_t*)(data+(y*w+x)*4) = c.encode();
	}
	void load(string file){
		puts(("Loading "+file+" ...").c_str());
		data = stbi_load(file.c_str(), &w, &h,0,4);
		assert(data && "Image load");
    unsigned char tmp;
    for(long i=0;i<w*h;i++)
    {
      tmp=data[i*4];
      data[i*4]=data[i*4+2];
      data[i*4+2]=tmp;
    }
		puts("Done");
	}
	void save(string file){
		puts(("Saving "+file+" ...").c_str());
		save_bmp(file.c_str(),w,h,data,4);
		puts("Done");
	}
};
int generateMaps(int argc,char** argv){
	if(argc<2)
	return 1;
	string base=argv[1];
	string names[]={"color","depth","disp","metal","smooth"};
	int num_manifolds=5;
	vector<Image*> maps;
	vector<Image*> finals;
	auto basemap= new Image();
	basemap->load(base+"-base.bmp");
	for(auto name: names){
		auto map= new Image();
		map->load(base+"-"+name+".bmp");
		maps.push_back(map);
		map= new Image();
		map->init(basemap->w,basemap->h);
		finals.push_back(map);
	}
	for (int y=0;y<basemap->h;y++){
		for (int x=0;x<basemap->w;x++){
			auto base=basemap->get(x,y);
			auto degree=1-base.r;
			for(int i=0;i<3;i++){
				auto manifold=maps[i]->get(maps[i]->w/2,degree*maps[i]->h);
				finals[i]->set(x,y,manifold);
			}
			auto metal=maps[3]->get(maps[3]->w/2,degree*maps[3]->h);
			auto smooth=maps[4]->get(maps[4]->w/2,degree*maps[4]->h);
			finals[3]->set(x,y,Color(metal.r,0,smooth.r,smooth.r));
			finals[4]->set(x,y,Color(metal.r,0,smooth.r,smooth.r));
		}
	}
	for(int i=0;i<num_manifolds;i++){
		finals[i]->save(base+"-out-"+names[i]+".bmp");
	}
	delete basemap;
	for(auto map:maps)
		delete map;
}

#include <unordered_map>
#include <map>

#define THRESHOLD (3e-2)
#define CONTROL_POINTS (8)
#define FLOAT_MAX 1e6

float dist(const Color& c1,const Color& c2){
	return sqrt((c1.r-c2.r)*(c1.r-c2.r) + (c1.g-c2.g)*(c1.g-c2.g) + (c1.b-c2.b)*(c1.b-c2.b) + (c1.a-c2.a)*(c1.a-c2.a));
}
#define mp make_pair
void dijkstra(int beg,int n,vector<vector<pair<float,int> > > &e,vector<float> &cur){
	priority_queue< pair<float,int>, vector<pair<float,int> >, greater< pair<float,int> > >  q;
	int node,i;
	float cost;
	q.push(mp(0.0,beg));
	cur[beg]=0;
	while(!q.empty()){
		cost=q.top().first;
		node=q.top().second;
		q.pop();
		if(cur[node]<cost)continue;
		for(i=0;i<e[node].size();i++){
			if(cur[e[node][i].second]>cost+e[node][i].first){
				cur[e[node][i].second]=cost+e[node][i].first;
				q.push(mp(cost+e[node][i].first,e[node][i].second));
			}
		}
	}
}

// void distribution(vector<float> v,float _max,int _nslots){
// 	float len = _max/_nslots;
// 	int dist[_nslots+1]={0};
// 	int val;
// 	for(int i=0;i<v;i++){
// 		val = int((v[i]*_nslots)/_max);
// 		if(val<_nslots){
// 			dist[val]++;
// 		}
// 		else{
// 			dist[_nslots]++;
// 		}
// 	}
// 	for(int i=0;i<_nslots;i++){
// 		cout<<i*len<<"-"<<(i+1)*len<<"==>"<<dist[i]<<endl;
// 	}
// 	cout<<">"<<(_max)<<"==>"<<dist[_nslots]<<endl;
// }

map<Color,int> color_map;									//Frequency of each color
vector<Color> colors;
int main(int argc, char **argv){
	if(argc<2)return 1;

	string name=argv[1];
	Image* map = new Image();
	map->load(name);
	puts("Dimestion of the image(width,height):");
	cout<<map->w<<" "<<map->h<<endl;


	puts("Getting the control points...");
	int x,y;
	float corrosion_value;
	vector<pair<int,float> > cv(0);  	//control point(color space) ,corrosion_value
	vector<pair<int,int> > cp(0);		//control_point(pixel space)
	for(int i=0;i<CONTROL_POINTS;i++){
		puts("Enter x y val");
		cin>>x>>y>>corrosion_value;
		cv.push_back(mp(0,corrosion_value));
		cp.push_back(mp(x,y));
	}
	puts("Control points done.");

	puts("Processing image. Obtaining color frequency...");
	color_map.clear();
	Color c(0,0,0,0);
	for(int i=0;i<map->w;i++){
		for(int j=0;j<map->h;j++){
			c = map->get(i,j);
			auto it = color_map.find(c);
			if(it==color_map.end()){
				color_map.insert(pair<Color,int>(c,1));
			}
			else{
				it->second++;
			}
		}
	}
	puts("Color frequency done.");

	puts("Getting unique color...");

	for(auto _maps : color_map){
		for(int i=0;i<CONTROL_POINTS;i++){
			if(_maps.first==map->get(cp[i].first,cp[i].second)) cv[i].first=colors.size();
		}
		colors.push_back(_maps.first);
	}
	cout<<"Number of colors="<<colors.size()<<endl;
	puts("Unique color done.");


	puts("Generating the graph...");
	vector<vector<pair<float,int> > > adj;
	int N = colors.size();
	float d;
	adj.resize(N,vector<pair<float,int> >(0));
	for(int i=0;i<N;i++){
		for(int j=i+1;j<N;j++){
			d = dist(colors[i],colors[j]);
			if(d<=THRESHOLD){
				adj[i].push_back(make_pair(d,j));
				adj[j].push_back(make_pair(d,i));
			}
		}
	}
	puts("Generatin graph done.");

	puts("Calculating the control point distance...");
	vector<vector<float> > cpd(0);				//control_point_distance
	for(int i=0;i<CONTROL_POINTS;i++){
		cpd.push_back(vector<float>(N+1,FLOAT_MAX));
		dijkstra(cv[i].first,N,adj,cpd[i]);

		for(int j=0;j<=N;j++){
			if(cpd[i][j]==FLOAT_MAX){
				cpd[i][j]=-1;
				continue;
			}
			if(cpd[i][j]>1e-6) cpd[i][j] = 1.0/cpd[i][j];
			else cpd[i][j] = FLOAT_MAX;
		}
	}
	puts("Done.");

	// Corrosion and the color map
	vector<pair<float,Color> > cor_color;
	float deno,num;
	int flag;
	for(int i=0;i<=N;i++){
		deno=0;num=0;flag=0;
		for(int j=0;j<CONTROL_POINTS;j++){
			if(cpd[j][i]>=0){
				flag=1;
				deno += cpd[j][i];
				num += cpd[j][i]*cv[j].second;
			}
		}
		if(flag){
			cor_color.push_back(mp(num/deno,colors[i]));
		}
	}

	int res[10]={0};
	for(int i=0;i<N;i++){
		if(cor_color[i].first<=1.0)
		res[int(cor_color[i].first*10)]++;
	}
	for(int i=0;i<10;i++)
		cout<<res[i]<<" ";
	cout<<endl;

	puts("Generating output...");
	Image* out = new Image();

	out->init(100,1000);
  vector<int> blanks(out->h,-1);
  int minh=out->h;
  Color last;
	for(int i=0;i<=N;i++){
    int h=int(cor_color[i].first*out->h);
    if(h>=out->h)
      h=out->h-1;
    if(h<0)
      continue;
    blanks[h]=1;
    if(minh>=h){
      minh=h;
      last=cor_color[i].second;
    }
		for(int j=0;j<out->w;j++)
      out->set(j,h,cor_color[i].second);
	}
  puts("Interpolating unknown corrosion appearances..");
  for(int i=0;i<out->h;i++)
  {
    if(blanks[i]==1)
      last=out->get(0,i);
    else
      for(int j=0;j<out->w;j++)
        out->set(j,i,last);
  }
	out->save("cor_out.bmp");
	puts("Done.");
	if(map)delete map;
	if(out)delete out;
}
