#include "common.h"
#include <iostream>
#include <algorithm>

using namespace std;

void scaleDownDecimation(Image& in,Image& out){
	puts("Scaling down...");
	Color c;
	for(int i=0;i<out.h;i++){
		for(int j=0;j<out.w;j++){
			c=(in.get(i<<1,j<<1)+in.get((i<<1) + 1,j<<1)+in.get(i<<1,(j<<1) + 1)+in.get((i<<1) + 1,(j<<1) + 1))/4;
			out.set(i,j,c);
		}
	}
	puts("Scaling down done");
}

void scaleDownSkip(Image& in,Image& out){
	puts("Scaling down...");
	for(int i=0;i<out.h;i++){
		for(int j=0;j<out.w;j++){
			out.set(i,j,in.get(i<<1,j<<1));
		}
	}
	puts("Scaling down done");
}

void scaleUpInterpolation(Image& in,Image& out){
	puts("Scaling up...");

	puts("Interpolating one dimension...");
	for(int i=0;i<out.h;i+=2){
		for(int j=0;j<out.w;j++){
			out.set(i,j,in.get(i>>1,j>>1));
			j++;
			if(j==out.w-1)
				out.set(i,j,out.get(i,j-1));
			else
				out.set(i,j,(in.get(i>>1,j>>1)+in.get(i>>1,(j>>1)+1))/2);
		}
	}
	puts("Done");
	puts("Interpolating second dimension...");
	for(int i=1;i<out.h-1;i+=2){
		for(int j=0;j<out.w;j++){
			out.set(i,j,(out.get(i-1,j)+out.get(i+1,j))/2);
		}
	}
	for(int j=0,i=out.h-1;j<out.w;j++){
		out.set(i,j,out.get(i-1,j));
	}
	puts("Done");
	puts("Scaling up done");
}

void scaleUpReplication(Image& in,Image& out){
	puts("Scaling up...");
	puts("Replicating one dimension...");
	for(int i=0;i<out.h;i+=2){
		for(int j=0;j<out.w;j+=2){
			out.set(i,j,in.get(i>>1,j>>1));
			out.set(i,j+1,in.get(i>>1,j>>1));
		}
	}
	puts("Done");
	puts("Replicating second dimension...");
	for(int i=1;i<out.h-1;i+=2){
		for(int j=0;j<out.w;j++){
			out.set(i,j,out.get(i-1,j));
		}
	}
	for(int j=0,i=out.h-1;j<out.w;j++){
		out.set(i,j,out.get(i-1,j));
	}
	puts("Done");
	puts("Scaling up done");
}

int main(int argc, char** argv){
	if(argc<2){
		puts("Usage: a <name-of-image>");
		return 1;
	}
	string name(argv[1]);
	Image in;
	in.load(name);

	Image out3;
	out3.init(in.w<<1,in.h<<1);
	scaleUpInterpolation(in,out3);


	out3.save("bigInterpolation.bmp");
	puts("Images saved");
	return 0;
}