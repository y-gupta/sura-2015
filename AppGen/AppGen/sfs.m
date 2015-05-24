function [ output_args ] = sfs( inputImage )
%UNTITLED Summary of this function goes here
%   Detailed explanation goes here

[albedo I slant tilt] = estimate_albdeo(inputImage);

% read the image
    E = imread(inputImage);
% making sure that it is a grayscale image
      % downsampling to speedup
      E = E(1:2:end,1:2:end);
      E = double(E);
      % normalizing the image to have maximum of one
      E = E ./ max(E(:));
% first compute the surface albedo and illumination direction ...
%[albedo I] = estimate_albedo(inputImage);
% Initializations ...
[M,N] = size(E);
% surface normals
p = zeros(M,N);
q = zeros(M,N);
% the second order derivatives of surface normals
p_ = zeros(M,N);
q_ = zeros(M,N);
% the estimated reflectance map
R = zeros(M,N);
% the controling parameter
lamda = 1000;
% maximum number of iterations
maxIter = 2000;
% The filter to be used to get the second order derivates of the surface % normals.
w = 0.25*[0 1 0;1 0 1;0 1 0]; % refer to equations (39) and (40)
      % wx and wy in (47) and (48)
      [x,y] = meshgrid(1:N,1:M);
      wx = (2.* pi .* x) ./ M;
      wy = (2.* pi .* y) ./ N;
      
      
      for k = 1 : maxIter
% compute the second order derivates of the surface normals p_ = conv2(p,w,'same');
q_ = conv2(q,w,'same');
% Using the computed surface albedo, illumination direction and
% surface normals, compute estimation for the reflectance map.
% refer to (16)
R = (albedo.*(-I(1).* p - I(2).* q + I(3)))./sqrt(1 + p.^2 + q.^2);
% Compute the partial derivatives of the reflectance map with respect % to p and q. it will be the differenation of (16) with respect to p % and q
pq = (1 + p.^2 + q.^2);
dR_dp = (-albedo*I(1) ./ (pq .^(1/2))) + (-I(1) * albedo .* p - I(2) * albedo .* q + I(3) * albedo) .* (-1 .* p .* (pq .^(-3/2)));
dR_dq = (-albedo*I(2) ./ (pq .^(1/2))) + (-I(1) * albedo .* p - I(2) * albedo .* q + I(3) * albedo) .* (-1 .* q .* (pq .^(-3/2)));
% Compute the newly estimated surface normals ... refer to (41) and % (42)
p = p_ + (1/(4*lamda))*(E-R).*dR_dp;
q = q_ + (1/(4*lamda))*(E-R).*dR_dq;
% Compute the Fast Fourier Transform of the surface normals.
    Cp = fft2(p);
    Cq = fft2(q);
% Compute the Fourier Transform of the surface Z from the Fourier % Transform of the surface normals ... refer to (46) ...
C = -i.*(wx .* Cp + wy .* Cq)./(wx.^2 + wy.^2);
% Compute the surface Z using inverse Fourier Transform ... refer to % (45)
Z = abs(ifft2(C));
% Compute the integrable surface normals .. refer to (47) and (48)
    p = ifft2(i * wx .* C);
    q = ifft2(i * wy .* C);
    % saving intermediate results
if (mod(k,100) == 0)
save(['Z_after_' num2str(k) '_iterations.mat'],'Z');
end
      end
% visualizing the result
figure;
surfl(Z);
shading interp;
colormap gray(256);
lighting phong;




end

function [output1 output2] = estimate_albdeo(I)
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

