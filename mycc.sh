#!/bin/sh
 g++ -I. -I/usr/include \
      -L/usr/lib -L/opt/local/lib/ $1.cpp \
      -o $1.x
