/*
A fast and simple stretch-minimizing mesh parameterization C++ code
Copyright:(c) Shin Yoshizawa, 2008
E-mail: shin@riken.jp

 All right is reserved by Shin Yoshizawa.
This C++ sources are allowed for only primary user of 
research and educational purposes. Don't use secondary: copy, distribution, 
diversion, business purpose, and etc.. 
 */
#define PI 3.1415926535897932385
#define next(p) p->next
#define back(p) p->back
// parameterization class
class Polyhedron{
 public:
  int numberV;
  int numberF;
  int boundarytype;
  /* 
     0: unit square
     1: unit circle
     2: natural boundary of the intrinsic parameterization
  */
  int paramtype;
  /* 
     1: U1 parameterization: single optimization step
     2: Uopt parameterization: optimal parameterization
  */
  int weighttype;
  /*
    0: Shape Preserving: Floater CAGD 1997.
    1: w = 1: Tutte 1963. 
    2: Harmonic Map: Eck et al. SIGGRAPH 1995.
    3: Intrinsic Map: Desbrun et al. EUROGRAPHICS 2002.
       ((1-intrinsiclambda)*Chi + intrinsiclambda*Authalic)
       If intrinsiclambda=1 then it is equivalent to Eck's map
    4: Mean value: Floater CAGD 2003.

   */
  double intrinsiclambda;
  
  int smooth;
  /* 
     For the optimal low stretch parameterization,
     1: fast and aggressive
     2: smooth: if you will get a problem (unstability) using 1 then it is better to use 2.
  */  
  Point3d **point;
  int pickID;/* first boundary point ID: it will be conner of the unit square. 
		if pickID=-1 then pickID will be the first boundary vertex ID.
	      */

  int boundarysigma;/* When we test current stretch distortion,
		       0 without boundary triangles,
		       1 with boundary triangles
		    */
  int level;  

  /* Parameter Mesh Coordinates */
  double *pU;
  double *pV;
  /*******************************/

  int **Face;
  double *areaMap3D;
  double sumarea3D;
  double constsumarea3D;
  
  double *sigma;
  double *E;
  double *G;
  
  IDSet *IDtool;
  PointTool *PT;
  /*************************/
  
  // 1-ring neighbor structure
  PolarList **PHead;
  PolarList **PTail;
  IDList **IHead;
  IDList **ITail;
  IDList **VHead;
  IDList **VTail;
  IDList **FHead;
  IDList **FTail;
  int *boundary;
  int *neighborI;
  int *neighborF;
  
  int numboundary;
  Point3d **bc;
  Point3d **dbc;  

  double totalarea3D;
  double totalarea2D;
  double gammaP;/* transfer parameter: eta in the paper */

  int iteNum; /* Maximum iteration number in PCBCG */
  double PCBCGerror;/* Error threshold in PCBCG */
  
  
  
  Polyhedron(){
    // Default setting
    pickID=-1;
    paramtype=2;
    boundarytype=1;
    weighttype=2;
    iteNum=2000;
    gammaP=1.0;
    PCBCGerror=pow(0.1,6.0);
    smooth=1;
    intrinsiclambda=0.5;
    boundarysigma=1;
    PT=new PointTool();
  }
  virtual ~Polyhedron(){
    if(numberV!=0&&point!=NULL){
      memorydelete();
    }
  }
  // parameterization
  void param(){
    //boundary mapping 
    
    BoundaryMap();
    

    if(boundarytype==2){
      setNaturalB(iteNum,PCBCGerror); 
      //make local parameterization via polar map 
      setPolarMap(); 
    }else{
      //make local parameterization via polar map 
      setPolarMap(); 
      // for (int i = 0; i < numberV; ++i)
      // {
      // 	printf (" %lf \n",pU[i]);
      // }
    }
    
    if(paramtype==1){
      ParametrizationSingle(iteNum,PCBCGerror);
      

    }else{
      if(smooth==1){ 
	ParametrizationOptimal(iteNum,PCBCGerror);
      }else{ 
	ParametrizationSmoothOptimal(iteNum,PCBCGerror);
      }
    }
     
  }

  
  void readmesh(char *filename){
    
    FILE *in=NULL;
    in = fopen(filename,"r");
    int dV=0;
    int dF=0;
    int i,j;
    int di=0;
    int dj=0;
    int dk=0;
    double dx=0.0;
    double dy=0.0;
    double dz=0.0;
    size_t ssize = fscanf(in,"%d",&dV);
    ssize = fscanf(in,"%d",&dF);
    
    memoryallocate(dV,dF);
    for(i=0;i<numberV;i++){
      ssize =fscanf(in,"%lf %lf %lf",&dx,&dy,&dz);
      setPoint(i,dx,dy,dz);
    }
    int val=3;
    for(i=0;i<numberF;i++){
      ssize = fscanf(in,"%d %d %d %d",&val,&di,&dj,&dk);
      setFace(i,di,dj,dk);
      IDtool->AppendVFSort(i,FHead[Face[i][0]],FTail[Face[i][0]]);
      IDtool->AppendVFSort(i,FHead[Face[i][1]],FTail[Face[i][1]]);
      IDtool->AppendVFSort(i,FHead[Face[i][2]],FTail[Face[i][2]]);
    }
    
    
    
    fclose(in);
    
    /* feature analysis */
    
    SetBoundaryLines();
    setAreaMap3D();

  }
  void writemesh(char *filename){
    int i=0;
    FILE *out = fopen(filename,"w");
   // fprintf(out,"%d\n",numberV);
   // fprintf(out,"%d\n",numberF);
    for(i=0;i<numberV;i++){
      fprintf(out,"vt %lf %lf\n",pU[i],pV[i]);
    }
    // for(i=0;i<numberF;i++)
    //   fprintf(out,"3 %d %d %d\n",Face[i][0],Face[i][1],Face[i][2]);
    fclose(out);
    
  }
  

  Polyhedron(const Polyhedron& rhs);
  const Polyhedron &operator=(const Polyhedron& rhs);




private:



