from __future__ import unicode_literals
import sys
# -*- coding: utf-8 -*-
print sys.argv

f = open(sys.argv[1],'r')
g = open(sys.argv[2],'w')
for lines in f:
	r = lines.split()
	if(r[0] == "v"):
		s = r[1]+"\n"+r[2]+"\n" +r[3]+"\n"
		g.write(s)
	elif(r[0] == "f"):
		s = "3" + "\n"+str(int(r[1])-1)+"\n"+str(int(r[2])-1)+"\n"+str(int(r[3])-1)+"\n"
		g.write(s)
g.close()
f.close()