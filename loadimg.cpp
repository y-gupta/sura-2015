#include <cstdio>
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

int main(int argc, char **argv){
  if(argc<2)
    return 1;
  puts("Loading image...");
  int w,h;
  unsigned char *data = stbi_load(argv[1], &w, &h,0,4);
  puts("Loaded image. Saving...");
  save_bmp("output.bmp",w,h,data,4);
  puts("Done");
  stbi_image_free(data);
}