  void BoundaryMap(){
    int i,j,k;
  
    pU = new double[numberV];
    pV = new double[numberV];
    double maxvalU,minvalU,valrangeU;
    double maxvalV,minvalV,valrangeV;
    
    IDList *now=NULL;
    IDList *BedgeH = new IDList();
    IDList *BedgeT = new IDList();
    BedgeH->next = BedgeT;
    BedgeT->back = BedgeH;
    int cnt=0;
    
    for(i=0;i<numberF;i++){
      if(boundary[Face[i][0]]==1&&
	 boundary[Face[i][1]]==1&&
	 boundary[Face[i][2]]==1){
	
	  IDtool->AppendVF(Face[i][0],BedgeT);
	  IDtool->AppendVF(Face[i][1],BedgeT);
	  IDtool->AppendVF(Face[i][1],BedgeT);
	  IDtool->AppendVF(Face[i][2],BedgeT);
	  IDtool->AppendVF(Face[i][2],BedgeT);
	  IDtool->AppendVF(Face[i][0],BedgeT);
	
	
	
      }else{
	if((boundary[Face[i][0]]==1&&boundary[Face[i][1]]==1)||
	   (boundary[Face[i][1]]==1&&boundary[Face[i][2]]==1)||
	   (boundary[Face[i][2]]==1&&boundary[Face[i][0]]==1)
	   ){
	  if(boundary[Face[i][0]]==1&&boundary[Face[i][1]]==1){
	    IDtool->AppendVF(Face[i][0],BedgeT);
	    IDtool->AppendVF(Face[i][1],BedgeT);
	    
	  }else if(boundary[Face[i][1]]==1&&boundary[Face[i][2]]==1){
	    IDtool->AppendVF(Face[i][1],BedgeT);
	    IDtool->AppendVF(Face[i][2],BedgeT);
	    
	  }else if(boundary[Face[i][2]]==1&&boundary[Face[i][0]]==1){
	    IDtool->AppendVF(Face[i][2],BedgeT);
	    IDtool->AppendVF(Face[i][0],BedgeT);
	    
	  }
	}
      }
    }
    
    int jumpcheck=0;
    now = BedgeH;
    while(next(now)!=BedgeT){
      now = next(now);jumpcheck=0;
      if(boundary[now->ID]== 1&&boundary[next(now)->ID]==1){
	int checkbedge = 0;
	IDList *now2 = VHead[now->ID];
	while(next(now2)!=VTail[now->ID]){
	  now2 = next(now2);
	  if(now2->ID==next(now)->ID||
	     next(now2)->ID==next(now)->ID){
	    checkbedge++;
	  }
	  now2 = next(now2);
	  
	}
	if(checkbedge>=2){
	  
	  IDList *dummyn = next(now)->next;
	  IDList *dummyb = now->back;
	  dummyb->next = dummyn;
	  dummyn->back = dummyb;
	  
	  IDList *ddn = next(now);
	  IDList *ddnn = now;
	  delete ddn;
	  delete ddnn;
	  
	  now = dummyb;
	  jumpcheck=1;
	}
      }
      if(jumpcheck!=1)
      now = next(now);
    }
    

    
    
    IDList *BpointH = new IDList();
    IDList *BpointT = new IDList();
    
    BpointH->next = BpointT;
    BpointT->back = BpointH;
    double tlength=0.0;
    now = BedgeH;
    while(next(now)!=BedgeT){
      now = next(now);
      tlength += PT->Distance(point[now->ID],point[next(now)->ID]);    
      
      now = next(now);
      
    }
    int startID=0;
    int *checkbin = new int[numberV];
    for(i=0;i<numberV;i++)checkbin[i]=-1;
    if(pickID==-1){
      for(i=0;i<numberV;i++)if(boundary[i]==1){
	pickID=i;break;
      }
    }
    
    //printf("start id = %d\n",pickID);
    startID=pickID;
    checkbin[pickID]=0;
    
  IDtool->AppendVF(pickID,BpointT);
  k=1;
  
  
  while(1){
    now = BedgeH;
    
    while(next(now)!=BedgeT){
      now = next(now);
      if(startID==now->ID||startID==next(now)->ID){
	if(startID==now->ID&&checkbin[next(now)->ID]==-1){
	  k++;
	  checkbin[next(now)->ID]=0;
	  IDtool->AppendVF(next(now)->ID,BpointT);
	  startID=next(now)->ID;
	  break;
	}else if(startID==next(now)->ID&&checkbin[now->ID]==-1){
	  k++;
	  checkbin[now->ID]=0;
	  IDtool->AppendVF(now->ID,BpointT);
	  startID=now->ID;
	  break;
	}
	
      }
      now = next(now);
      //printf("%d\n",k );
    }
   // printf("%d %d\n",k,numboundary );
    if(k>=numboundary)break;
    // if(k==6)
    // 	break;
   // printf("stuck here\n");
  }
  // 	now = BpointH;
  // 		while(now!=BpointT) {
  // 		    boundary[now->ID] = 2;
  // 		    now = next(now);
  // 		}
  // 	for (int i = 0; i < numberV; ++i)
  // 	{
  // 		if(boundary[i] == 2)
  // 			{
  // 				boundary[i] = 1;
  // 				printf("%d\n", i);
		// 	}
  // 		else
  // 			boundary[i] = 0;
  // 	}

  // numboundary = 6;

  IDtool->CleanNeighbor(BedgeH,BedgeT);
  tlength *= 0.25;
  
  double clen=0.0;
  int state=0;
  i=0;
  
  
  /* Unit Circle */
  if(boundarytype==1){
    now = BpointH;
    while(next(now)!=BpointT){
      now = next(now);
      pU[now->ID] = 0.5*cos(2.0*PI*(((double)(i))/((double)(numboundary))));
      pV[now->ID] = 0.5*sin(2.0*PI*(((double)(i))/((double)(numboundary))));
      i++;
    }
  }else{
    
    /* Unit Square */
    
    now = BpointH;
    
    clen=0.0;
    
    
    while(next(next(now))!=BpointT){
      now = next(now);
      
      if(clen<1.0){
	pU[now->ID] = clen; 
	pV[now->ID]=0.0;
	
      }else if(clen>=1.0&&clen<2.0){
	if(state==0){
	  clen = 1.0;
	  pU[now->ID] = 1.0;
	  pV[now->ID]=0.0;
	  
	  state=1;
	}else{
	  pU[now->ID] = 1.0;
	  pV[now->ID]= clen-1.0;
	  
	}
	
      }else if(clen>=2.0&&clen<3.0){
	if(state==1){
	  clen = 2.0;
	  pU[now->ID] = 1.0;
	  pV[now->ID] = 1.0;
	  state=2;
	}else{
	  pU[now->ID] = 3.0-clen;
	  pV[now->ID]= 1.0;
	}
	
	
      }else if(clen>=3.0){
	if(state==2){
	  clen = 3.0;
	  pU[now->ID] = 0.0;
	  pV[now->ID] = 1.0;
	  state=3;
	}else{
	  pU[now->ID] = 0.0;
	  pV[now->ID] = 4.0-clen;
	}
      }
      
      clen += PT->Distance(point[now->ID],point[next(now)->ID])/tlength;
    }
    now = next(now);
    clen += PT->Distance(point[now->ID],point[next(BpointH)->ID])/tlength;
    pU[now->ID] = 0.0;
    pV[now->ID] = 4.0-clen;
  }
  
  for(i=0;i<numberV;i++){
    if(boundary[i]==1){
            
    }else{
      pU[i] = 0.5;
      pV[i] = 0.5;    
    }
  }
  
  IDtool->CleanNeighbor(BpointH,BpointT);
  delete [] checkbin;
  
  
  
  
  
  }
  
  void memorydelete(){
    int i;
    if(E!=NULL)delete [] E;
    if(G!=NULL)delete [] G;
    for(i=0;i<10;i++){
      delete bc[i];
      delete dbc[i];
    }
    delete [] bc;
    bc = NULL;
    delete [] dbc;
    dbc = NULL;
    
    if(pU!=NULL){
      delete [] pU;
      pU = NULL;
      
    }
    if(pV!=NULL){
      delete [] pV;
      pV = NULL;
      
    }
    if(areaMap3D!=NULL){
      delete [] areaMap3D;
      
    }
    if(IDtool!=NULL){
      if(FHead!=NULL){
	IDtool->CleanNeighborL(FHead,FTail,numberV);
	
      }
      if(IHead!=NULL&&ITail!=NULL){
	IDtool->CleanNeighborL(IHead,ITail,numberV);
	IHead=NULL;
	ITail=NULL;
      }
      if(PHead!=NULL&&PTail!=NULL){
	for(i=0;i<numberV;i++){
	  
	  if(boundary[i]!=1&&PHead[i]!=NULL&&PTail[i]!=NULL&&PHead[i]->next!=PTail[i]){
	    IDtool->CleanNeighborPolar(PHead[i],PTail[i]);
	  }
	}
	delete [] PHead;
	delete [] PTail;
	
      }
      
      if(VHead!=NULL&&VTail!=NULL){
	IDtool->CleanNeighborL(VHead,VTail,numberV);
	VHead=NULL;
	VTail=NULL;
      }
      delete IDtool;
      IDtool=NULL; 
    }
    
  if(point!=NULL){
    if(numberV!=0){
      for(i=0;i<numberV;i++){
	delete point[i];
      }
    }
    delete [] sigma;
    delete [] point;
    point = NULL;
    
  }
  if(Face!=NULL){
    if(numberF!=0){
      for(i=0;i<numberF;i++)delete [] Face[i];
    }
    delete [] Face;
    Face=NULL;
  }
  if(neighborI!=NULL){
    delete [] neighborI;
  }
 if(neighborF!=NULL){
    delete [] neighborF;
  }
 if(boundary!=NULL){
    delete [] boundary;
  }
 delete PT;
  }  
  
  void memoryallocate(int dV,int dF){
            
    numberV=dV;
    numberF=dF;
    E = new double[numberF];
    G = new double[numberF];
    sigma = new double[numberV];
    point = new Point3d* [numberV];
    Face = new int* [numberF];
    IDtool = new IDSet();
    IHead = new IDList* [numberV];
    ITail = new IDList* [numberV];
    PHead = new PolarList* [numberV];
    PTail = new PolarList* [numberV];
  
    VHead = new IDList* [numberV];
    VTail = new IDList* [numberV];
    FHead= new IDList* [numberV];
    FTail= new IDList* [numberV];
    neighborF= new int[numberV];
    boundary = new int[numberV];
    neighborI = new int[numberV]; 
    areaMap3D = new double[numberF];
    
    numboundary=0;
    bc = new Point3d* [10];
    dbc = new Point3d* [10];
    int i;
    for(i=0;i<10;i++){
      bc[i] = new Point3d(0.0,0.0,0.0);
      dbc[i] = new Point3d(0.0,0.0,0.0);
    }
    for(i=0;i<numberV;i++){
      boundary[i]=0;
      neighborI[i]=0;
      neighborF[i]=0;
      sigma[i]=0.0;
      
    
    }
    for(i=0;i<numberF;i++){
      areaMap3D[i]=0.0;
      E[i]=0.0;
      G[i]=0.0;
    }
  }
  void setPoint(int i,double dx,double dy,double dz){
  point[i] = new Point3d(dx,dy,dz);
  IHead[i] = new IDList();
  ITail[i] = new IDList();
  IHead[i]->next = ITail[i];
  ITail[i]->back = IHead[i];
  PHead[i] = new PolarList();
  PTail[i] = new PolarList();
  PHead[i]->next = PTail[i];
  PTail[i]->back = PHead[i];
  FHead[i] = new IDList();
  FTail[i] = new IDList();
  FHead[i]->next = FTail[i];
  FTail[i]->back = FHead[i];
  
  VHead[i] = new IDList();
  VTail[i] = new IDList();
  VHead[i]->next = VTail[i];
  VTail[i]->back = VHead[i];
  
}
  void SetBoundaryLines(){
    int i=0;numboundary=0;
    if(neighborI!=NULL && neighborF!=NULL && boundary!=NULL)
      for(i=0;i<numberV;i++){
	if(((neighborI[i]) == neighborF[i]) && 
	   (neighborF[i] !=0) && 
	   (neighborI[i] !=0)){
	  boundary[i] = 0;
	}else{
	  boundary[i] = 1;
	  numboundary++;
	  printf("%d\n", i);
	}
      }
    printf("number of boundary points = %d\n",numboundary);
    
  }
  void setFace(int i,int di,int dj,int dk){
  Face[i] = new int[3];
  Face[i][0] = di;
  Face[i][1] = dj;
  Face[i][2] = dk;
  /* One */
  neighborF[di]++;
  
  IDtool->AppendISort(dj,IHead[di],ITail[di],di,neighborI);
  IDtool->AppendISort(dk,IHead[di],ITail[di],di,neighborI);
  IDtool->AppendVF(dj,VTail[di]);
  IDtool->AppendVF(dk,VTail[di]);
  /* Two */
  neighborF[dj]++;
  
  IDtool->AppendISort(di,IHead[dj],ITail[dj],dj,neighborI);
  IDtool->AppendISort(dk,IHead[dj],ITail[dj],dj,neighborI);
  IDtool->AppendVF(dk,VTail[dj]);
  IDtool->AppendVF(di,VTail[dj]);
  /* Three */
  neighborF[dk]++;
  
  IDtool->AppendISort(di,IHead[dk],ITail[dk],dk,neighborI);
  IDtool->AppendISort(dj,IHead[dk],ITail[dk],dk,neighborI);
  IDtool->AppendVF(di,VTail[dk]);
  IDtool->AppendVF(dj,VTail[dk]);
  }


