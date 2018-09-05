#!/bin/sh
#dest=/media/gisiv02/VCI_upd/products/VCI_smo/Day$2
dest=/tmp
src=/media/gisiv02/VCI_upd/products/VCI/16days/$1

echo 'Year '$1
echo 'Day '$2
echo 'Kernel Size '$3

mkdir $dest

#timing starts
START=$(date +%s)
#
./VCI_medX_glob.v6.x $src/vci.$1.$2.raw.tif $dest/med$3.$1.$2.tif $3
#
##step 3: compress
/opt/local/bin/gdal_translate -co compress=DEFLATE $dest/med$3.$1.$2.tif $src/mf$3.$1.$2.tif
#
rm $dest/med$3.$1.$2.tif

/media/gisev01/Chunming/home/program/upd/setProjNGeom.x /media/gisiv01/products/VCI/16days/$1/vci.$1.$2.tif /media/gisiv01/products/VCI_smoothed/16days/$1/mf$3.$1.$2.tif

#step 4: pyramid

/opt/local/bin/gdaladdo -ro --config USE_RRD YES --config BIGTIFF_OVERVIEW YES --config COMPRESS_OVERVIEW DEFLATE \
$src/mf$3.$1.$2.tif 2 4 8 16 32 64

#rm $old/mf$3.$1.$2.axe
#
#timing ends
END=$(date +%s)
DIFF=$(( $END - $START ))
echo "For a global tiff, it took $DIFF seconds to do median filter "$3" by "$3"."
