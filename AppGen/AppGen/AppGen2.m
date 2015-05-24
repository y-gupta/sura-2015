function [ output_args ] = AppGen2( inputImage,windowSize,iterations )


I = imread(inputImage);
m = size(I,1);
n = size(I,2);
albedo = zeros(m,n);
albedo = double(albedo);


imgIntensity = formIntensity(I);
imgChroma = formChroma(I,imgIntensity);
[omegaIndex indexArray omegaPop] = formOmega(I,windowSize);
 
 disp 'done';

%---------------Expectation Step--------------------


omega = size(omegaIndex,1);
pixels = size(indexArray,1);

expec = zeros(omega,400);
expec = double(expec);
cnt = zeros(400);
albedoFinal = zeros(m,n,3);
albedoFinal = uint8(albedoFinal);

for i=1:m
    for j=1:n
        albedo(i,j) = imgIntensity(i,j);
    end
end


for steps =1:iterations
    disp (steps);
    for i=1:omega
        for j=1:400
            expec(i,j) = 0.0;
        end
    end

for i=1:omega
    
    for k=1:400
        cnt(k) = 0;
    end
    val = 0.0;
    x = omegaIndex(i,1);
    y = omegaIndex(i,2);
    for j=0:windowSize-1
        for k=0:windowSize-1
            p = x+j;
            q = y+k;
            a = imgChroma(p,q,1);
            b = imgChroma(p,q,2);
            % c = imgChroma(p,q,3); not needed presently
            chromaIndex = chromaValue(a,b);
            expec(i,chromaIndex) = expec(i,chromaIndex) + imgIntensity(p,q);
            cnt(chromaIndex) = cnt (chromaIndex) + 1;
            val = val + (imgIntensity(p,q)/albedo(p,q));
        end
    end
    
    den = val / 400;
    for j = 1:400
        if cnt(j) == 0
            num = 0.0;
        else
            num = expec(i,j)/cnt(j);
        end
       
        expec(i,j) = num/den;
       % disp (expec(i,j));
    end
    
    
end
% 
% %---------Maximization Step--------------------

for i =1:pixels
    
    sum = 0.0;
    total = 0.0;
    p = indexArray(i,1);
    q = indexArray(i,2);

    for j=1:windowSize*windowSize
        if omegaPop(i,j) == -1
            break;
        else
            a = imgChroma(p,q,1);
            b = imgChroma(p,q,2);
            % c = imgChroma(p,q,3); not needed presently
            chromaIndex = chromaValue(a,b);
            
            omindex = omegaPop(i,j);
            sum = sum + expec(omindex,chromaIndex);
            total = total + 1;
        end
    end
    val = sum/total;
    albedo(p,q) = val;
    
end



for i=1:m
    for j=1:n
        albedoFinal(i,j,1) = uint8(albedo(i,j)*imgChroma(i,j,1));
        albedoFinal(i,j,2) = uint8(albedo(i,j)*imgChroma(i,j,2));
        albedoFinal(i,j,3) = uint8(albedo(i,j)*imgChroma(i,j,3));
    end
end
 disp(albedoFinal(1,1));
 %imshow(albedoFinal);
 %drawnow;

end
% 


%albedoFinal
shading = zeros(m,n);
shading = double(shading);
for i=1:m
    for j=1:n
        shading(i,j) = imgIntensity(i,j)/albedo(i,j);
    end
end

imwrite(albedoFinal,'output.jpg');
imwrite(shading,'shading.jpg');



end

function [output_args] = formIntensity(I)

m = size(I,1);
n = size(I,2);

imgIntensity = zeros(m,n);

imgIntensity = double(imgIntensity);

for i = 1:m
    for j =1:n
        a = double(I(i,j,1));
        b = double(I(i,j,2));
        c = double(I(i,j,3));
        imgIntensity(i,j) = (a + b + c) / 3.0;
    end
end

output_args = imgIntensity;

end

function [output_args,o,pl] = formOmega(I,w)

m = size(I,1);
n = size(I,2);

indexarray = zeros(m*n,2);
cnt = 1;
for i = 1:m
    for j=1:n
        indexarray(cnt,1) = i;
        indexarray(cnt,2) = j;
        cnt = cnt + 1;
    end
end


OmegaIndex = zeros((m-(w-1)*(n-(w-1))),2);
OmegaCnt = ones(m*n);
OmegaPop = zeros(m*n,w*w);
cnt = 1;

for i=1:m*n
    for j=1:w*w
        OmegaPop(i,j) = -1;
    end
end


for i=1:m-(w-1)
    for j=1:n-(w-1)
        OmegaIndex(cnt,1) = i;
        OmegaIndex(cnt,2) = j;
        
        for a = 0:19
            for b=0:19
                ind = (i+a-1)*n + (j+b);
                OmegaPop(ind,OmegaCnt(ind)) = cnt;
                OmegaCnt(ind) = OmegaCnt(ind) + 1;
            end
        end
        cnt = cnt +1;
    end
end


output_args = OmegaIndex;
o = indexarray;
pl = OmegaPop;
end

%--------------------------------------

function [output_args] = formChroma(I,imgIntensity)
    

m = size(I,1);
n = size(I,2);


imgChroma = zeros(m,n,3);

imgChroma = double(imgChroma);

val = 0.0;

for i=1:m
    for j=1:n
        imgChroma(i,j,1) = (double(I(i,j,1)))/imgIntensity(i,j);
        imgChroma(i,j,2) = (double(I(i,j,2)))/imgIntensity(i,j);
        imgChroma(i,j,3) = 3.0 - (imgChroma(i,j,1) + imgChroma(i,j,2));
        val = max(val,max(imgChroma(i,j,1),imgChroma(i,j,2)));
    end
    
end
val
output_args = imgChroma;


end

function [w] = chromaValue(a,b)

p = (a*100.0)/10.0;
q = (b*100.0)/10.0;

x = max(1,ceil(p));
y = max(1,ceil(q));

w = (x-1)*20+y;



end



