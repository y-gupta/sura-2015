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
#include <unordered_map>
#include <map>
#include "common.h"
using namespace std;

#define THRESHOLD (3e-2)
#define THRESHOLD2 (3e-1)
#define CONTROL_POINTS (10)
#define FLOAT_MAX (1e9)
#define mp make_pair
/**
 * @brief Distance between two colors
 * @details Used euclidean distance. Can be modified to taxicab(N-4) distance or N-8 distance
 */
float dist(const Color& c1,const Color& c2){
	return sqrt((c1.r-c2.r)*(c1.r-c2.r) + (c1.g-c2.g)*(c1.g-c2.g) + (c1.b-c2.b)*(c1.b-c2.b));
}

/**
 * @brief Dijkstra shortest path
 * @details Calculates the shortest path from source vertex to all vertices in graph
 * 
 * @param beg Source vertex
 * @param n Total number of vertics in graph
 * @param cur cur[i] represent path length to reach vertex i at some point in algorithm. Finally it contains shortest path.
 * @param e Adjanceny of graph. e[i] contains list of all neighbours along with edge cost
 */
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
/**
 * color_map[i] - Represent color with its frequency in the image. This gives us information about histogram of the image.
 */
map<Color,int> color_map;
/**
 * colors - Represent unique color of image. We here do not consider frequency of colors.
 */
vector<Color> colors;
vector<vector<pair<float,int> > > adj;
int main(int argc, char **argv){
	if(argc<2){
		puts("Usage: <executable> <file_name> <file_name_segment>");
		return 1;
	}
	
	string name(argv[1]);
	Image* map = new Image();
	map->load(name);
	puts("Dimestion of the image(width,height):");
	cout<<map->w<<" "<<map->h<<endl;


	puts("Getting the control points...");
	int x,y;
	float corrosion_value;
	/**
	 * cv represents control point in color space.
	 * cv[i] - (idx,val) where idx = index/position of color in unique color array, val = corrosion value
	 * We thus get color information corresponding to corrosion value.
	 */
	vector<pair<int,float> > cv(0);
	/**
	 *	cp represents control points in the pixel space.
	 *	cp[i] - (x,y) where x,y are coordinates of control points entered by user
	 */
	vector<pair<int,int> > cp(0);
	for(int i=0;i<CONTROL_POINTS;i++){
		puts("Enter x y val:");
		cin>>x>>y;
		cin>>corrosion_value;
		cv.push_back(mp(0,corrosion_value));
		cp.push_back(mp(x,y));
	}
	puts("Control points done.");

	
	/**
	 * @brief Histogram processing of the image
	 * @details Extracting unique color from the image and number of pixel corresponding to each color.
	 */
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

	/**
	 * Obtain unique color neglecting frequency.
	 * Also corresponding to each control point marked by the user, find corrosion value. This gives us rough information of which color represents what degree of corrosion.(Entirely based on user interaction. We are not extracting this information from image.)
   */
	puts("Getting unique color...");
	for(auto _maps : color_map){
		for(int i=0;i<CONTROL_POINTS;i++){
			if(_maps.first==map->get(cp[i].first,cp[i].second)) cv[i].first=colors.size();
		}
		colors.push_back(_maps.first);
	}
	cout<<"Number of colors="<<colors.size()<<endl;
	puts("Unique color done.");


	/**
	 * Generating the graph
	 * For each color c, we calculate distance of all the colors from c. Only those color whose value is less then some threshold, we consider it to be neighbour of c. This way adjanceny list of graph is generated.
	 */
	puts("Generating the graph...");
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

	/**
	 * Control point Distance
	 * Now for each of control point marked by the user, we apply dijkstra algorithm to find shortest path considering control point as source. Thus we evaluate inverse distance for each control point.
	 */
	puts("Calculating the control point distance...");
	vector<vector<float> > cpd(0);
	for(int i=0;i<CONTROL_POINTS;i++){
		cpd.push_back(vector<float>(N+1,FLOAT_MAX));
		dijkstra(cv[i].first,N,adj,cpd[i]);

		for(int j=0;j<=N;j++){
			if(cpd[i][j]==FLOAT_MAX){
				cpd[i][j]=-1;
				continue;
			}
			if(cpd[i][j]>1e-6) cpd[i][j] = 1.0/cpd[i][j];
			else{
				cout<<"FLOAT_MAX"<<endl;
				cpd[i][j] = FLOAT_MAX;
				cout<<i<<" "<<j<<endl;
			}
		}
	}
	puts("Done.");

	/**
	 * Distributing the corrosion in the graph
	 * Now from each of control point, we leak to corrosion value along the graph. Leakage is directly proportional to the inverse distance caluculated with dijkstra algorithm. This way we get corrosion value for all the vertices.
	 * Also using this, we get mapping from corrosion value to color.
	 */
	vector<pair<float,Color> > cor_color;
	float max_cor_value=0;
	float deno,num;
	int flag;
	for(int i=0;i<N;i++){
		deno=0;num=0;flag=0;
		for(int j=0;j<CONTROL_POINTS;j++){
			if(cpd[j][i]>=0){
				if(flag!=2)
				flag=1;
				if(cpd[j][i]==FLOAT_MAX){
					flag=2;
					cout<<j<<" "<<i<<endl;
				}
				deno += cpd[j][i];
				num += cpd[j][i]*cv[j].second;

			}
		}
		if(flag){
			if(flag==2){
				printf("%f\n",num/deno);
			}
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

	/**
	 * Generating output
	 * Now for each vertex in the graph, we have corrosion value calculated from control points. Using this corrosion value, and color of corresponding pixel, we generate spectrum of corrosion appearance. This shows appearance at each corrosion degree value.
	 */
	puts("Generating output...");
	Image* out = new Image();

	out->init(100,1000);
  vector<int> blanks(out->h,0);
  int minh=out->h;
  Color last;
	for(int i=0;i<N;i++){
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
        float last_w=float(next_h-i+1)/(next_h-last_h-1);
        float next_w=1-last_w;
        // tot=1.f/(last_w+next_w);
        // last_w*=tot;
        // next_w*=tot;
        if(no_next)
          last_w=1,next_w=0;
        for(int j=0;j<out->w;j++)
          // out->set(j,i,Color(0,0,0));
          out->set(j,i,last*last_w+next*next_w);
          // out->set(j,i,Color(1,1,1)*last_w);
      }
      i--;
    }
  }
  
  std::map<Color,float> inverse_map;
  for(int i=0;i<out->h;i++){
  	c=out->get((out->w)>>1,i);
  	inverse_map.insert(mp(c,float(i)/out->h));
  }
  cout<<"Done.";

  Image* wmap = new Image();
	wmap->load(name);
	for(int i=0;i<map->w;i++){
		for(int j=0;j<map->h;j++){
			c = map->get(i,j);
			auto it = inverse_map.lower_bound(c),it1=it;
			if(it!=inverse_map.begin())
			it1 = prev(it);
			if(it!=inverse_map.end() && dist(c,it->first)<=THRESHOLD2){
				Color tmp(it->second,it->second,it->second);
				wmap->set(i,j,tmp);
			}
			else if(it1!=inverse_map.end() && dist(c,it1->first)<=THRESHOLD2){
				Color tmp(it1->second,it1->second,it1->second);
				wmap->set(i,j,tmp);
			}
			else{
				wmap->set(i,j,Color(1,0,0));
			}
		}
	}
	wmap->save("weathering_map.bmp");
	if(wmap)delete wmap;

	out->save("cor_out.bmp");
	puts("Done.");
	if(map)delete map;
	if(out)delete out;
}
