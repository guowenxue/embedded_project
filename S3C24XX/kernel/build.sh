#!/bin/sh
# Descripion:  This shell script used to choose a linux kernel version to cross compile
#     Author:  GuoWenxue<guowenxue@gmail.com>
#  ChangeLog:
#       1, Version 1.0.0(2011.04.01), initialize first version 
#

PWD=`pwd`
PACKET_DIR=$PWD/
PATCH_DIR=$PWD/patch
INST_PATH=$PWD/../bin

ARCH=s3c2440
SRC_NAME=

#===============================================================
#               Functions forward definition                   =
#===============================================================
function disp_banner()
{
   echo ""
   echo "+------------------------------------------+"
   echo "|      Build $SRC_NAME for $ARCH            "
   echo "+------------------------------------------+"
   echo ""
}

sup_ver=("" "linux-3.0" "linux-2.6.33" "linux-2.6.38")
function select_version()
{
   echo "Current support linux kernel version:"
   i=1
   len=${#sup_ver[*]}


   while [ $i -lt $len ]; do
       echo "$i: ${sup_ver[$i]}"
       let i++;
   done

   echo "Please select: "
   index=
   read index 

   SRC_NAME=${sup_ver[$index]}
}


#===============================================================
#                   Script excute body start                   =
#===============================================================

# If not define default version, then let user choose a one
if [ -z $SRC_NAME ] ; then
    select_version
fi

# If $SRC_NAME not set, then abort this cross compile
if [ -z $SRC_NAME ] ; then 
    echo "ERROR: Please choose a valid version to cross compile"
    exit 1;
fi

disp_banner    #Display this shell script banner

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
PATCH_FILE=$PATCH_DIR/$SRC_NAME-${ARCH}.patch
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
cp .cfg-$ARCH .config
make
VERSION=`echo $SRC_NAME | awk -F "-" '{print $2}'`
set -x
cp -af uImage.gz $INST_PATH/uImage-$VERSION-$ARCH.gz


