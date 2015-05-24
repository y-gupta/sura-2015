import sys

g = open(sys.argv[1],'r')
l = open(sys.argv[2],'r')
h = open(sys.argv[3],'w')


vertices	= g.readline()
faces 		= g.readline()

vertices 	= int(vertices)
faces 		= int(faces)

for t in range(0,vertices):
	x = g.readline();
	y = g.readline();
	z = g.readline();
	x = float(x);
	y = float(y);
	z = float(z);
	s = "v" + " " + str(x) + " "+ str(y) + " " + str(z) + "\n"
	h.write(s)
for t in range(0,vertices):
	x = l.readline();
	h.write(s)


for t in range(0,faces):
	d = g.readline();
	x = g.readline();
	y = g.readline();
	z = g.readline();
	x = int(x) + 1;
	y = int(y) + 1;
	z = int(z) + 1;
	s = "f" + " " + str(x) + "/" + str(x) + " "+ str(y)+ "/" +str(y) + " " + str(z)+ "/" +str(z) + "\n"
	h.write(s)


# for lines in g:
	
# 	if len(r) == 3:
# 		s = "v" + " " + r[0] + " "+ r[1] + " " + r[2] + "\n";
# 		h.write(s)
# 	elif len(r) == 4:
# 		s = "f" + " " + str(int(r[1])+1) + "/" + str(int(r[1])+1) + " "+ str(int(r[2])+1) + "/"+ str(int(r[2])+1) + " "+str(int(r[3])+1) +"/"+ str(int(r[3])+1) + "\n";
# 		h.write(s)
g.close()
h.close()