  void setPolarMapNaturalB(){

  int i,j,k;
  IDList *now=NULL;
  IDList *now2=NULL;
  double dx,dy;
  PolarList *nowp=NULL;
  PolarList *nowp2=NULL;
  int nowID,nextID;
  double angle = 0.0;
  double theta = 0.0;
  int bid0=0;
  int bid1=0;
   int sw0=0;
  int cn0=0;
  // make Geodesic Polar Map 
  for(i=0;i<numberV;i++){
    dy = 0.0;
    theta = 0.0;
    now = VHead[i];
    while(next(now)!=VTail[i]){
      now = next(now);
      PT->makeVector(bc[0],point[i],point[now->ID]);
      PT->makeVector(bc[1],point[i],point[next(now)->ID]);
      angle = acos((PT->InnerProduct(bc[0],bc[1])/(PT->Point3dSize(bc[0])*PT->Point3dSize(bc[1]))));
      theta += angle;
      now = next(now);
    }
    
    if(boundary[i]!=1){
      now = IHead[i];
      now = next(now);
      nowID = now->ID;
      dx = PT->Distance(point[i],point[now->ID]);
      dy = 0.0;
      IDtool->AppendPolarI(nowID,PTail[i],dx,dy);
      cn0 = 1;
      while(cn0<neighborI[i]){
	now = VHead[i];
	while(next(now)!=VTail[i]){
	  now = next(now);
	  if(now->ID==nowID){
	    nextID = next(now)->ID;
	    break;
	  }
	  now = next(now);
	}
	
	PT->makeVector(bc[0],point[i],point[nowID]);
	PT->makeVector(bc[1],point[i],point[nextID]);
	dx = PT->Point3dSize(bc[1]);
	angle = acos((PT->InnerProduct(bc[0],bc[1])/(PT->Point3dSize(bc[0])*PT->Point3dSize(bc[1]))));
	dy += (2.0*PI*angle)/theta;
	IDtool->AppendPolarI(nextID,PTail[i],dx,dy);
	nowID = nextID;
	cn0++;
      }
    }else{
       int thcheck=0;
      now = IHead[i];
      while(next(now)!=ITail[i]){
	now = next(now);
	if(neighborI[now->ID]==2&&boundary[now->ID]==1){
	  thcheck=1;
	  nowID = now->ID;break;
	}
	
      }
      if(thcheck==1){sw0=0;
	now = VHead[i];
	while(next(now)!=VTail[i]){
	  now = next(now);
	  if(now->ID==nowID){
	    sw0=1;
	    break;
	  }
	  now = next(now);
	}
	if(sw0==0){
	  cn0 = 1;
	
	  while(cn0<neighborI[i]){
	    now = VHead[i];
	    while(next(now)!=VTail[i]){
	      now = next(now);
	      if(next(now)->ID==nowID){
		nextID = now->ID;
	      
		break;
	      }
	      now = next(now);
	    }
	    nowID = nextID;
	    cn0++;
	  }
	}
      }else{

      
	now = VHead[i];
	while(next(now)!=VTail[i]){
	  now = next(now);
	  if(boundary[now->ID]==1){
	    nowID = now->ID;
	    break;
	  }
	  now = next(now);
	}
      }
      
	dx = PT->Distance(point[i],point[nowID]);
	dy = 0.0;
	IDtool->AppendPolarI(nowID,PTail[i],dx,dy);
	cn0 = 1;
	
	while(cn0<neighborI[i]){
	  now = VHead[i];
	  while(next(now)!=VTail[i]){
	    now = next(now);
	    if(now->ID==nowID){
	      nextID = next(now)->ID;
	      
	      break;
	    }
	    now = next(now);
	  }
	  
	  PT->makeVector(bc[0],point[i],point[nowID]);
	  PT->makeVector(bc[1],point[i],point[nextID]);
	  dx = PT->Point3dSize(bc[1]);
	  angle = acos((PT->InnerProduct(bc[0],bc[1])/(PT->Point3dSize(bc[0])*PT->Point3dSize(bc[1]))));
	  
	  dy += (PI*angle)/theta;
	  IDtool->AppendPolarI(nextID,PTail[i],dx,dy);
	  
	  nowID = nextID;
	  cn0++;
	}
           
      
      
      
      
      
    }
  }
}

