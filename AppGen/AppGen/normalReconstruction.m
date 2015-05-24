function [intensity, output_args ] = normalReconstruction( inputImage,lightDirection )
%presently take [1,-1,1]
%UNTITLED Summary of this function goes here
%   Detailed explanation goes here

I = imread(inputImage);
m = size(I,1);
n = size(I,2);
lambda = 0.0001;
I  = double(I);
for i=1:m
    for j=1:n
        I(i,j) = I(i,j)/255;
    end
end

A = zeros(m*n*3,m*n*3);
A = double(A);
X = zeros(m*n*3,1);
X = double(X);
b = zeros(m*n*3,1);
b = double(b);
disp('done');

intensity = I;

for i=1:(m*n)
    
        disp(i);
        x = (i-1)*3+1+0;
        
        
        A(x,x) = lightDirection(1)*lightDirection(1) + 4*lambda;
        A(x,x+1) = lightDirection(1)*lightDirection(2);
        A(x,x+2) = lightDirection(1)*lightDirection(3);
        
        p = ((i+1) - 1)*3+1;
        if (p <= m*n*3 && p > 0) 
            A(x,p) = -1*lambda; 
        end
        
        p = ((i-1) - 1)*3+1;
        if (p <= m*n*3 && p > 0) 
            A(x,p) = -1*lambda; 
        end
        p = ((i+n) - 1)*3+1;
        if (p <= m*n*3 && p > 0) 
            A(x,p) = -1*lambda; 
        end
        p = ((i-n) - 1)*3+1;
        if (p <= m*n*3 && p > 0) 
            A(x,p) = -1*lambda; 
        end
        
        s = mod(i,n);
        if s==0
            s = n;
        end
        t = ceil(double(i)/double(n));
        b(x,1) = I(s,t)*lightDirection(1);
        
        
       %--------------------------------------------------------

        x = (i-1)*3+1+1;
        
        
        A(x,x) = lightDirection(2)*lightDirection(2) + 4*lambda;
        A(x,x-1) = lightDirection(2)*lightDirection(1);
        A(x,x+1) = lightDirection(2)*lightDirection(3);
        
        p = (i+1 - 1)*3+1+1;
        if (p <= m*n*3 && p > 0) 
            A(x,p) = -1*lambda; 
        end
        
        p = (i-1 - 1)*3+1+1;
        if (p <= m*n*3 && p > 0) 
            A(x,p) = -1*lambda; 
        end
        p = (i+n - 1)*3+1+1;
        if (p <= m*n*3 && p > 0) 
            A(x,p) = -1*lambda; 
        end
        p = (i-n - 1)*3+1+1;
        if (p <= m*n*3 && p > 0) 
            A(x,p) = -1*lambda; 
        end
        
        s = mod(i,n);
        if s==0
            s = n;
        end
        t = ceil(double(i)/double(n));
        b(x,1) = I(s,t)*lightDirection(2);


        %------------------------------------------------
         x = (i-1)*3+1+2;
        
        
        A(x,x) = lightDirection(3)*lightDirection(3) + 4*lambda;
        A(x,x-1) = lightDirection(3)*lightDirection(2);
        A(x,x-2) = lightDirection(3)*lightDirection(1);
        
        p = (i+1 - 1)*3+1+2;
        if (p <= m*n*3 && p > 0) 
            A(x,p) = -1*lambda; 
        end
        
        p = (i-1 - 1)*3+1+2;
        if (p <= m*n*3 && p > 0) 
            A(x,p) = -1*lambda; 
        end
        p = (i+n - 1)*3+1+2;
        if (p <= m*n && p > 0) 
            A(x,p) = -1*lambda; 
        end
        p = (i-n - 1)*3+1+2;
        if (p <= m*n*3 && p > 0) 
            A(x,p) = -1*lambda; 
        end
        
        
        s = mod(i,n);
        if s==0
            s = n;
        end
        t = ceil(double(i)/double(n));
        b(x,1) = I(s,t)*lightDirection(3);

    
end



[ X, err, iter, flag ] = sor(A,X,b,1,100,1e-4);
X(1),X(2),X(3)
N = zeros(m,n,3);
N = double(N);
for i=1:m*n
    s = mod(i,n);
    if s==0
        s = n;
    end
    t = ceil(double(i)/double(n));

    for j=0:2
        e = (i-1)*3 + 1 + j;
        N(s,t,j+1) = X(e);
        N(s,t,j+1) = N(s,t,j+1);
    end
    

end

%image(N);
disp(err);
disp(iter);
disp(flag);
output_args = N;
for i=1:m
    for j=1:n
        N(i,j,1) = (N(i,j,1)/2) + 0.5;
        N(i,j,2) = (N(i,j,2)/2) + 0.5;
        N(i,j,3) = (N(i,j,3)/2) + 0.5;
    end
end
imshow(N);

end

