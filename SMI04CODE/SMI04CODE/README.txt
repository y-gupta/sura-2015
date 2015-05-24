A fast and simple stretch-minimizing mesh parameterization C++ code

************************
Simple Manual
Steps

1)Use Convert_obj_to_my_input_format.py <input>.obj <output>.ply2 to convert obj to ply2
2)run ./Parameterization <input_mesh>.ply2 <output_mesh>.ply2 
3)Use Convert_output_to_my_input_format.py <output>.ply2 <output_mesh>.ply2 <output>.obj to get the final obj with texture coordinates

Compile: using “Make”
Execute: Run “./Parameterization <input_mesh> <output_mesh>


You can use the following options by changing the code in the constructor "Polyhedron()" of "Polyhedron.h". Also you should clean object file "make clean" then "make". The default setting is as follows:

      Polyhedron(){
      pickID=-1;
      paramtype=2;
      boundarytype=0;
      weighttype=0;
      iteNum=2000;
      gammaP=1.0;
      PCBCGerror=pow(0.1,6.0);
      smooth=1;
      intrinsiclambda=0.5;
      boundarysigma=1;
      }