  void setPolarMap(){
    
    int i,j,k;
  IDList *now=NULL;
  IDList *now2=NULL;
  double dx,dy;
  PolarList *nowp=NULL;
  PolarList *nowp2=NULL;
  int nowID,nextID;
  double angle = 0.0;
  double theta = 0.0;
  int cn0=0;
  // make Geodesic Polar Map 
  for(i=0;i<numberV;i++){
    if(boundary[i]!=1){
      
      dy = 0.0;
      theta = 0.0;
      now = VHead[i];
      while(next(now)!=VTail[i]){
	now = next(now);
	PT->makeVector(bc[0],point[i],point[now->ID]);
	PT->makeVector(bc[1],point[i],point[next(now)->ID]);
	angle = acos((PT->InnerProduct(bc[0],bc[1])/(PT->Point3dSize(bc[0])*PT->Point3dSize(bc[1]))));
	theta += angle;
	now = next(now);
      }
      
      
      now = IHead[i];
      now = next(now);
      nowID = now->ID;
      dx = PT->Distance(point[i],point[now->ID]);
      dy = 0.0;
      IDtool->AppendPolarI(nowID,PTail[i],dx,dy);
      cn0 = 1;
      while(cn0<neighborI[i]){
	now = VHead[i];
	while(next(now)!=VTail[i]){
	  now = next(now);
	  if(now->ID==nowID){
	    nextID = next(now)->ID;
	    break;
	  }
	  now = next(now);
	}
	
	PT->makeVector(bc[0],point[i],point[nowID]);
	PT->makeVector(bc[1],point[i],point[nextID]);
	dx = PT->Point3dSize(bc[1]);
	angle = acos((PT->InnerProduct(bc[0],bc[1])/(PT->Point3dSize(bc[0])*PT->Point3dSize(bc[1]))));
	dy += (2.0*PI*angle)/theta;
	IDtool->AppendPolarI(nextID,PTail[i],dx,dy);
	nowID = nextID;
	cn0++;
      }
    }
  }
  
  
  
  
}

void setAreaMap3D(){
  int i,j;
  IDList *now=NULL;
  sumarea3D = 0.0;
  for(i=0;i<numberF;i++){
    PT->makeVector(bc[0],point[Face[i][0]],point[Face[i][1]]);
    PT->makeVector(bc[1],point[Face[i][0]],point[Face[i][2]]);
    PT->CrossVector(bc[2],bc[1],bc[0]);
    areaMap3D[i] = PT->Point3dSize(bc[2])/2.0;
    sumarea3D += areaMap3D[i];
  }
  
  if(boundarytype==0||boundarytype==2){
    constsumarea3D = sqrt(1.0/sumarea3D);
  }else if(boundarytype==1){
    constsumarea3D = sqrt(((0.5*0.5*PI)/sumarea3D));
  }
 
}




// U1 low stretch parameterization
 void ParametrizationSingle(int itenum,double error){
   double *UaXY = new double[2*(numberV)+1];
   double *vecb = new double[2*(numberV)+1];
   
   int i;
   IDList *now;
   IDList *now2;
   PolarList *nowp;
   
   int nonzero=(numberV);
   for(i=0;i<numberV;i++){vecb[i+1]=0.0;
   if(boundary[i]!=1){
     now = IHead[i];
     while(next(now)!=ITail[i]){
       now = next(now);
       nonzero++;
     }
   }
   }
   int iter=0;
   double linerr=0.0;
   double weight=0.0;
   
   PCBCGSolver *mybcg = new PCBCGSolver(2*nonzero);
   
   double *sigsum = new double[numberV];
   
   if(weighttype==0){
     setFloaterC();
   }else if(weighttype==1){
     setLaplaceC();
   }else if(weighttype==2){
     setEckHC();
   }else if(weighttype==3){
     setDesbrunC();
   }else if(weighttype==4){
     setMVCC();
   }else{
     setFloaterC();
   }
   
   
   SortIndexP();
   for(i=0;i<numberV;i++){
     
     if(boundary[i]!=1){
       mybcg->sa[i+1] = 1.0;
       mybcg->sa[i+1+numberV] = 1.0;
       vecb[i+1] = 0.0;
       vecb[i+1+numberV] = 0.0;
     }else{
       mybcg->sa[i+1] = 1.0;
       vecb[i+1] = pU[i];
       mybcg->sa[i+1+numberV] = 1.0;
       vecb[i+1+numberV] = pV[i];
     }
   }
   mybcg->ija[1] = 2*(numberV)+2;
   int dlk=2*(numberV)+1;
   
   for(i=0;i<numberV;i++){
     if(boundary[i]!=1){
       nowp = PHead[i];
      
       while(next(nowp)!=PTail[i]){
	 nowp = next(nowp);
	 ++dlk;
	mybcg->sa[dlk] = -nowp->lambda;
	mybcg->ija[dlk]=nowp->ID+1;
      }
    }
    mybcg->ija[i+1+1]=dlk+1;
  }
  for(i=0;i<numberV;i++){
    if(boundary[i]!=1){
      nowp = PHead[i];
      while(next(nowp)!=PTail[i]){
	nowp = next(nowp);
	++dlk;
	mybcg->sa[dlk] = -nowp->lambda;
	mybcg->ija[dlk]=nowp->ID+numberV+1;
      }
    }
    mybcg->ija[i+numberV+1+1]=dlk+1;
  }
  
  for(i=0;i<numberV;i++){
    UaXY[i+1] = pU[i];
    UaXY[i+numberV+1] = pV[i];
  }
  mybcg->linbcg(((unsigned long)(2*(numberV))),vecb,UaXY,1,error,(1000+itenum),&iter,&linerr);
  
  for(i=0;i<numberV;i++){
    if(boundary[i]!=1){
      pU[i] = UaXY[i+1];
      pV[i] = UaXY[i+numberV+1];
    }
  }
  
  // Re-solving linear system 

  
  
  // local stretch calculation
  setSigmaZero();
  
  // Re-solving linear system according to local stretches

  for(i=0;i<numberV;i++){
    
    if(boundary[i]!=1){
       sigsum[i]=0.0;
      nowp = PHead[i];
      while(next(nowp)!=PTail[i]){
	nowp = next(nowp);
	nowp->lambda /= sigma[nowp->ID];
	sigsum[i] += nowp->lambda;
      }
      
      mybcg->sa[i+1] = 1.0;
      mybcg->sa[i+1+numberV] = 1.0;
      
    }else{
      
      mybcg->sa[i+1] = 1.0;
      mybcg->sa[i+1+numberV] = 1.0;
    }
  }
  
  
  dlk=2*(numberV)+1;
  
  for(i=0;i<numberV;i++){
    if(boundary[i]!=1){
      nowp = PHead[i];
      
      while(next(nowp)!=PTail[i]){
	nowp = next(nowp);
	++dlk;
	mybcg->sa[dlk] = -nowp->lambda/sigsum[i];
      }
    }
    
  }
  for(i=0;i<numberV;i++){
    if(boundary[i]!=1){
      nowp = PHead[i];
       while(next(nowp)!=PTail[i]){
	nowp = next(nowp);
	++dlk;
	mybcg->sa[dlk] = -nowp->lambda/sigsum[i];
	
      }
    }
  }
  
  for(i=0;i<numberV;i++){
    UaXY[i+1] = pU[i];
    UaXY[i+numberV+1] = pV[i];
  }
  mybcg->linbcg(((unsigned long)(2*(numberV))),vecb,UaXY,1,error,itenum,&iter,&linerr);
  
  for(i=0;i<numberV;i++){
    if(boundary[i]!=1){
      pU[i] = UaXY[i+1];
      pV[i] = UaXY[i+numberV+1];
    }
  }
    
  
  printf("STL2 error = %lf\n",getCurrentE());
  
  for(i=0;i<numberV;i++){
    
    if(boundary[i]!=1){
      IDtool->CleanNeighborPolar(PHead[i],PTail[i]);
      PHead[i] = new PolarList();
      PTail[i] = new PolarList();
      PHead[i]->next = PTail[i];
      PTail[i]->back = PHead[i];
    }
  }

  delete mybcg;
  delete [] vecb;
  delete [] UaXY;
  delete [] sigsum;
  
  
  
 }
  // UOpt: Optimal low stretch parameterization
  void ParametrizationOptimal(int itenum,double error){
    double *UaXY = new double[2*(numberV)+1];
    double *vecb = new double[2*(numberV)+1];
  
  int i;
  IDList *now;
  IDList *now2;
  PolarList *nowp;
  level=0;
  
  int nonzero=(numberV);
  for(i=0;i<numberV;i++){
    vecb[i+1]=0.0;
    if(boundary[i]!=1){
      now = IHead[i];
      while(next(now)!=ITail[i]){
	now = next(now);
      nonzero++;
      }
    }
  }
  int iter=0;
  double linerr=0.0;
  double weight=0.0;
  
  PCBCGSolver *mybcg = new PCBCGSolver(2*nonzero);
  double *sigsum = new double[numberV];
  if(weighttype==0){
    setFloaterC();
  }else if(weighttype==1){
    setLaplaceC();
  }else if(weighttype==2){
    setEckHC();
  }else if(weighttype==3){
    setDesbrunC();
  }else if(weighttype==4){
    setMVCC();
  }else{
    setFloaterC();
  }
  
  
  SortIndexP();
  
  
  for(i=0;i<numberV;i++){
    
    if(boundary[i]!=1){
      
      
      
      mybcg->sa[i+1] = 1.0;
      mybcg->sa[i+1+numberV] = 1.0;
      vecb[i+1] = 0.0;
      vecb[i+1+numberV] = 0.0;
    }else{
      mybcg->sa[i+1] = 1.0;
      vecb[i+1] = pU[i];
      mybcg->sa[i+1+numberV] = 1.0;
      vecb[i+1+numberV] = pV[i];
    }
  }
  mybcg->ija[1] = 2*(numberV)+2;
  int dlk=2*(numberV)+1;
  
  for(i=0;i<numberV;i++){
    if(boundary[i]!=1){
      nowp = PHead[i];
      
      while(next(nowp)!=PTail[i]){
	nowp = next(nowp);
	
	
	++dlk;
	mybcg->sa[dlk] = -nowp->lambda;
	mybcg->ija[dlk]=nowp->ID+1;
      }
    }
    mybcg->ija[i+1+1]=dlk+1;
  }
  for(i=0;i<numberV;i++){
    if(boundary[i]!=1){
       nowp = PHead[i];
       while(next(nowp)!=PTail[i]){
	 nowp = next(nowp);
	 ++dlk;
	 mybcg->sa[dlk] = -nowp->lambda;
	 mybcg->ija[dlk]=nowp->ID+numberV+1;
       }
    }
    mybcg->ija[i+numberV+1+1]=dlk+1;
  }
  
  for(i=0;i<numberV;i++){
    UaXY[i+1] = pU[i];
    UaXY[i+numberV+1] = pV[i];
  }
  mybcg->linbcg(((unsigned long)(2*(numberV))),vecb,UaXY,1,error,itenum,&iter,&linerr);
  
  for(i=0;i<numberV;i++){
    if(boundary[i]!=1){
      pU[i] = UaXY[i+1];
      pV[i] = UaXY[i+numberV+1];
    }
  }

  
  // Re-solving linear system
  double initialstrech=0.0;
  double currentstrech=0.0;
  Point2d **prevU = new Point2d* [numberV];
  for(i=0;i<numberV;i++){
    prevU[i] = new Point2d(0.0,0.0); 
  }
  initialstrech = getCurrentE();
  int kk = 0;
  
  for(kk=0;kk<itenum;kk++){
    
    setSigmaZero();
    for(i=0;i<numberV;i++){
      
      if(boundary[i]!=1){
	sigsum[i]=0.0;
	nowp = PHead[i];
	while(next(nowp)!=PTail[i]){
	  nowp = next(nowp);
	 
	  nowp->lambda /= sigma[nowp->ID];
	  
	 
	  
	  sigsum[i] += nowp->lambda;
	}
	
	mybcg->sa[i+1] = 1.0;
	mybcg->sa[i+1+numberV] = 1.0;
	
      }else{
      
	mybcg->sa[i+1] = 1.0;
	mybcg->sa[i+1+numberV] = 1.0;
      }
    }
    dlk=2*(numberV)+1;
    
    for(i=0;i<numberV;i++){
      if(boundary[i]!=1){
	nowp = PHead[i];
	
	while(next(nowp)!=PTail[i]){
	  nowp = next(nowp);
	  ++dlk;
	  mybcg->sa[dlk] = -nowp->lambda/sigsum[i];
	}
      }
    }
    for(i=0;i<numberV;i++){
      if(boundary[i]!=1){
	nowp = PHead[i];
	while(next(nowp)!=PTail[i]){
	  nowp = next(nowp);
	  ++dlk;
	  mybcg->sa[dlk] = -nowp->lambda/sigsum[i];
	}
      }
    }
     
    for(i=0;i<numberV;i++){
      UaXY[i+1] = pU[i];
      UaXY[i+numberV+1] = pV[i];
    }
    mybcg->linbcg(((unsigned long)(2*(numberV))),vecb,UaXY,1,error,itenum,&iter,&linerr); 
    for(i=0;i<numberV;i++){
      if(boundary[i]!=1){
	prevU[i]->x = pU[i];
	prevU[i]->y = pV[i];
	
	pU[i] = UaXY[i+1];
	pV[i] = UaXY[i+numberV+1];
      }
    }
    currentstrech = getCurrentE();
    //printf("currentstrech = %lf\n",currentstrech);
    if(initialstrech<currentstrech){
      for(i=0;i<numberV;i++){
	if(boundary[i]!=1){
	  pU[i] = prevU[i]->x;
	  pV[i] = prevU[i]->y;
	}
      }
      break;
    }else{
      initialstrech = currentstrech;
    }
  }
  
  level = kk;
 
 
  printf("STL2 error = %lf\n",getCurrentE());
  delete mybcg;
  for(i=0;i<numberV;i++){
    
    if(boundary[i]!=1){
    IDtool->CleanNeighborPolar(PHead[i],PTail[i]);
      
    PHead[i] = new PolarList();
    PTail[i] = new PolarList();
    PHead[i]->next = PTail[i];
    PTail[i]->back = PHead[i];
    }
  }
  for(i=0;i<numberV;i++){
    delete prevU[i];
  }

 

  delete [] prevU;
  delete [] vecb;
  delete [] UaXY;
  delete [] sigsum;

}
 
