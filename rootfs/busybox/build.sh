#!/bin/sh

#+--------------------------------------------------------------------------------------------
#|Description:  This shell script used to download busybox source code and cross compile it.
#|     Author:  GuoWenxue <guowenxue@gmail.com>
#|  ChangeLog:
#|           1, Initialize 1.0.0 on 2011.04.21
#+--------------------------------------------------------------------------------------------

PRJ_PATH=`pwd`

APP_NAME="busybox-1.18.4"
PACK_SUFIX="tar.bz2"
DL_ADDR="http://www.busybox.net/downloads/busybox-1.18.4.tar.bz2"
INST_PATH=

ARCH=$1

ARM920T_CROSS="/opt/buildroot-2011.02/arm920t/usr/bin/arm-linux-"
ARM926T_CROSS="/opt/buildroot-2011.02/arm926ejs/usr/bin/arm-linux-"

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

if [ $ARCH = "s3c2440" -o $ARCH = "s3c2410" ] ; then 
    CROSS="$ARM920T_CROSS"
elif [ $ARCH = "at91sam9260" ] ; then
    CROSS="$ARM926T_CROSS"
else
    echo "+------------------------------------------------------------------+"
    echo "|  ERROR: Unsupport platform to cross compile "  
    echo "+------------------------------------------------------------------+"
    exit -1;
fi


# Download source code packet
if [ ! -s $APP_NAME.$PACK_SUFIX ] ; then
   echo "+------------------------------------------------------------------+"
   echo "|  Download $APP_NAME.$PACK_SUFIX  now "  
   echo "+------------------------------------------------------------------+"

   wget $DL_ADDR
fi

# Decompress source code packet
if [ ! -d $APP_NAME ] ; then
    decompress_packet $APP_NAME.tar.*
fi

#Copy the configure file
if [ ! -s .config ]; then
    cp patch/$APP_NAME.config $APP_NAME/.config
fi

if [ ! -s $APP_NAME/.config ]; then
   echo "+------------------------------------------------------------------+"
   echo "| ERROR: Miss default configure file"  
   echo "+------------------------------------------------------------------+"
   exit -2
fi

if [ -z $INST_PATH ] ; then
   INST_PATH=$PRJ_PATH/../$ARCH/mnt
fi

echo "+------------------------------------------------------------------+"
echo "|          Build $APP_NAME for $ARCH "
echo "| Crosstool:  $CROSS"
echo "+------------------------------------------------------------------+"

cd $APP_NAME

   #Modify the cross config in the configure file
   line=`sed -n '/CONFIG_CROSS_COMPILER_PREFIX/=' .config`
   sed -i -e ${line}s"|.*|CONFIG_CROSS_COMPILER_PREFIX=\"$CROSS\"|" .config

   #Modify the install path in the configure file
   line=`sed -n '/CONFIG_PREFIX=/=' .config`
   sed -i -e ${line}s"|.*|CONFIG_PREFIX=\"$INST_PATH\"|" .config

   set -x
   make clean
   make
   set +x

   #Check Root filesystem mount or not
   mountpoint $INST_PATH
   if [ 0 != $? ] ; then 
      echo "+------------------------------------------------------------------+"
      echo "| WARNING: Root Filesystem $INST_PATH not mounted, don't install."  
      echo "+------------------------------------------------------------------+"
      exit
   fi

   #install busybox
   sudo rm -rf $INST_PATH/bin/*
   sudo rm -rf $INST_PATH/sbin/*
   make install 
cd  -

