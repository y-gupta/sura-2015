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
	
	Image out1;
	out1.init(in.w/2,in.h/2);
	Image out2;
	out2.init(in.w/2,in.h/2);
	Image out3;
	out3.init(in.w<<1,in.h<<1);
	Image out4;
	out4.init(in.w<<1,in.h<<1);

	scaleDownDecimation(in,out1);
	scaleDownSkip(in,out2);

	scaleUpInterpolation(in,out3);
	scaleUpReplication(in,out4);

	
	out1.save("smallDecimation.bmp");
	out2.save("smallSkip.bmp");
	out3.save("bigInterpolation.bmp");
	out4.save("bigReplication.bmp");
	puts("Images saved");
	return 0;
}