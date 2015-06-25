#include <cstdio>
#include <vector>
#include <algorithm>
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

void invertColor(unsigned char* data,int w,int h){
	for (int y=0;y<h;y++){
		for (int x=0;x<w;x++){
			int idx = (w*y + x)*4;
			//Inverting Color(max=255 as 8 bit unsigned)
			data[idx] = 255-data[idx];
			data[idx+1] = 255-data[idx+1];
			data[idx+2] = 255-data[idx+2];

		}
	}
}

vector<char> getColor(unsigned char *data,int w,int h,float percent){
	vector<char> ret(0);
	int y=(h*(100-percent))/100;
	int idx = w*y*4;

	ret.push_back(data[idx]);
	ret.push_back(data[idx+1]);
	ret.push_back(data[idx+2]);
	ret.push_back(data[idx+3]);
	return ret;
}

void applyColor(unsigned char *data,int w,int h){
	puts("Loading color image...");
	int w1,h1;
	unsigned char *color_image_data = stbi_load("color.bmp",&w1,&h1,0,4);
	puts("Color image done.");

	for (int y=0;y<h;y++){
		for (int x=0;x<w;x++){
			int idx = (w*y + x)*4;
			vector<char> color = getColor(color_image_data,w1,h1,(data[idx+2]*100.0)/255.0);
			data[idx] = color[2];
			data[idx+1] = color[1];
			data[idx+2] = color[0];
		}
	}
	stbi_image_free(color_image_data);
}

void generateMetallicMap(unsigned char *data,int w,int h,char** argv){
	// applyColor(data,w,h);
	puts("Loading input image...");
	int w1,h1;
	unsigned char *data1 = stbi_load(argv[2], &w1, &h1,0,4);
	puts("Done.");

	puts("Loading input image...");
	int w2,h2;
	unsigned char *data2 = stbi_load(argv[3], &w2, &h2,0,4);
	puts("Done.");

	for (int y=0;y<h;y++){
		for (int x=0;x<w;x++){
			int idx = (w*y + x)*4;
			vector<char> v1 = getColor(data1,w1,h1,(data[idx+2]*100.0)/255.0);
			vector<char> v2 = getColor(data2,w2,h2,(data[idx+2]*100.0)/255.0);

			data[idx] = v1[0];
			data[idx+1] = 10;
			data[idx+2] = v2[0];
			data[idx+3] = v1[0];
		}
	}
	stbi_image_free(data1);
	stbi_image_free(data2);
}

int main(int argc, char **argv){
	if(argc<2)
		return 1;
	puts("Loading input image...");
	int w,h;
	unsigned char *data = stbi_load(argv[1], &w, &h,0,4);
	puts("Done.");

	
	generateMetallicMap(data,w,h,argv);

	puts("Saving...");
	save_bmp("output.bmp",w,h,data,4);
	puts("Done");
	stbi_image_free(data);
}