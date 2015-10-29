/** \file*/
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
using namespace std;

int main(int argc, char **argv){
	if(argc<2){
		puts("Usage: <executable name> <filename> [<alpha>]");
		return 1;
	}
  float alpha=0;
  if(argc>=3){
    sscanf(argv[3],"%f",&alpha);
    cout<<"Using alpha = "<<alpha<<endl;
  }
	/**
	 * For any given base, we need to have base-color, base-depth, base-disp, base-metal, base-smooth, base-noise.
	 * Using the above six gradients, we generate corresponding maps correlating to input weathering degree map.
	 */
	string base=argv[1];
	string names[]={"color","depth","disp","metal","smooth","noise"};
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
      if(fabs(base.r-base.g) > EPS || fabs(base.r-base.b) > EPS || fabs(base.b-base.g) > EPS){
        finals[0]->set(x,y,base);
        for(int i=1;i<5;i++)
          finals[i]->set(x,y,Color(0.5,0.5,0.5,0.5));
        continue;
      }
			auto degree=base.r;
      degree = ((1-alpha)+maps[5]->get(x,y).r*alpha*2)*degree;
      if(degree>1)
        degree=1;
      if(degree<0)
        degree=0;
      basemap->set(x,y,Color(1,1,1)*degree);
      float h;
			for(int i=0;i<3;i++){
        h=degree*maps[i]->h;
        if(h==maps[i]->h)
          h--;
				auto manifold=maps[i]->get(maps[i]->w/2,h);
        if(i==1){
          manifold=Color(1,1,1)+manifold*-1;
        }
				finals[i]->set(x,y,manifold);
			}
      h=degree*maps[3]->h;
      if(h==maps[3]->h)h--;
			auto metal=maps[3]->get(maps[3]->w/2,h);
      h=degree*maps[4]->h;
      if(h==maps[4]->h)h--;
			auto smooth=maps[4]->get(maps[4]->w/2,h);
			finals[3]->set(x,y,Color(metal.r,0,smooth.r,smooth.r));
			finals[4]->set(x,y,Color(metal.r,0,smooth.r,smooth.r));
		}
	}
	for(int i=0;i<num_manifolds;i++){
		finals[i]->save(base+"-out-"+names[i]+".bmp");
	}
  basemap->save(base+"-out-base.bmp");
	delete basemap;
	for(auto map:maps)
		delete map;
}
