# Makefile

CXX        = /usr/bin/g++ 
all:	IDSet\
	PointTool\
	PCBCGSolver\
	Parameterization\

PCBCGSolver: PCBCGSolver.cxx
	$(CXX) -c PCBCGSolver.cxx
IDSet: IDSet.cxx
	$(CXX) -c IDSet.cxx
PointTool: PointTool.cxx
	$(CXX) -c PointTool.cxx
Parameterization: Parameterization.cxx
	$(CXX) -o Parameterization Parameterization.cxx IDSet.o PointTool.o PCBCGSolver.o -lm

clean:
	-rm -f *.o Parameterization





