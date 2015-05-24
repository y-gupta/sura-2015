function [ output_args ] = AppGen(inpImg,outputImg)

I = imread(inpImg);
I = double(I);
m = size(I,1);
n = size(I,2);

for i=1:m
    for j=1:n
        sum = I(i,j,1)+I(i,j,2)+I(i,j,3);
        sum = sum/3;
        if(sum > 230.0 || sum < 25.0)
            I(i,j,1) = 0.0;
            I(i,j,2) = 255.0;
            I(i,j,3) = 0.0;
        end
        %I(i,j,1) = I(i,j,1)/255.0;
        %I(i,j,2) = I(i,j,2)/255.0;
        %I(i,j,3) = I(i,j,3)/255.0;
    end
end
I = uint8(I);

imwrite(I,outputImg);

output_args = I;

end

