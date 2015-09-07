all: colorgen mapgen scale

colorgen: colorgen.cpp common.h stb-image.h
	g++ colorgen.cpp -O2 -std=c++11 -o colorgen.out
mapgen: mapgen.cpp common.h stb-image.h
	g++ mapgen.cpp -O2 -std=c++11 -o mapgen.out
scale:
	g++ scale.cpp -O2 -std=c++11 -o scale.out
