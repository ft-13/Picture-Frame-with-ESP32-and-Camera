In Windows terminal with free software tool "magick"

convert saved raw byte on SD card to a jpg file:

convert raw bytes to jpg file:
magick -depth 8 -size 1024x768 gray:4.txt  result.jpg


For jpg files to display on picture frame we have to convert it to raw bytes
Follow these steps:

1. convert jpg to grayscale:
magick convert my-image.jpg -colorspace Gray my-image-gray.jpg

2. Resize picture (! = force):
magick convert name.jpg   -resize 1024x768!  resize_name.jpg

3. convert to raw bytes:
magick convert resize_name.jpg -depth 8 image.gray

4. rename image.gray to image.txt

5. copy file image.txt to sd root folder


Check output file:
magick identify image.gray