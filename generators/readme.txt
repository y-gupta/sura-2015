# Readme

This project performs the following tasks:
 - colorgen: generates color-space appearance manifold for material in a given photograph and control points with known corrosion values
 - The generated image can be interpret as color of 0% corroded material at the top of image and 100% at the bottom.
 - mapgen: This utility takes in several gradients as input appearance manifold, a noise image to enhance corrosion values, and actual corrosion degree map. In the given map, white is treated as 100%corroded and black as 0% corroded.

# Compiling

make all

# Using

This project generated two executables, colorgen.out and mapgen.out

## colorgen.out:

./colorgen.out <path to input image> < <path to ctrl-points txt file>

or if you want to enter the control points manually through keyboard, use

./colorgen.out <path to input image>

The output imageis named "cor_out.bmp" and is generated in present directory. This image serves the purpose of <base>-color.bmp in the mapgen utility. Rename it accordingly to use in next step.
ctrl-points follow the following syntax:
<x-coord> <y-coord> <corrosion degree on a scale from 0 to 1>

## mapgen.out:

./mapgen.out <base-path>

As this utility requires multiple input files, the following structure is imposed on the input files:

<base-path>-base.bmp      <- The corrosion degree map
<base-path>-noise.bmp     <- the noise to be used
<base-path>-color.bmp     <- the color gradient
<base-path>-metal.bmp     <- the metallicity gradient
<base-path>-smooth.bmp    <- the smoothness gradient
<base-path>-disp.bmp      <- the dist gradient
<base-path>-depth.bmp     <- the depth gradient

Sample images for all of these are included. Output images generated are:

<base-path>-out-color.bmp     <- color/diffuse/albedo map to be used for rendering
<base-path>-out-base.bmp      <- The actual corrosion degree values used, after applying noise
<base-path>-out-metal.bmp     <- metal map to be used in unity (R channel = metallicity, B/A channel = smoothness, G channel = 0)
<base-path>-out-smooth.bmp    <- identical to metal map.
<base-path>-out-disp.bmp      <- displacement map
<base-path>-out-depth.bmp     <- depth map

The resolution of all these maps is same as <base-path>-base.bmp.
Note that <base-path> has to be replaced with actual base path. For example, images/sample-base.bmp has <base-path> = images/sample


# Example

To generate rust maps:

./colorgen.out rust.bmp < ctrl-points-rust.txt
mv cor_out.bmp images/rust-color.bmp
./mapgen.out images/rust

