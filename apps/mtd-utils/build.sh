#!/bin/sh

#+--------------------------------------------------------------------------------------------
#|Description:  This shell script used to download lzo,zlib,mtd-utils source code
#|              and cross compile it for ARM Linux, all is static cross compile.
#|     Author:  GuoWenxue <guowenxue@gmail.com>
#|  ChangeLog:
#|           1, Initialize 1.0.0 on 2011.04.12
#+--------------------------------------------------------------------------------------------

PRJ_PATH=`pwd`
CROSS=arm-linux-

CC=${CROSS}gcc
AR=${CROSS}ar
LD=${CROSS}ld
STRIP=${CROSS}strip
NM=${CROSS}nm
RANLIB=${CROSS}ranlib
OBJDUMP=${CROSS}objdump

export CC=${CC}
export AR=${AR}
export LD=${LD}
export STRIP=${STRIP}
export NM=${NM}
export RANLIB=${RANLIB}
export OBJDUMP=${OBJDUMP}

LZO="lzo-2.04"
ZLIB="zlib-1.2.5"

ARCH=$1

ARM920T_CROSS="/opt/buildroot-2011.02/arm920t/usr/bin/arm-linux-"
ARM926T_CROSS="/opt/buildroot-2011.02/arm926ejs/usr/bin/arm-linux-"
CROSS=

sup_arch=("" "s3c2410" "s3c2440" "at91sam9260")

function select_arch()
{
   echo "Current support ARCH: "
   i=1
   len=${#sup_arch[*]}

   while [ $i -lt $len ]; do
     echo "$i: ${sup_arch[$i]}"
     let i++;
   done

   echo "Please select: "
   index=
   read index
   ARCH=${sup_arch[$index]}
}

function decompress_packet()
(
   echo "+---------------------------------------------+"
   echo "|  Decompress $1 now"  
   echo "+---------------------------------------------+"

    ftype=`file "$1"`
    case "$ftype" in
       "$1: Zip archive"*)
           unzip "$1" ;;
       "$1: gzip compressed"*)
           if [ `expr "$1" : ".*.tar.*" ` ] ; then
               tar -xzf $1
           else
               gzip -d "$1"
           fi ;;
       "$1: bzip2 compressed"*)
           if [ `expr "$1" : ".*.tar.*" ` ] ; then
               tar -xjf $1
           else
               bunzip2 "$1"
           fi ;;
       "$1: POSIX tar archive"*)
           tar -xf "$1" ;;
       *)
          echo "$1 is unknow compress format";;
    esac
)

if [ -z $ARCH ] ; then
  select_arch
fi

if [ "s3c2440" = $ARCH  -o "s3c2410" = $ARCH ] ; then
    CROSS="$ARM920T_CROSS"
elif [ $ARCH = "at91sam9260" ] ; then
    CROSS="$ARM926T_CROSS"
else
    echo "+------------------------------------------------------------------+"
    echo "|  ERROR: Unsupport platform to cross compile "  
    echo "+------------------------------------------------------------------+"
    exit -1;
fi

echo "+----------------------------------------+"
echo "|  Cross compile $LZO now "  
echo "| Crosstool:  $CROSS"
echo "+----------------------------------------+"

# Download lzo source code packet
if [ ! -s $LZO.tar.gz ] ; then
   wget http://www.oberhumer.com/opensource/lzo/download/$LZO.tar.gz
fi

# Decompress lzo source code packet
if [ ! -d $LZO ] ; then
    decompress_packet $LZO.tar.*
fi

# Cross compile lzo

cd  $LZO
if [ ! -s src/.libs/liblzo*.a ] ; then
unset LDFLAGS
./configure  --host=arm-linux --enable-static --disable-shared CC=${CROSS}gcc AR=${AR} LD=${LD} \
NM=${NM} RANLIB=${RANLIB} STRIP="${STRIP}" OBJDUMP=${OBJDUMP}
make
fi
cd  -


echo "+----------------------------------------+"
echo "|  Cross compile $ZLIB now "  
echo "| Crosstool:  $CROSS"
echo "+----------------------------------------+"

# Download zlib source code packet
if [ ! -s $ZLIB.tar* ] ; then
#wget http://www.zlib.net/$ZLIB.tar.gz
   wget http://www.imagemagick.org/download/delegates/$ZLIB.tar.bz2
fi

# Decompress zlib source code packet
if [ ! -d $ZLIB ] ; then
    decompress_packet $ZLIB.tar.*
fi

#Cross compile zlib

cd  $ZLIB
if [ ! -s libz.a ] ; then
unset LDFLAGS
./configure  --static 
make
fi
cd  -



echo "+----------------------------------------+"
echo "|  Cross compile mtd-utils now "  
echo "| Crosstool:  $CROSS"
echo "+----------------------------------------+"

if [ -s mtd-utils.tar.* ] ; then
    decompress_packet mtd-utils.tar.*
fi

# download mtd-utils source code
if [ ! -d  mtd-utils ] ; then
   git clone git://git.infradead.org/mtd-utils.git
fi

#Add the CROSS tool in file mtd-utils/common.mk
head -1 mtd-utils/common.mk | grep "CROSS=arm-linux-"
if [ 0 != $? ] ; then
   echo "Modify file mtd-utils/common.mk"
   sed -i -e 1i"CROSS=arm-linux-" mtd-utils/common.mk
fi
cd mtd-utils
unset LDFLAGS
export CFLAGS="-DWITHOUT_XATTR -I$PRJ_PATH/$ZLIB -I$PRJ_PATH/$LZO/include"
export ZLIBLDFLAGS=-L$PRJ_PATH/$ZLIB
export LZOLDFLAGS=-L$PRJ_PATH/$LZO/src/.libs/
export LDFLAGS=-static
make

