/*
A fast and simple stretch-minimizing mesh parameterization C++ code
Copyright:(c) Shin Yoshizawa, 2004
E-mail: shin.yoshizawa@mpi-sb.mpg.de
URL: http://www.mpi-sb.mpg.de/~shin
Affiliation: Max-Planck-Institut fuer Informatik: Computer Graphics Group 
 Stuhlsatzenhausweg 85, 66123 Saarbruecken, Germany
 Phone +49 681 9325-408 Fax +49 681 9325-499 

 All right is reserved by Shin Yoshizawa.
This C++ sources are allowed for only primary user of 
research and educational purposes. Don't use secondary: copy, distribution, 
diversion, business purpose, and etc.. 
 */
#include<stdio.h>
#include<math.h>
#include"Point3d.h"
#include"Point2d.h"
#include"IDList.h"
#include"PointTool.h"
#include"PolarList.h"
#include"IDSet.h"
#include"PCBCGSolver.h"
#include"Polyhedron.h"
// Main 
int main(int argc,char *argv[]){
  
  if(argv[1]==NULL)return 0;
  Polyhedron *mymesh = new Polyhedron();
  mymesh->readmesh(argv[1]);

  mymesh->param();
  printf("reached here\n");
  if(argv[2]==NULL){
    delete mymesh;
    return 0;
  }
  mymesh->writemesh(argv[2]);
  delete mymesh;
  return 0;
}

