#!/bin/sh
 g++ -I. -I/usr/include -I/usr/include/hdf -I/usr/include/gdal/ \
      -L/usr/lib -L/opt/local/lib/ $1.cpp \
      -lgdal -lhdfeos -lmfhdfalt  -lgctp -ldfalt -lsz -ljpeg -lz -lm -o $1.x
