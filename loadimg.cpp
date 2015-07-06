#include <cstdio>
#include <vector>
#include <algorithm>
#include <string>
#include <cassert>
#include <cstdint>
using namespace std;
#define STB_IMAGE_IMPLEMENTATION
#include "stb-image.h"

#define BF_TYPE 0x4D42             /* "MB" */
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

void save_bmp(const char *filename,int w,int h,unsigned char *data,int bytesPerPixel=4){
	FILE *fp=fopen(filename,"wb");
	size_t hsize=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);
	size_t dsize=w*h*bytesPerPixel;
	BITMAPFILEHEADER file={BF_TYPE,hsize+dsize,0,0,hsize};
	BITMAPINFOHEADER info={sizeof(BITMAPINFOHEADER),w,-h,0,8*bytesPerPixel,0,dsize,1,1,0,0};
	fwrite(&file,sizeof(file),1,fp);
	fwrite(&info,sizeof(info),1,fp);
	fwrite(data,dsize,1,fp);
	fclose(fp);
}

class Color{
public:
  float r,g,b,a;
  Color(float _r,float _g,float _b,float _a=1.f){
    r=_r;g=_g;b=_b;a=_a;
  }
  Color(uint32_t brga){
    decode(brga);
  }
  void normalize(){
    if(r>1.f)r=1.f;if(r<0.f)r=0.f;
    if(g>1.f)g=1.f;if(g<0.f)g=0.f;
    if(b>1.f)b=1.f;if(b<0.f)b=0.f;
    if(a>1.f)a=1.f;if(a<0.f)a=0.f;
  }
  void decode(uint32_t brga){
    r=(brga & 0x000000ff)/255.f;
    g=((brga & 0x0000ff00)>>8)/255.f;
    b=((brga & 0x00ff0000)>>16)/255.f;
    a=((brga & 0xff000000)>>24)/255.f;
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
    puts("Done");
  }
  void save(string file){
    puts(("Saving "+file+" ...").c_str());
    save_bmp(file.c_str(),w,h,data,4);
    puts("Done");
  }
};
using namespace std;
int main(int argc, char **argv){
	if(argc<2)
		return 1;
  string base=argv[1];
  string names[]={"color","depth","disp","metal","smooth"};
  string final_names[]={"color","depth","disp","metal"};
  int num_manifolds=5,num_finals=4;
  vector<Image*> maps;
  vector<Image*> finals;
  auto basemap= new Image();
  auto noisemap = new Image();
  noisemap->load(base+"-noise.bmp");
  basemap->load(base+"-base.bmp");
  for(auto name: names){
    auto map= new Image();
    map->load(base+"-"+name+".bmp");
    maps.push_back(map);
  }
  for(auto name: final_names){
    auto map= new Image();
    map->init(basemap->w,basemap->h);
    finals.push_back(map);
  }
	for (int y=0;y<basemap->h;y++){
		for (int x=0;x<basemap->w;x++){
      auto base=basemap->get(x,y);
      auto degree=1-base.r;
      degree = degree * noisemap->get(x,y).r * 1.6;
      if(degree>1.f)degree=1.f;
      if(degree<0.f)degree=0;
      for(int i=0;i<3;i++){
        auto manifold=maps[i]->get(maps[i]->w/2,degree*(maps[i]->h-1));
        finals[i]->set(x,y,manifold);
      }
      auto metal=maps[3]->get(maps[3]->w/2,degree*(maps[3]->h-1));
      auto smooth=maps[4]->get(maps[4]->w/2,degree*(maps[4]->h-1));
      finals[3]->set(x,y,Color(metal.r,0,smooth.r,smooth.r));
		}
	}
  for(int i=0;i<num_finals;i++){
    finals[i]->save(base+"-out-"+final_names[i]+".bmp");
  }
  delete basemap;
	for(auto map:maps)
    delete map;
}