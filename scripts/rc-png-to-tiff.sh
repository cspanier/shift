#!/bin/bash

# This little script properly converts PNG input files to TIFF format.

# Albedo color textures should all be stored in sRGB format.
for i in $(find resources/textures/freepbr.com/ -name "*-albedo.png"); do
  echo $i
  mogrify -format tiff -compress zstd -colorspace sRGB -set colorspace:auto-grayscale off $i
done
# All other textures store data for which sRGB doesn't make much sense.
# However, many textures still have a sRGB color profile assigned. These
# commands will assume linear input and output and strip meaningless ICC
# profiles from the images.
for i in $(find resources/textures/freepbr.com/ -name "*-ao.png"); do
  echo $i
  mogrify -format tiff -compress zstd -set colorspace RGB -colorspace RGB -alpha off -strip $i
done
for i in $(find resources/textures/freepbr.com/ -name "*-height.png"); do
  echo $i
  mogrify -format tiff -compress zstd -set colorspace RGB -colorspace RGB -alpha off -strip $i
done
for i in $(find resources/textures/freepbr.com/ -name "*-metalness.png"); do
  echo $i
  mogrify -format tiff -compress zstd -set colorspace RGB -colorspace RGB -alpha off -strip $i
done
for i in $(find resources/textures/freepbr.com/ -name "*-normal.png"); do
  echo $i
  mogrify -format tiff -compress zstd -set colorspace RGB -colorspace RGB -alpha off -strip $i
done
for i in $(find resources/textures/freepbr.com/ -name "*-roughness.png"); do
  echo $i
  mogrify -format tiff -compress zstd -set colorspace RGB -colorspace RGB -alpha off -strip $i
done

# Change image compression:
#   -compress (lzw|zip|zstd|...)
# Assume input to be linear instead of sRGB:
#   -set colorspace RGB
# Assume input signed integer:
#   -define quantum:format=signed
# Assume input float:
#   -define quantum:format=floating-point

# Output linear instead of sRGB pixels:
#   -colorspace RGB
# Remove all ICC color profiles from output:
#   -strip
# Remove alpha channel:
#   -alpha off
# Enable/Disable automatic grayscale colorspace if input image has only gray pixels (R==G==B):
#   -set colorspace:auto-grayscale on|off
