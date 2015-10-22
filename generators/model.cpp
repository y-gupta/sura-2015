#include "common.h"
#include <iostream>
#include <random>
#include <algorithm>
using namespace std;
int main(int argc,char **argv){
  srand(NULL);
  for(int t=1;t<=100;t++){
    double r=float(rand())/RAND_MAX,g=float(rand())/RAND_MAX,b=float(rand())/RAND_MAX;
    Color c(r,g,b);
    cout<<"CASE "<<t<<endl;
    cout<<r<<" "<<g<<" "<<b<<endl;
    c.print();
    c.convertHSVtoRGB();
    c.print();
    cout<<endl;
  }
  return 0;
}