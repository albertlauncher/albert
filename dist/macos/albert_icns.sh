#!/bin/sh

input_path=../../resources/albert.svg
output_path=output.iconset
mkdir -p "$output_path"

# the convert command comes from imagemagick
for size in 32 64 128 256; do
  double="$(($size * 2))"
  convert -background none -density $size -resize x$size ${input_path} $output_path/icon_${size}x${size}.png
  convert -background none -density $size -resize x$double ${input_path} $output_path/icon_${size}x${size}@2x.png
done

iconutil -c icns $output_path -o albert.icns
