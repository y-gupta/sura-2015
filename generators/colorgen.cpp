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
#include "common.h"
#include <unordered_map>
#include <map>

using namespace std;

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
	N = cor_color.size();

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
  vector<int> blanks(out->h,0);
  int minh=out->h;
  Color last;
	for(int i=0;i<=N;i++){
    int h=int(cor_color[i].first*out->h);
    if(h>=out->h)
      h=out->h-1;
    if(h<0)
      continue;
    if(minh>=h){
      minh=h;
      last=cor_color[i].second;
    }
    auto avg=out->get(0,h)*blanks[h]+cor_color[i].second;
    blanks[h]++;
    avg = avg * (1.f/blanks[h]);
		for(int j=0;j<out->w;j++)
      out->set(j,h,avg);
	}
  puts("Interpolating unknown corrosion appearances..");
  int last_h=0,next_h=0;
  Color next;
  for(int i=0;i<out->h;i++)
  {
    if(blanks[i]!=0)
    {
      last=out->get(0,i);
      last_h=i;
    }
    else{
      bool no_next=true;
      for(int j=i+1;j<out->h;j++){
        if(blanks[j]!=0){
          next_h=j;
          next=out->get(0,j);
          no_next=false;
          break;
        }
      }
      if(no_next)
        next_h=out->h-1;
      for(;i<next_h;i++){
        float last_w=1.f/(i-last_h),next_w=1.f/(next_h-i),tot;
        tot=1.f/(last_w+next_w);
        last_w*=tot;
        next_w*=tot;
        if(no_next)
          last_w=1,next_w=0;
        for(int j=0;j<out->w;j++)
          out->set(j,i,last*last_w+next*next_w);
      }
    }
  }
	out->save("cor_out.bmp");
	puts("Done.");
	if(map)delete map;
	if(out)delete out;
}
