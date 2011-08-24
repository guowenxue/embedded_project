#!/bin/sh
# Descripion:  This shell script used to choose a u-boot version to cross compile
#     Author:  GuoWenxue<guowenxue@gmail.com>
#  ChangeLog:
#       1, Version 1.0.0(2011.04.01), initialize first version 
#       2, Version 1.0.1(2011.04.03), modify it to compatible with Linux kernel build script
#

#User can pass a argument to specify which version should be cross compile
#or uncomment the DEF_VERSION variable to specify the version 
DEF_VERSION=$1
#DEF_VERSION=u-boot-1.3.4

PWD=`pwd`
PACKET_DIR=$PWD/../packet
PATCH_DIR=$PWD/patch

PLATFORM=at91sam9xxx
#CPU=sam9g20
CPU=sam9260
PATCH_SUFFIX=-${PLATFORM}.patch
PRJ_NAME="u-boot"
BIN_NAME="u-boot.bin"
INST_PATH=$PWD/../../bin/bootloader
SRC_NAME=


sup_ver=("" "u-boot-2010.09")

#===============================================================
#               Functions forward definition                   =
#===============================================================
function disp_banner()
{
   echo ""
   echo "+------------------------------------------+"
   echo "|      Build $PRJ_NAME for $CPU             "
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

   if [ $len -eq 2 ] ; then
       SRC_NAME=${sup_ver[1]}
       return;
   fi

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
disp_compile

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
cd $SRC_NAME
make ${CPU}
cp -af $BIN_NAME $INST_PATH/


