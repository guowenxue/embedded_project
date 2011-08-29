#!/bin/sh
# Descripion:  This shell script used to choose a linux kernel version to cross compile
#     Author:  GuoWenxue<guowenxue@gmail.com>
#  ChangeLog:
#       1, Version 1.0.0(2011.04.01), initialize first version 
#

#User can pass a argument to specify which version should be cross compile
#or uncomment the DEF_VERSION variable to specify the version 
DEF_VERSION=$1
#DEF_VERSION=linux-2.6.24

PWD=`pwd`
PACKET_DIR=$PWD/../packet
PATCH_DIR=$PWD/patch

ARCH=fl2440
PATCH_SUFFIX=-${ARCH}.patch
PRJ_NAME="linux kernel"
INST_PATH=$PWD/bin
SRC_NAME=


sup_ver=("" "linux-2.6.24" "linux-2.6.33" "linux-2.6.38")

#===============================================================
#               Functions forward definition                   =
#===============================================================
function disp_banner()
{
   echo ""
   echo "+------------------------------------------+"
   echo "|      Build $PRJ_NAME for $ARCH            "
   echo "+------------------------------------------+"
   echo ""
}

function select_version()
{
   echo "Current support $PRJ_NAME version:"
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

function disp_compile()
{
   echo ""
   echo "********************************************"
   echo "*     Cross compile $SRC_NAME now...       "
   echo "********************************************"
   echo ""
}

#===============================================================
#                   Script excute body start                   =
#===============================================================

disp_banner    #Display this shell script banner

# If not define default version, then let user choose a one
if [ -z $DEF_VERSION ] ; then
    select_version
else
    SRC_NAME=$DEF_VERSION
fi

# If $SRC_NAME not set, then abort this cross compile
if [ -z $SRC_NAME ] ; then 
    echo "ERROR: Please choose a valid version to cross compile"
    exit 1;
fi

# Check original source code packet exist or not
SRC_ORIG_PACKET=$PACKET_DIR/$SRC_NAME.tar.bz2
if [ ! -s $SRC_ORIG_PACKET ] ; then
    echo ""
    echo "ERROR:$PRJ_NAME source code patcket doesn't exist:"
    echo "PATH: \"$SRC_ORIG_PACKET\""
    echo ""
    exit
fi

# Check patche file exist or not
PATCH_FILE=$PATCH_DIR/$SRC_NAME$PATCH_SUFFIX
if [ ! -f $PATCH_FILE ] ; then
    echo "ERROR:$PRJ_NAME patch file doesn't exist:"
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
disp_compile
cd $SRC_NAME
cp .cfg-$ARCH .config
make
VERSION=`echo $SRC_NAME | awk -F "-" '{print $2}'`
cp -af uImage.gz $INST_PATH/uImage-$VERSION-$ARCH.gz


