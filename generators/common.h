#pragma once
#include <string>
#include <cstdio>
#include <cassert>
#define EPS (1.0/32)

#define STB_IMAGE_IMPLEMENTATION
#include "stb-image.h"

#define BF_TYPE 0x4D42             /* "MB" */
using namespace std;

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
  BITMAPINFOHEADER info={sizeof(BITMAPINFOHEADER),w,h,1,(unsigned short)(8*bytesPerPixel),0,(unsigned int)dsize,1,1,0,0};
  fwrite(&file,sizeof(file),1,fp);
  fwrite(&info,sizeof(info),1,fp);
  //reverse the row order
  size_t row_size=w*bytesPerPixel;
  for(int i=h-1;i>=0;i--){
   fwrite(data+i*row_size,row_size,1,fp);
  }
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
    printf("%f R, %f G, %f B, %f A\n",r,g,b,a);
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
  Color operator /(int k){
    return Color(r/k,g/k,b/k,a/k);
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