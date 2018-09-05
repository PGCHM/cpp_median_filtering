#!/bin/sh
#dest=/media/gisiv01/products/VCI_smoothed/16days/$1
dest=/media/gisiv02/VCI_upd/products/VCI_smo/$1.$2
#dest=/tmp
src=/media/gisiv01/products/VCI/16days/$1
#src=/media/gisiv02/VCI_upd/products/VCI/16days/$1

echo 'Year '$1
echo 'Day '$2
echo 'Kernel Size '$3

mkdir $dest

#timing starts
START=$(date +%s)
#
mkdir $dest
#./VCI_medX_glob.v6.x $src/vci.$1.$2.raw.tif $dest/med$3.$1.$2.tif $3
./VCI_medX_glob.v6.x $src/vci.$1.$2.tif $dest/med$3.$1.$2.tif $3
#./VCI_medX_glob.x /media/usb0/test/VCI-US/US.vci.$1.$2.tif /media/usb0/Chunming/smooth/US.vci.med$3.$1.$2.v3.tif $3
#
##step 3: compress
/opt/local/bin/gdal_translate -co compress=DEFLATE $dest/med$3.$1.$2.tif $dest/mf$3.$1.$2.tif
#
rm $dest/med$3.$1.$2.tif

mv $dest/*mf9* /media/gisiv01/products/VCI_smoothed/16days/$1/

#step 4: pyramid

/opt/local/bin/gdaladdo -ro --config USE_RRD YES --config BIGTIFF_OVERVIEW YES --config COMPRESS_OVERVIEW DEFLATE \
/media/gisiv01/products/VCI_smoothed/16days/$1/mf$3.$1.$2.tif 2 4 8 16 32 64

#/opt/local/bin/gdaladdo -ro --config USE_RRD YES $dest/mf$3.$1.$2.tif 2 4 8 16 32 64
#/opt/local/bin/gdaladdo -ro --config USE_RRD YES $dest/med$3.$1.$2.tif 2 4 8 16 32 64
#
#rm $dest/mf$3.$1.$2.axe
#rm /media/gisiv01/products/VCI_smoothed/16days/$1/mf$3.$1.$2.axe
#
/media/gisev01/Chunming/home/program/upd/setProjNGeom.x /media/gisiv01/products/VCI/16days/$1/vci.$1.$2.tif /media/gisiv01/products/VCI_smoothed/16days/$1/mf$3.$1.$2.tif
#mv $dest/*mf9* /media/gisiv01/products/VCI_smoothed/16days/$1/
#timing ends
END=$(date +%s)
DIFF=$(( $END - $START ))
echo "For a global tiff, it took $DIFF seconds to do median filter "$3" by "$3"."
#
#END
#
