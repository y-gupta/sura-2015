CS7495 Final Project
Sooraj Bhat

Object Removal by Exemplar-based Inpainting


USING THE CODE

Everything was done in Matlab and MEX (i.e. a C function callable from
Matlab).  First, the C code needs to be compiled.  At the Matlab
prompt, type:

   mex bestexemplarhelper.c

You should then be able to run the following (which takes ~2mins on my
2GHz Pentium 4 laptop with 256MB of RAM):

   [i1,i2,i3,c,d,mov] = inpaint('bungee0.png','bungee1.png',[0 255 0]);

A smaller run (~25 seconds) would be:

   [i1,i2,i3,c,d,mov] = inpaint('bw0.png','bw2.png',[0 255 0]);

To visualize:

   plotall;
   close; movie(mov);


SAMPLE IMAGES & RESULTS

*0.png	      The original images ('bw' and 'bungee')
*{1,2}.png    The different fill region masks.
*P2.png	      Plots of confidence and data terms.
*P5.png	      Plots of original, fill region, inpainted images and
              confidence and data terms. 

Other images in the dir contain redundant information.  I didn't
include the movies since they are big, but they are available on my
project page (AVI format).
