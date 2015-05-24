from __future__ import unicode_literals
import sys
# -*- coding: utf-8 -*-
print sys.argv

f = open(sys.argv[1],'r')
g = open(sys.argv[2],'w')
for lines in f:
	r = lines.split()
	if(r[0] == "v"):
		g.write(lines)
	elif(r[0] == "f"):
		g.write(lines)
g.close()
f.close()