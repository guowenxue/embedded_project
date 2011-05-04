#!/bin/sh

#+--------------------------------------------------------------------------------------------
#|Description:  This shell script used to download mplayer source code and cross compile it.
#|     Author:  GuoWenxue <guowenxue@gmail.com>
#|  ChangeLog:
#|           1, Initialize 1.0.0 on 2011.04.26
#+--------------------------------------------------------------------------------------------

PRJ_PATH=`pwd`

APP_NAME="MPlayer-1.0rc2"
PACK_NAME="MPlayer-1.0rc2.tar.bz2"
DL_ADDR="http://www.mplayerhq.hu/MPlayer/releases/MPlayer-1.0rc2.tar.bz2"

PATCH_FILE=../patch/MPlayer-1.0rc2-s3c24x0.patch

ARCH=$1

ARM920T_CROSS="/opt/buildroot-2011.02/arm920t/usr/"
CROSS=

sup_arch=("" "s3c2410" "s3c2440" )

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
           if [ 0 != `expr "$1" : ".*.tar.*" ` ] ; then
               tar -xzf $1
           else
               gzip -d "$1"
           fi ;;
       "$1: bzip2 compressed"*)
           if [ 0 != `expr "$1" : ".*.tar.*" ` ] ; then
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
else
    echo "+------------------------------------------------------------------+"
    echo "|  ERROR: Unsupport platform to cross compile "  
    echo "+------------------------------------------------------------------+"
    exit -1;
fi

# Download source code packet
if [ ! -s $PACK_NAME ] ; then
   echo "+------------------------------------------------------------------+"
   echo "|  Download $PACK_NAME now "  
   echo "+------------------------------------------------------------------+"

   wget $DL_ADDR
fi

# Decompress source code packet
if [ ! -d $APP_NAME ] ; then
    decompress_packet $PACK_NAME
fi

if [ -z $INST_PATH ] ; then
   INST_PATH=$PRJ_PATH/../$ARCH/mnt
fi

echo "+------------------------------------------------------------------+"
echo "|          Build $APP_NAME for $ARCH "
echo "| Crosstool:  $CROSS"
echo "+------------------------------------------------------------------+"

cd $APP_NAME
   if [ ! -s build.sh ] ; then
        patch -p1 < $PATCH_FILE
   fi 
   sh build.sh $CROSS
cd -

