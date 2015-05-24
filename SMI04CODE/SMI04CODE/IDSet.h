
/*
A fast and simple stretch-minimizing mesh parameterization C++ code
Copyright:(c) Shin Yoshizawa, 2008
E-mail: shin@riken.jp


 All right is reserved by Shin Yoshizawa.
This C++ sources are allowed for only primary user of 
research and educational purposes. Don't use secondary: copy, distribution, 
diversion, business purpose, and etc.. 
 */

class IDSet{
  
 public:
  IDSet(){}
  virtual ~IDSet(){}
  void AppendVF(int,IDList *);
  int SearchI(int dID,IDList *dIHead,IDList *dITail);
 
  
  void AppendI(int dID,IDList *dIHead,IDList *dITail,int nowID,int *dnum);
  void AppendVFSort(int dID,IDList *dIHead,IDList *dITail);
  void AppendIF(int dID,IDList *dIHead,IDList *dITail,int nowID,int *dnum);
  void AppendISort(int dID,IDList *dIHead,IDList *dITail,int nowID,int *dnum);
  void AppendISort(int dID,IDList *dIHead,IDList *dITail);
 
  
  void AppendPolarI(int dID,PolarList *dITail,double dx,double dy);
   void AppendPolarI(int dID,PolarList *dITail,double dx,double dy,double dz,double cotw);
   
 
   void AppendPolarIRL(int dID,PolarList *dITail,double dr,double dthta,double dlambda,int tR,int tL,int nextP);
  void CleanNeighborLPolar(PolarList **,PolarList **,int );
 
  void CleanNeighbor(IDList*,IDList*);
  void CleanNeighborPolar(PolarList*,PolarList*);
  
  void CleanNeighborLL(IDList **,IDList **,int ,int *);
  void CleanNeighborL(IDList **,IDList **,int );
  
  void Clean(IDList **dFHead,IDList **dFTail,int numberSV,int *dneighborN);
 private:
  IDSet(const IDSet& rhs);
  const IDSet &operator=(const IDSet& rhs);

};


