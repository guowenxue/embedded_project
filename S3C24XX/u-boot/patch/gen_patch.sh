#!/bin/sh
# Description:  This shell script used to generate the patch file
#      Author:  GuoWenxue<guowenxue@gmail.com>
#    Changlog:
#         1,    Version 1.0.0(2011.04.01), initialize first version 
#               

PWD=`pwd`
PACKET_DIR=$PWD

# Parameter valid check
if [ $# != 2 ] ; then
    echo "+---------------------------------------------------"
    echo "|   Usage:  $0 [SRC_FOLDER] [ARCH]"
    echo "| Example:  $0 u-boot-2010.09 fl2440"
    echo "| Example:  $0 u-boot-1.3.4 at91sam9260"
    echo "+---------------------------------------------------"
    exit;
fi

SRC_NAME=`basename $1`
ARCH=$2

# Check latest source code exist or not
if [ ! -d $SRC_NAME ] ; then
    echo "+-------------------------------------------------------------------"
    echo "|  ERROR: Source code \"$SRC_NAME\" doesn't exist!"
    echo "+-------------------------------------------------------------------"
    exit;
fi

SRC_PACKET_PATH=$PACKET_DIR/$SRC_NAME.tar.bz2
# Check original source code packet exist or not
if [ ! -s $SRC_PACKET_PATH ] ; then
    echo "+-------------------------------------------------------------------"
    echo "| ERROR:  Orignal source code packet doesn't exist!"
    echo "| $SRC_PACKET_PATH"
    echo "+-------------------------------------------------------------------"
    exit;
fi

# Clean up the source code
echo "+----------------------------------------------------------"
echo "|            Clean up the new source code                  "
echo "+----------------------------------------------------------"
NEW_SRC=$SRC_NAME-$ARCH
cd $SRC_NAME
rm -f tags cscope*
make distclean
cd ..
mv $SRC_NAME $NEW_SRC

echo "+----------------------------------------------------------"
echo "|         Decompress original source code packet           "
echo "+----------------------------------------------------------"
ORIG_SRC=$SRC_NAME
tar -xjf $SRC_PACKET_PATH

# Generate the patch file
echo "+------------------------------------------------------------------------"
echo "| Generate patch file \"$NEW_SRC.patch\"                                 "
echo "+------------------------------------------------------------------------"
diff -Nuar $ORIG_SRC $NEW_SRC > $NEW_SRC.patch

# Rollback to the original status
rm -rf $ORIG_SRC
mv $NEW_SRC $SRC_NAME



