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
#include"PointTool.h"

void PointTool::Normalize3D(Point3d* inout){
  double dsize = this->Point3dSize(inout);
  if(dsize==0.0)dsize=1.0;
  inout->x /= dsize;
  inout->y /= dsize;
  inout->z /= dsize;
  

}
void PointTool::Normalize2D(Point2d* inout){
  double dsize = this->Point2DSize(inout);
  if(dsize==0.0)dsize=1.0;
  inout->x /= dsize;
  inout->y /= dsize;
  

}
double PointTool::getS(Point2d *v0,Point2d *v1,Point2d *v2){
  return (v1->x*v2->y-v1->y*v2->x - v0->x*(v2->y-v1->y) + v0->y*(v2->x-v1->x));
}
void PointTool::setC(double *out,Point2d *v1,Point2d *v2,Point2d *v3,Point2d *v4){
  double S123 = getS(v1,v2,v3);
  double S314 = getS(v3,v1,v4);
  double S432 = getS(v4,v3,v2);
  double S241 = getS(v2,v4,v1);
  double dist23 = sqrt((v2->x-v3->x)*(v2->x-v3->x) + (v2->y-v3->y)*(v2->y-v3->y));
  out[0] = (dist23)/S123;
  out[1] = -(dist23*S314)/(S123*S432);
  out[2] = -(dist23*S241)/(S123*S432);
  out[3] = (dist23)/S432;
  

}
double PointTool::Point2DSize(Point2d *dv){
  return sqrt((dv->x*dv->x+dv->y*dv->y));

}
double PointTool::Point2DSize(double d1x,double d1y,double d2x,double d2y){
  double ddx = d1x-d2x;
  double ddy = d1y-d2y;
  return sqrt((ddx*ddx+ddy*ddy));

}
double PointTool::Point2DSizeSq(double d1x,double d1y,double d2x,double d2y){
  double ddx = d1x-d2x;
  double ddy = d1y-d2y;
  return (ddx*ddx+ddy*ddy);

}
void PointTool::ScalarVector(Point3d *out,double dv,Point3d *in){
  out->x = dv*in->x;
  out->y = dv*in->y;
  out->z = dv*in->z;
  

}
void PointTool::setParametricDs(Point3d *out,Point3d *q1,Point3d *q2,Point3d *q3,double t1,double t2,double t3,double A){
  double dt1 = t2-t3;
  double dt2 = t3-t1;
  double dt3 = t1-t2;
  out->x = ((q1->x)*dt1 + (q2->x)*dt2 + (q3->x)*dt3)/(2.0*A);
  out->y = ((q1->y)*dt1 + (q2->y)*dt2 + (q3->y)*dt3)/(2.0*A);
  out->z = ((q1->z)*dt1 + (q2->z)*dt2 + (q3->z)*dt3)/(2.0*A);
  
}
void PointTool::setParametricDt(Point3d *out,Point3d *q1,Point3d *q2,Point3d *q3,double s1,double s2,double s3,double A){
  double ds1 = s3-s2;
  double ds2 = s1-s3;
  double ds3 = s2-s1;
  out->x = ((q1->x)*ds1 + (q2->x)*ds2 + (q3->x)*ds3)/(2.0*A);
  out->y = ((q1->y)*ds1 + (q2->y)*ds2 + (q3->y)*ds3)/(2.0*A);
  out->z = ((q1->z)*ds1 + (q2->z)*ds2 + (q3->z)*ds3)/(2.0*A);

}

double PointTool::getParametricA(double t1,double t2,double t3,double s1,double s2,double s3){
  double da = ((s2-s1)*(t3-t1)-(s3-s1)*(t2-t1))/2.0;
  if(da==0.0)return 1.0;
  return da;
}  
double PointTool::Distance(Point3d *in1,Point3d *in2){
  return sqrt(((((in1->x)-(in2->x))*((in1->x)-(in2->x)))
	       +(((in1->y)-(in2->y))*((in1->y)-(in2->y)))
	       +(((in1->z)-(in2->z))*((in1->z)-(in2->z)))));
		      
  }
void PointTool::makeVector(Point3d *out,Point3d *in1,Point3d *in2){
    out->x = ((in2->x) - (in1->x));
    out->y = ((in2->y) - (in1->y));
    out->z = ((in2->z) - (in1->z));
}
void PointTool::CrossVector(Point3d *out,Point3d *in1,Point3d *in2){
  out->x = ((in1->y)*(in2->z) - (in2->y)*(in1->z));
  out->y = ((in1->z)*(in2->x) - (in2->z)*(in1->x));
  out->z = ((in1->x)*(in2->y) - (in2->x)*(in1->y));
  }
double PointTool::InnerProduct(Point3d *in1,Point3d *in2){
    return ((in1->x)*(in2->x) + (in1->y)*(in2->y) + (in1->z)*(in2->z));
  }
double PointTool::InnerProduct(Point2d *in1,Point2d *in2){
    return ((in1->x)*(in2->x) + (in1->y)*(in2->y));
  }
void PointTool::setCenter(Point3d *out,Point3d *in1,Point3d *in2,Point3d *in3){
  out->x = (in1->x+in2->x+in3->x)/3.0;
  out->y = (in1->y+in2->y+in3->y)/3.0;
  out->z = (in1->z+in2->z+in3->z)/3.0;
  
}
double PointTool::Point3dSize(Point3d *in){
    return sqrt(((in->x)*(in->x)+(in->y)*(in->y)+(in->z)*(in->z)));
  }


int PointTool::getBraycentricC(Point3d *evaluation,Point3d *baryparam,Point3d *v1,Point3d *v2,Point3d *v3,Point3d **bc){
  double tA=0.0;
  double A1=0.0;
  double A2=0.0;
  double A3=0.0;
  
  this->makeVector(bc[0],evaluation,v1);
  this->makeVector(bc[1],evaluation,v2);
  this->makeVector(bc[2],evaluation,v3);
  
  this->CrossVector(bc[3],bc[0],bc[1]);
  A3 = this->Point3dSize(bc[3]);
  this->CrossVector(bc[3],bc[0],bc[2]);
  A2 = this->Point3dSize(bc[3]);
  this->CrossVector(bc[3],bc[1],bc[2]);
  A1 = this->Point3dSize(bc[3]);
  tA = A1+A2+A3;if(tA==0.0)tA=1.0;
  baryparam->x = A1/tA;
  baryparam->y = A2/tA;
  baryparam->z = A3/tA;
    
  if((baryparam->x >= 0.0) && (baryparam->y >=0.0) && (baryparam->z >=0.0)){
    return 1;
  
    }else{
      return 0;
    }
  return 0;

}


  
double PointTool::getArea(Point3d* dv1,Point3d* dv2,Point3d* dv3){
  makeVector(ddv1,dv2,dv1);
  makeVector(ddv2,dv3,dv1);
  CrossVector(ddv3,ddv1,ddv2);
  return (Point3dSize(ddv3)/2.0);

}
