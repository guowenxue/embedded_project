#!/bin/sh

CROSS=/opt/buildroot-eabi/staging/usr/bin/arm-linux-
#PNGPATH=/home/guowenxue/signpad/src/rootfs/application/signpad/libpng-1.4.3
#PNGPATH=/home/guowenxue/signpad/src/rootfs/application/signpad/libpng-1.4.0
PNGPATH=/home/guowenxue/signpad/src/rootfs/application/signpad/libpng-1.2.44
ZLIBPATH=/home/guowenxue/signpad/src/rootfs/application/signpad/zlib-1.2.5/

#:<<BLOCK
export CPPFLAGS="-I${PNGPATH} -I${ZLIBPATH}"
export LDFLAGS="-L${PNGPATH}/.libs -L${ZLIBPATH} "
echo "CPPFLAGS: $CPPFLAGS"
echo "LDFLAGS: $LDFLAGS"
#BLOCK
export CC=${CROSS}gcc
export AR=${CROSS}ar 
export AS=${CROSS}as
export LD=${CROSS}ld  
export STRIP=${CROSS}strip
export CPP=${CROSS}cpp 
export CXX=${CROSS}g++
./configure --host=arm-linux --prefix=/apps/imageMagick --disable-openmp --disable-opencl --disable-ltdl-install --disable-largefile --disable-deprecated --disable-cipher --disable-embeddable --disable-hdri --disable-maintainer-mode --disable-ccmalloc --disable-efence --disable-prof --disable-gprof --disable-gcov  --without-magick-plus-plus --with-perl=no --without-umem --without-dmalloc --without-included-ltdl --without-bzlib --without-x --without-dps --without-djvu --without-fftw  --without-fpx --without-gslib --without-gvc --without-jbig --without-jpeg --without-jp2 --without-lcms --without-lcms2  --without-lqr --without-openexr  --without-rsvg --without-tiff --without-wmf --with-png=yes --with-zlib=yes 
make && make install