 // UOpt: Optimal low stretch parameterization (slow and smooth)
  void ParametrizationSmoothOptimal(int itenum,double error){
    double *UaXY = new double[2*(numberV)+1];
    double *vecb = new double[2*(numberV)+1];
  
  int i;
  IDList *now;
  IDList *now2;
  PolarList *nowp;
  level=0;
  
  int nonzero=(numberV);
  for(i=0;i<numberV;i++){
    vecb[i+1]=0.0;
    if(boundary[i]!=1){
      now = IHead[i];
      while(next(now)!=ITail[i]){
	now = next(now);
      nonzero++;
      }
    }
  }
  int iter=0;
  double linerr=0.0;
  double weight=0.0;
  
  PCBCGSolver *mybcg = new PCBCGSolver(2*nonzero);
  double *sigsum = new double[numberV];
  if(weighttype==0){
    setFloaterC();
  }else if(weighttype==1){
    setLaplaceC();
  }else if(weighttype==2){
    setEckHC();
  }else if(weighttype==3){
    setDesbrunC();
  }else if(weighttype==4){
    setMVCC();
  }else{
    setFloaterC();
  }
    
  SortIndexP();
  
  
  for(i=0;i<numberV;i++){
    
    if(boundary[i]!=1){
      
      
      
      mybcg->sa[i+1] = 1.0;
      mybcg->sa[i+1+numberV] = 1.0;
      vecb[i+1] = 0.0;
      vecb[i+1+numberV] = 0.0;
    }else{
      mybcg->sa[i+1] = 1.0;
      vecb[i+1] = pU[i];
      mybcg->sa[i+1+numberV] = 1.0;
      vecb[i+1+numberV] = pV[i];
    }
  }
  mybcg->ija[1] = 2*(numberV)+2;
  int dlk=2*(numberV)+1;
  
  for(i=0;i<numberV;i++){
    if(boundary[i]!=1){
      nowp = PHead[i];
      
      while(next(nowp)!=PTail[i]){
	nowp = next(nowp);
	
	
	++dlk;
	mybcg->sa[dlk] = -nowp->lambda;
	mybcg->ija[dlk]=nowp->ID+1;
      }
    }
    mybcg->ija[i+1+1]=dlk+1;
  }
  for(i=0;i<numberV;i++){
    if(boundary[i]!=1){
       nowp = PHead[i];
       while(next(nowp)!=PTail[i]){
	 nowp = next(nowp);
	 ++dlk;
	 mybcg->sa[dlk] = -nowp->lambda;
	 mybcg->ija[dlk]=nowp->ID+numberV+1;
       }
    }
    mybcg->ija[i+numberV+1+1]=dlk+1;
  }
  
  for(i=0;i<numberV;i++){
    UaXY[i+1] = pU[i];
    UaXY[i+numberV+1] = pV[i];
  }
  mybcg->linbcg(((unsigned long)(2*(numberV))),vecb,UaXY,1,error,itenum,&iter,&linerr);
  
  for(i=0;i<numberV;i++){
    if(boundary[i]!=1){
      pU[i] = UaXY[i+1];
      pV[i] = UaXY[i+numberV+1];
    }
  }
  
  // Re-solving linear system
  double initialstrech=0.0;
  double currentstrech=0.0;
  Point2d **prevU = new Point2d* [numberV];
  for(i=0;i<numberV;i++){
    prevU[i] = new Point2d(0.0,0.0); 
  }
  initialstrech = getCurrentE();
  int kk = 0;
  
  for(kk=0;kk<itenum;kk++){
    setSigmaZero();
    for(i=0;i<numberV;i++){
      
      if(boundary[i]!=1){
	sigsum[i]=0.0;
	nowp = PHead[i];
	while(next(nowp)!=PTail[i]){
	  nowp = next(nowp);
	  
	  nowp->lambda /= (0.5*(sigma[nowp->ID]+sigma[i]));
	  
	  
	  
	  sigsum[i] += nowp->lambda;
	}
	
	mybcg->sa[i+1] = 1.0;
	mybcg->sa[i+1+numberV] = 1.0;
	
      }else{
      
	mybcg->sa[i+1] = 1.0;
	mybcg->sa[i+1+numberV] = 1.0;
      }
    }
    dlk=2*(numberV)+1;
    
    for(i=0;i<numberV;i++){
      if(boundary[i]!=1){
	nowp = PHead[i];
	
	while(next(nowp)!=PTail[i]){
	  nowp = next(nowp);
	  ++dlk;
	  mybcg->sa[dlk] = -nowp->lambda/sigsum[i];
	}
      }
    }
    for(i=0;i<numberV;i++){
      if(boundary[i]!=1){
	nowp = PHead[i];
	while(next(nowp)!=PTail[i]){
	  nowp = next(nowp);
	  ++dlk;
	  mybcg->sa[dlk] = -nowp->lambda/sigsum[i];
	}
      }
    }
    
    for(i=0;i<numberV;i++){
      UaXY[i+1] = pU[i];
      UaXY[i+numberV+1] = pV[i];
    }
    mybcg->linbcg(((unsigned long)(2*(numberV))),vecb,UaXY,1,error,itenum,&iter,&linerr);
    for(i=0;i<numberV;i++){
      if(boundary[i]!=1){
	prevU[i]->x = pU[i];
	prevU[i]->y = pV[i];
	
	pU[i] = UaXY[i+1];
	pV[i] = UaXY[i+numberV+1];
      }
    }
    currentstrech = getCurrentE();
    //printf("currentstrech = %lf\n",currentstrech);
    if(initialstrech<currentstrech){
      for(i=0;i<numberV;i++){
	if(boundary[i]!=1){
	  pU[i] = prevU[i]->x;
	  pV[i] = prevU[i]->y;
	}
      }
      break;
    }else{
      initialstrech = currentstrech;
    }
  }
  
  level = kk;
  
  
  printf("STL2 error = %lf\n",getCurrentE());
  delete mybcg;
  for(i=0;i<numberV;i++){
    
    if(boundary[i]!=1){
    IDtool->CleanNeighborPolar(PHead[i],PTail[i]);
      
    PHead[i] = new PolarList();
    PTail[i] = new PolarList();
    PHead[i]->next = PTail[i];
    PTail[i]->back = PHead[i];
    }
  }
  for(i=0;i<numberV;i++){
    delete prevU[i];
  }

  

  delete [] prevU;
  delete [] vecb;
  delete [] UaXY;
  delete [] sigsum;

}
 
 
  // compute natural boundary 
  void setNaturalB(int itenum,double error){
  double *UaXY = new double[2*(numberV)+1];
  double *vecb = new double[2*(numberV)+1];
  
  int i;
  IDList *now;
  IDList *now2;

  int myite = 2000;
  
  if(numberV > 20000)myite = 5000;
  if(numberV > 50000)myite = 10000;
  
  int Bfix0=0;
  int Bfix1=0;
  for(i=0;i<numberV;i++){
    if(boundary[i]==1){
      Bfix0 = i;
      break;
    }
  }
  
  for(i=0;i<numberV;i++){
    if(boundary[i]==1&&neighborI[i]==2){
      Bfix0 = i;
      break;
    }
  }
  

  double dmax=0.0;
  double ddis=0.0;
  int firstcheck=0;
  double dx,dy;
  for(i=0;i<numberV;i++){
    if(boundary[i]==1){
      dx = pU[i]-pU[Bfix0];
      dy = pV[i]-pV[Bfix0];
      
      if(firstcheck==0){
	firstcheck=1;
	dmax = sqrt((dx*dx+dy*dy));
	Bfix1 = i;
      }else{
	ddis = sqrt((dx*dx+dy*dy));
	if(ddis>dmax){
	  dmax = ddis;
	  Bfix1 = i;
	}
      }    
    }
  }

  
  int nonzero=(numberV);
  for(i=0;i<numberV;i++){
    vecb[i+1]=0.0;
    if(i!=Bfix0&&i!=Bfix1){
      now = IHead[i];
      while(next(now)!=ITail[i]){
	now = next(now);
	nonzero++;
      }
      if(boundary[i]==1)nonzero += 2;
    }
  }
  
  int bbID0=0;
  int bbID1=0;
  
  

  int iter=0;
  double linerr=0.0;
  double weight=0.0;
  double *coef = new double[4];
  Point2d *v0 = new Point2d(0.0,0.0);
  Point2d *v1 = new Point2d(0.0,0.0);
  Point2d *v2 = new Point2d(0.0,0.0);
  Point2d *v3 = new Point2d(0.0,0.0);
  PCBCGSolver *mybcg = new PCBCGSolver(2*nonzero);
  
  int nextID=0;
  int backID=0;

  double conformalterm;
  double *sigsum = new double[numberV];
  
  setPolarMapNaturalB();
  SortIndexPNaturalB();
  



  double ancos1=0.0;
  PolarList *nowp=NULL;
  double dl1,dl2;
  for(i=0;i<numberV;i++){
    nowp = PHead[i];
    while(next(nowp)!=PTail[i]){
      nowp = next(nowp);
      nowp->lambda = 0.0;
      now = VHead[i];
      while(next(now)!=VTail[i]){
	now = next(now);
	
	
	if(nowp->ID==now->ID){
	  PT->makeVector(bc[0],point[next(now)->ID],point[i]);
	  PT->makeVector(bc[1],point[next(now)->ID],point[now->ID]);
	  ancos1 = PT->InnerProduct(bc[0],bc[1])/(PT->Point3dSize(bc[0])*PT->Point3dSize(bc[1]));
	  nowp->lambda += (ancos1/sqrt(1.0-ancos1*ancos1));
	  
	}
	if(nowp->ID==next(now)->ID){
	  PT->makeVector(bc[0],point[now->ID],point[i]);
	  PT->makeVector(bc[1],point[now->ID],point[next(now)->ID]);
	  ancos1 = PT->InnerProduct(bc[0],bc[1])/(PT->Point3dSize(bc[0])*PT->Point3dSize(bc[1]));
	  nowp->lambda += (ancos1/sqrt(1.0-ancos1*ancos1));
	}
	now = next(now);
      }
    }
  }
  



  int cn=0;
  for(i=0;i<numberV;i++){
    if(i==Bfix0||i==Bfix1){
      mybcg->sa[i+1] = 1.0;
      vecb[i+1] = pU[i];
      mybcg->sa[i+1+numberV] = 1.0;
      vecb[i+1+numberV] = pV[i];
    }else{
      sigsum[i]=0.0;
      nowp = PHead[i];
      while(next(nowp)!=PTail[i]){
	nowp = next(nowp);
	sigsum[i] += nowp->lambda;	
      }
      mybcg->sa[i+1] = -1.0;
      mybcg->sa[i+1+numberV] = -1.0;
      vecb[i+1] = 0.0;
      vecb[i+1+numberV] = 0.0;
    }
  
  }
  mybcg->ija[1] = 2*(numberV)+2;
  int dlk=2*(numberV)+1;
  
  int BID0=0;
  int BID1=0;
  
  for(i=0;i<numberV;i++){
    if(i!=Bfix0&&i!=Bfix1){
      
      nowp = PHead[i];
      while(next(nowp)!=PTail[i]){
	nowp = next(nowp);
	++dlk;
	mybcg->sa[dlk] = nowp->lambda/sigsum[i];
	mybcg->ija[dlk]=nowp->ID+1;
      }
      if(boundary[i]==1){
	
	nowp = PHead[i];
	 while(next(nowp)!=PTail[i]){
	   nowp = next(nowp);
	   if(boundary[nowp->ID]==1&&nowp->theta==0.0){
	     BID0 = nowp->ID;
	     break;
	   }
	 }
	 nowp = PHead[i];
	 while(next(nowp)!=PTail[i]){
	   nowp = next(nowp);
	   if(boundary[nowp->ID]==1&&nowp->theta!=0.0){
	     
	     BID1 = nowp->ID;
	     break;
	   }
	 }
	
	 
	 if(BID0<BID1){
	   ++dlk;
	   mybcg->sa[dlk] = 1.0/sigsum[i];
	   mybcg->ija[dlk]=BID0+numberV+1;
	   
	   ++dlk;
	   mybcg->sa[dlk] = -1.0/sigsum[i];
	   mybcg->ija[dlk]=BID1+numberV+1;
	 }else{
	   ++dlk;
	   mybcg->sa[dlk] = -1.0/sigsum[i];
	   mybcg->ija[dlk]=BID1+numberV+1;
	   
	   ++dlk;
	   mybcg->sa[dlk] = 1.0/sigsum[i];
	   mybcg->ija[dlk]=BID0+numberV+1;
	 }
      }
      
      
      
    }    
    
    mybcg->ija[i+1+1]=dlk+1;
  }
  for(i=0;i<numberV;i++){
    if(i!=Bfix0&&i!=Bfix1){
      
    if(boundary[i]==1){
	 nowp = PHead[i];
	 while(next(nowp)!=PTail[i]){
	   nowp = next(nowp);
	   if(boundary[nowp->ID]==1&&nowp->theta==0.0){
	    
	     BID0 = nowp->ID;break;
	   }
	 }
	 nowp = PHead[i];
	 while(next(nowp)!=PTail[i]){
	   nowp = next(nowp);
	   if(boundary[nowp->ID]==1&&nowp->theta!=0.0){
	    
	     BID1 = nowp->ID;break;
	   }
	 }
	 if(BID0<BID1){
	   ++dlk;
	   mybcg->sa[dlk] = -1.0/sigsum[i];
	   mybcg->ija[dlk]=BID0+1;
	   ++dlk;
	   mybcg->sa[dlk] = 1.0/sigsum[i];
	   mybcg->ija[dlk]=BID1+1;
	 }else{
	   ++dlk;
	   mybcg->sa[dlk] = 1.0/sigsum[i];
	   mybcg->ija[dlk]=BID1+1;
	   ++dlk;
	   mybcg->sa[dlk] = -1.0/sigsum[i];
	   mybcg->ija[dlk]=BID0+1;
	 }
      }     
    nowp = PHead[i];
    while(next(nowp)!=PTail[i]){
      nowp = next(nowp);
      ++dlk;
      mybcg->sa[dlk] = nowp->lambda/sigsum[i];
      mybcg->ija[dlk]=nowp->ID+numberV+1;
    }
    
    }
    mybcg->ija[i+numberV+1+1]=dlk+1;
  }
  
  for(i=0;i<numberV;i++){
    UaXY[i+1] = pU[i];
    UaXY[i+numberV+1] = pV[i];
  }
  mybcg->linbcg(((unsigned long)(2*(numberV))),vecb,UaXY,1,error,(myite),&iter,&linerr);
  
  for(i=0;i<numberV;i++){
    if(i!=Bfix0&&i!=Bfix1){
      pU[i] = UaXY[i+1];
      pV[i] = UaXY[i+numberV+1];
    }
  }
  for(i=0;i<numberV;i++){
    
    
    IDtool->CleanNeighborPolar(PHead[i],PTail[i]);
      
    PHead[i] = new PolarList();
    PTail[i] = new PolarList();
    PHead[i]->next = PTail[i];
    PTail[i]->back = PHead[i];
    
  }
  
  delete mybcg;
  delete [] vecb;
  delete [] UaXY;
  delete [] coef;
  delete v0;
  delete v1;
  delete v2;
  delete v3;
  delete [] sigsum;

}
void setEckHC(){
  PolarList *nowp;
  int i;
  int nextID=0;
  int backID=0;
  double ddweight=0.0;
  for(i=0;i<numberV;i++){ 
    if(boundary[i]!=1){
      nowp =PHead[i];
      ddweight=0.0;
      while(next(nowp)!=PTail[i]){
	nowp = next(nowp);
	if(nowp->next!=PTail[i]){
	  nextID = nowp->next->ID;
	}else{
	  nextID = PHead[i]->next->ID;
	}
	if(nowp->back!=PHead[i]){
	  backID = nowp->back->ID;
	}else{
	  backID = PTail[i]->back->ID;
	}
	nowp->lambda = getHarmonicTerm(i,backID,nowp->ID,nextID);
	ddweight += nowp->lambda;
      }
      
      if(ddweight!=0.0){
	nowp =PHead[i];
	while(next(nowp)!=PTail[i]){
	  nowp = next(nowp);
	  nowp->lambda /= ddweight;
	}
      }
    }
  }
}
double getHarmonicTerm(int i,int backID,int nowID,int nextID){
  double ancos1,ancos2;
  PT->makeVector(bc[0],point[i],point[backID]);
  PT->makeVector(bc[1],point[i],point[nowID]);
  PT->makeVector(bc[2],point[i],point[nextID]);
  PT->CrossVector(bc[3],bc[0],bc[1]);

  double area1 = 0.5*PT->Point3dSize(bc[3]);if(area1==0.0)area1=1.0;
  PT->CrossVector(bc[3],bc[1],bc[2]);
  double area2 = 0.5*PT->Point3dSize(bc[3]);if(area2==0.0)area2=1.0;
  
  double lenik1 = PT->Point3dSize(bc[0]);
  double lenjk1 = PT->Distance(point[backID],point[nowID]);
  double lenij = PT->Point3dSize(bc[1]);
  double lenik2 = PT->Point3dSize(bc[2]);
  double lenjk2 = PT->Distance(point[nextID],point[nowID]);
  lenij *= lenij;
  
  ancos1 = (lenik1*lenik1 +lenjk1*lenjk1 - lenij)/area1; 
  
  ancos2 = (lenik2*lenik2 +lenjk2*lenjk2 - lenij)/area2;
    

  return (ancos1+ancos2);

}



void setDesbrunC(){

  PolarList *nowp;
  int i;
  int nextID=0;
  int backID=0;
  double ddweight=0.0;
  for(i=0;i<numberV;i++){ 
    if(boundary[i]!=1){
      nowp =PHead[i];
      ddweight=0.0;
      while(next(nowp)!=PTail[i]){
	nowp = next(nowp);
	if(nowp->next!=PTail[i]){
	  nextID = nowp->next->ID;
	}else{
	  nextID = PHead[i]->next->ID;
	}
	if(nowp->back!=PHead[i]){
	  backID = nowp->back->ID;
	}else{
	  backID = PTail[i]->back->ID;
	}
	nowp->lambda = ((1.0-intrinsiclambda)*getChiTerm(i,backID,nowp->ID,nextID)+intrinsiclambda*getAuthalicTerm(i,backID,nowp->ID,nextID));
	ddweight += nowp->lambda;
      }
      
      if(ddweight!=0.0){
	nowp =PHead[i];
	while(next(nowp)!=PTail[i]){
	  nowp = next(nowp);
	  nowp->lambda /= ddweight;
	}
      }
    }
  }
}
double getAuthalicTerm(int i,int backID,int nowID,int nextID){
  double ancos1,ancos2;
  PT->makeVector(bc[0],point[backID],point[i]);
  PT->makeVector(bc[1],point[backID],point[nowID]);
  PT->makeVector(bc[2],point[nextID],point[nowID]);
  PT->makeVector(bc[3],point[nextID],point[i]);


  
  ancos1 = PT->InnerProduct(bc[0],bc[1])/(PT->Point3dSize(bc[0])*PT->Point3dSize(bc[1]));
  
  ancos2 = PT->InnerProduct(bc[2],bc[3])/(PT->Point3dSize(bc[2])*PT->Point3dSize(bc[3]));
  
  return (ancos1/sqrt(1.0-ancos1*ancos1) +ancos2/sqrt(1.0-ancos2*ancos2));

}
double getChiTerm(int i,int backID,int nowID,int nextID){
  double ancos1,ancos2,len;
  PT->makeVector(bc[0],point[nowID],point[i]);
  PT->makeVector(bc[1],point[nowID],point[backID]);
  PT->makeVector(bc[2],point[nowID],point[nextID]);
  
  
  len = PT->Point3dSize(bc[0]);if(len==0.0)len=1.0;
  ancos1 = PT->InnerProduct(bc[0],bc[1])/(len*PT->Point3dSize(bc[1]));
  ancos2 = PT->InnerProduct(bc[0],bc[2])/(len*PT->Point3dSize(bc[2]));
 
  return ((ancos1/sqrt(1.0-ancos1*ancos1) +ancos2/sqrt(1.0-ancos2*ancos2))/(len*len));
  
}
void setMVCC(){
  PolarList *nowp;
  int i;
  int nextID=0;
  int backID=0;
  double ddweight=0.0;
  for(i=0;i<numberV;i++){ 
    if(boundary[i]!=1){
      nowp =PHead[i];
      ddweight=0.0;
      while(next(nowp)!=PTail[i]){
	nowp = next(nowp);
	if(nowp->next!=PTail[i]){
	  nextID = nowp->next->ID;
	}else{
	  nextID = PHead[i]->next->ID;
	}
	if(nowp->back!=PHead[i]){
	  backID = nowp->back->ID;
	}else{
	  backID = PTail[i]->back->ID;
	}
	nowp->lambda = getMVCTerm(i,backID,nowp->ID,nextID);
	ddweight += nowp->lambda;
      }
      
      if(ddweight!=0.0){
	nowp =PHead[i];
	while(next(nowp)!=PTail[i]){
	  nowp = next(nowp);
	  nowp->lambda /= ddweight;
	  
	}
      }
    }
  }
  
}
void setFloaterC(){
  int i;
  double *coef = new double[4];
  Point2d *v0 = new Point2d(0.0,0.0);
  Point2d *v1= new Point2d(0.0,0.0);
  Point2d *v2= new Point2d(0.0,0.0);
  Point2d *v3= new Point2d(0.0,0.0);
  IDList *now=NULL;
  PolarList *nowp=NULL;
  PolarList *nowp2=NULL;
  int checkintersect=0;
  double angle = 0.0;
  double theta = 0.0;
  double interval=0.0;
  double dweight=0.0;
  for(i=0;i<numberV;i++){
    if(boundary[i]!=1){
      dweight=0.0;
      nowp = PHead[i];
      while(next(nowp)!=PTail[i]){
	nowp = next(nowp);
	nowp2 = PHead[i];
	interval = nowp->theta+PI;
	if(interval>=(2.0*PI))interval -= (2.0*PI);
	checkintersect=0;
	while(next(next(nowp2))!=PTail[i]){
	  nowp2 = next(nowp2);
	  if((interval>=nowp2->theta)&&(interval<=next(nowp2)->theta)){
	    v1->x = nowp2->r*cos(nowp2->theta);
	    v1->y = nowp2->r*sin(nowp2->theta);
	    v2->x = next(nowp2)->r*cos(next(nowp2)->theta);
	    v2->y = next(nowp2)->r*sin(next(nowp2)->theta);
	    v3->x = nowp->r*cos(nowp->theta);
	    v3->y = nowp->r*sin(nowp->theta);
	    PT->setC(coef,v0,v1,v2,v3);
	    nowp->lambda += -coef[0]*coef[3];
	    nowp2->lambda += -coef[0]*coef[1];
	    nowp2->next->lambda += -coef[0]*coef[2];
	    dweight += -coef[0]*coef[0];
	    checkintersect=1;
	    break;
	  }
	}
	if(checkintersect==0){
	  nowp2 = next(nowp2);
	  v1->x = nowp2->r*cos(nowp2->theta);
	  v1->y = nowp2->r*sin(nowp2->theta);
	  v2->x = next(PHead[i])->r;
	  v2->y = 0.0;
	  v3->x = nowp->r*cos(nowp->theta);
	  v3->y = nowp->r*sin(nowp->theta);
	  PT->setC(coef,v0,v1,v2,v3);
	  nowp->lambda += -coef[0]*coef[3];
	  nowp2->lambda += -coef[0]*coef[1];
	  PHead[i]->next->lambda += -coef[0]*coef[2];
	  dweight += -coef[0]*coef[0];
	}
      }
      if(dweight!=0.0)
      nowp = PHead[i];
      while(next(nowp)!=PTail[i]){
	nowp = next(nowp);
	nowp->lambda /= -dweight;
      }
    }
  }
  delete coef;
  delete v0;
  delete v1;
  delete v2;
  delete v3;
  
}

double getMVCTerm(int i,int backID,int nowID,int nextID){
   double angle1,angle2;
  PT->makeVector(bc[0],point[i],point[backID]);
  PT->makeVector(bc[1],point[i],point[nowID]);
  PT->makeVector(bc[2],point[i],point[nextID]);
  double len1 = PT->Point3dSize(bc[0]);
  double len2 = PT->Point3dSize(bc[1]);
  double len3 = PT->Point3dSize(bc[2]);
  angle1 = PT->InnerProduct(bc[0],bc[1])/(len1*len2);
  angle2 = PT->InnerProduct(bc[1],bc[2])/(len2*len3);
  double w1,w2;
  
  
  if((1.0+angle1)==0.0){
    w1 = 1.0;
  }else{
    w1 = sqrt(((1.0-angle1)/(1.0+angle1)));
  }
  if((1.0+angle2)==0.0){
    w2 = 1.0;
  }else{
    w2 = sqrt(((1.0-angle2)/(1.0+angle2)));
  }
  
    
  return ((w1+w2)/len2);
  
 }
 
void SortIndexP(){
  int i;
  IDList *now=NULL;
  PolarList *nowp=NULL;
  
  PolarList *DummyHead = NULL;
  PolarList *DummyTail = NULL;
  
  for(i=0;i<numberV;i++){
    if(boundary[i]!=1){
      DummyHead = new PolarList();
      DummyTail = new PolarList();
      DummyHead->next = DummyTail;
      DummyTail->back = DummyHead;
      
      now = IHead[i];
      while(next(now)!=ITail[i]){
	now = next(now);
	nowp = PHead[i];
	while(next(nowp)!=PTail[i]){
	  nowp = next(nowp);
	  if(now->ID==nowp->ID){
	    
	    IDtool->AppendPolarI(nowp->ID,DummyTail,nowp->r,nowp->theta,nowp->lambda,nowp->cotw);
	    
	    
	    
	    break;
	  }
	}
      }
      
      IDtool->CleanNeighborPolar(PHead[i],PTail[i]);
      
      PHead[i] = new PolarList();
      PTail[i] = new PolarList();
      PHead[i]->next = PTail[i];
      PTail[i]->back = PHead[i];
      nowp = DummyHead;
      while(next(nowp)!=DummyTail){
	nowp = next(nowp);
	
	IDtool->AppendPolarI(nowp->ID,PTail[i],nowp->r,nowp->theta,nowp->lambda,nowp->cotw);
	
	
      }
      
      
      IDtool->CleanNeighborPolar(DummyHead,DummyTail);
      
    }
  }

}
void SortIndexPNaturalB(){
  int i;
  IDList *now=NULL;
  PolarList *nowp=NULL;
  
  PolarList *DummyHead = NULL;
  PolarList *DummyTail = NULL;
  
  for(i=0;i<numberV;i++){
    if(PHead[i]->next!=PTail[i]){
      DummyHead = new PolarList();
      DummyTail = new PolarList();
      DummyHead->next = DummyTail;
      DummyTail->back = DummyHead;
      
      now = IHead[i];
      while(next(now)!=ITail[i]){
	now = next(now);
	nowp = PHead[i];
	while(next(nowp)!=PTail[i]){
	  nowp = next(nowp);
	  if(now->ID==nowp->ID){
	    IDtool->AppendPolarI(nowp->ID,DummyTail,nowp->r,nowp->theta,nowp->lambda,0.0);
	    break;
	  }
	}
      }
      
      IDtool->CleanNeighborPolar(PHead[i],PTail[i]);
      
      PHead[i] = new PolarList();
      PTail[i] = new PolarList();
      PHead[i]->next = PTail[i];
      PTail[i]->back = PHead[i];
      nowp = DummyHead;
      while(next(nowp)!=DummyTail){
	nowp = next(nowp);
	IDtool->AppendPolarI(nowp->ID,PTail[i],nowp->r,nowp->theta,nowp->lambda,0.0);
      }
      
      IDtool->CleanNeighborPolar(DummyHead,DummyTail);
      
    }
  }
  
}

  
void setSigmaZero(){
  int i,j;
  IDList *now=NULL;
  double varphi,ddv,dsize1,sumarea;
  double dddhval=0.0;
  double localsum=0.0;
  for(i=0;i<numberF;i++){
    
    dsize1 = PT->getParametricA(pV[Face[i][0]],
				pV[Face[i][1]],
				pV[Face[i][2]],
				pU[Face[i][0]],
				pU[Face[i][1]],
				pU[Face[i][2]]);
    PT->setParametricDs(bc[0],point[Face[i][0]],
			point[Face[i][1]],point[Face[i][2]],
			pV[Face[i][0]],pV[Face[i][1]],
			pV[Face[i][2]],dsize1);
    PT->setParametricDt(bc[1],point[Face[i][0]],
			point[Face[i][1]],point[Face[i][2]],
			pU[Face[i][0]],pU[Face[i][1]],
			pU[Face[i][2]],dsize1);
    
    E[i] = PT->InnerProduct(bc[0],bc[0]);
    G[i] = PT->InnerProduct(bc[1],bc[1]);
  }
  for(i=0;i<numberV;i++){
         
    sigma[i]=0.0;
    now = FHead[i];
    varphi=0.0;
    localsum=0.0;
    
  
   
    
    
    
    while(next(now)!=FTail[i]){
      now = next(now);
      varphi += (areaMap3D[now->ID]*(0.5*(E[now->ID]+G[now->ID])));
      localsum += (areaMap3D[now->ID]);
      
      
    }
    
    sigma[i] = sqrt((varphi/localsum));
    
    sigma[i] = pow(sigma[i],gammaP);
    
  
  }
  
  
  
} 
  void setLaplaceC(){
  PolarList *nowp;
  int i;
  
  for(i=0;i<numberV;i++){ 
    if(boundary[i]!=1){
      nowp =PHead[i];
      
      while(next(nowp)!=PTail[i]){
	nowp = next(nowp);
	nowp->lambda = 1.0/((double)(neighborI[i]));
      }
    }
  }
    
}

double getCurrentE(){
  int i,j;
  IDList *now=NULL;
  double varphi,ddv,dsize1,sumarea;
  double dddhval=0.0;
  double dsum=0.0;
  
  
  
  double localsum=0.0;
  double pV1,pV2,pV3,pU1,pU2,pU3;
  double dE,dG;
  dE = 0.0;
  dG = 0.0;
  if(boundarysigma==0){
  for(i=0;i<numberF;i++){
    if(boundary[Face[i][0]]!=1&&boundary[Face[i][1]]!=1&&boundary[Face[i][2]]!=1){
    pV1 = pV[Face[i][0]];
    pV2 = pV[Face[i][1]];
    pV3 = pV[Face[i][2]];
    pU1 = pU[Face[i][0]];
    pU2 = pU[Face[i][1]];
    pU3 = pU[Face[i][2]];
   
    
    dsize1 = PT->getParametricA(pV1,pV2,pV3,pU1,pU2,pU3);
    
   
      
    PT->setParametricDs(bc[0],point[Face[i][0]],
			point[Face[i][1]],point[Face[i][2]],
			pV1,pV2,pV3,dsize1);
    PT->setParametricDt(bc[1],point[Face[i][0]],
			point[Face[i][1]],point[Face[i][2]],
			pU1,pU2,pU3,dsize1);
    dE = PT->InnerProduct(bc[0],bc[0]);
    
    dG = PT->InnerProduct(bc[1],bc[1]);
    dsum +=  areaMap3D[i]*0.5*(dE+dG);
    }
  }
  }else{
    for(i=0;i<numberF;i++){
      
    pV1 = pV[Face[i][0]];
    pV2 = pV[Face[i][1]];
    pV3 = pV[Face[i][2]];
    pU1 = pU[Face[i][0]];
    pU2 = pU[Face[i][1]];
    pU3 = pU[Face[i][2]];
   
    
    dsize1 = PT->getParametricA(pV1,pV2,pV3,pU1,pU2,pU3);
    
   
      
    PT->setParametricDs(bc[0],point[Face[i][0]],
			point[Face[i][1]],point[Face[i][2]],
			pV1,pV2,pV3,dsize1);
    PT->setParametricDt(bc[1],point[Face[i][0]],
			point[Face[i][1]],point[Face[i][2]],
			pU1,pU2,pU3,dsize1);
    dE = PT->InnerProduct(bc[0],bc[0]);
    
    dG = PT->InnerProduct(bc[1],bc[1]);
    dsum +=  areaMap3D[i]*0.5*(dE+dG);
   
  }
  
  
  }
  dsum =constsumarea3D*sqrt(dsum/sumarea3D);
  return dsum;

}

};
