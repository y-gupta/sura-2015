function [ output_args ] = sfs2( input_args )
%UNTITLED Summary of this function goes here
%   Detailed explanation goes here

E = imread(input_args);
% making sure that it is a grayscale image

[albedo I slant tilt] = estimate_albdeo(input_args);

E = double(E);


% initializations ...
[M,N] = size(E);
% surface normals
p = zeros(M,N);
q = zeros(M,N);
% the surface
Z = zeros(M,N);
% surface derivatives in x and y directions
Z_x = zeros(M,N);
Z_y = zeros(M,N);
% maximum number of iterations
maxIter = 200;
% the normalized illumination direction
ix = cos(tilt) * tan(slant);
iy = sin(tilt) * tan(slant);


for k = 1 : maxIter
% using the illumination direction and the currently estimate
% surface normals, compute the corresponding reflectance map. % refer to (57) ...
R=(cos(slant) + p .* cos(tilt)*sin(slant)+ q .* sin(tilt)*sin(slant))./sqrt(1 + p.^2 + q.^2);
% at each iteration, make sure that the reflectance map is positive at % each pixel, set negative values to zero.
R = max(0,R);
% compute our function f which is the deviation of the computed % reflectance map from the original image ...
f = E - R;
% compute the derivative of f with respect to our surface Z ... refer % to (62)
df_dZ =(p+q).*(ix*p + iy*q + 1)./(sqrt((1 + p.^2 + q.^2).^3)* sqrt(1 + ix^2 + iy^2))-(ix+iy)./(sqrt(1 + p.^2 + q.^2)* sqrt(1 + ix^2 + iy^2));
% update our surface ... refer to (61)
Z = Z - f./(df_dZ + eps); % to avoid dividing by zero
% compute the surface derivatives with respect to x and y
          Z_x(2:M,:) = Z(1:M-1,:);
          Z_y(:,2:N) = Z(:,1:N-1);
% using the updated surface, compute new surface normals, refer to (58) % and (59)
p = Z - Z_x;
q = Z - Z_y;
end

% smoothing the recovered surface
      Z = medfilt2(abs(Z),[21 21]);
      % visualizing the result
      figure;
      surfl(Z);
      shading interp;
      colormap gray(256);
      lighting phong;
      
      Z = Z ./ max(Z(:));
      imwrite(Z,'sfs_output.jpg');
     


end

function [output1 output2 output3 output4] = estimate_albdeo(I)
% read the image
      E = double(imread(I));
      % normalizing the image to have maximum of one
      E = E ./ max(E(:));
      % compute the average of the image brightness
      Mu1 = mean(E(:));
% compute the average of the image brightness square
      Mu2 = mean(mean(E.^2));
      
      % now lets compute the image's spatial gradient in x and y directions
[Ex,Ey] = gradient(E);
% normalize the gradients to be unit vectors
Exy = sqrt(Ex.^2 + Ey.^2);
nEx = Ex ./(Exy + eps); % to avoid dividing by zero nEy = Ey ./(Exy + eps);
nEy = Ey ./(Exy + eps);

% computing the average of the normalized gradients
avgEx = mean(Ex(:));
avgEy = mean(Ey(:));

% now lets estimate the surface albedo
gamma = sqrt((6 *(pi^2)* Mu2) - (48 * (Mu1^2)));
albedo = gamma/pi;
% estimating the slant
slant = acos((4*Mu1)/gamma);
% estimating the tilt
tilt = atan(avgEy/avgEx);
if tilt < 0
    tilt = tilt + pi;
end
% the illumination direction will be ...
Illumination = [cos(tilt)*sin(slant) sin(tilt)*sin(slant) cos(slant)];

output1 = albedo;
output2 = Illumination ;
output3 = slant;
output4 = tilt;

end



