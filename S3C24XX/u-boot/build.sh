#!/bin/sh
# Descripion:  This shell script used to choose a u-boot version to cross compile
#     Author:  GuoWenxue<guowenxue@gmail.com>
#  ChangeLog:
#       1, Version 1.0.0(2011.04.01), initialize first version 
#       2, Version 1.0.1(2011.04.03), modify it to compatible with Linux kernel build script
#       3, Version 1.1.0(2011.04.10), modify to support s3c2410 cross compile support
#

#User can pass a argument to specify which version should be cross compile
#or uncomment the SRC_NAME variable to specify the version 

#SRC_NAME=u-boot-2010.09
#ARCH=fl2440
ARCH=$1
SRC_NAME=$2

PWD=`pwd`
PACKET_DIR=$PWD/
PATCH_DIR=$PWD/patch

INST_PATH=$PWD/../bin

#===============================================================
#               Functions forward definition                   =
#===============================================================
sup_ver=("" "u-boot-2010.09")
function select_version()
{
   echo "Current support U-Boot version:"
   i=1
   len=${#sup_ver[*]}

   while [ $i -lt $len ]; do
       echo "$i: ${sup_ver[$i]}"
       let i++;
   done

   if [ $len -eq 2 ] ; then
       SRC_NAME=${sup_ver[1]}
       return;
   fi

   echo "Please select: "
   index=
   read index 

   SRC_NAME=${sup_ver[$index]}
}

sup_arch=("" "fl2440" "sd2410")
function select_arch()
{
   echo "Current support S3C24X0 boards:"
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


function disp_banner()
{
   echo ""
   echo "*******************************************************"
   echo "*     Cross compile $SRC_NAME for $ARCH now...       "
   echo "*******************************************************"
   echo ""
}

#===============================================================
#                   Script excute body start                   =
#===============================================================

# If not define default version, then let user choose a one
if [ -z $SRC_NAME ] ; then
    select_version
fi

# If don't set the ARCH, then select one
if [ -z $ARCH ] ; then
   select_arch
fi

disp_banner

# If $SRC_NAME not set, then abort this cross compile
if [ -z $SRC_NAME ] ; then 
    echo "ERROR: Please choose a valid version to cross compile"
    exit 1;
fi

# Check original source code packet exist or not
SRC_ORIG_PACKET=$PACKET_DIR/$SRC_NAME.tar.bz2
if [ ! -s $SRC_ORIG_PACKET ] ; then
    echo ""
    echo "ERROR:$SRC_NAME source code patcket doesn't exist:"
    echo "PATH: \"$SRC_ORIG_PACKET\""
    echo ""
    exit
fi

# Check patche file exist or not
PATCH_FILE=$PATCH_DIR/$SRC_NAME-s3c24xx.patch
if [ ! -f $PATCH_FILE ] ; then
    echo "ERROR:$SRC_NAME patch file doesn't exist:"
    echo "PATH: \"$PATCH_FILE\""
    echo ""
    exit
fi

#decompress the source code packet and patch
echo "*  Decompress the source code patcket and patch now...  *"

if [ -d $SRC_NAME ] ; then
    rm -rf $SRC_NAME
fi


#Remove old source code
tar -xjf $SRC_ORIG_PACKET
patch -p0 < $PATCH_FILE

#Start to cross compile the source code and install it now

cd $SRC_NAME
sh build.sh $ARCH
set -x
cp -af u-boot.bin $INST_PATH/$SRC_NAME-$ARCH.bin

