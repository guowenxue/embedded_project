#!/bin/sh

ARCH=$1
INST_PATH=./mnt/lib

sup_arch=("" "arm920t" "arm926ejs")
function select_arch()
{
   echo "Current support cross compile arch:"
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

if [ -z $ARCH ] ; then
select_arch
fi

set -x
CROSS_PATH=/opt/buildroot-2011.02/$ARCH/usr

cp -af $CROSS_PATH/arm-linux/lib/*so* $INST_PATH
cp -af $CROSS_PATH/arm-linux/sysroot/lib/*so* $INST_PATH
cp -af $CROSS_PATH/arm-linux/sysroot/usr/lib/libz*.so* $INST_PATH
cp -af $CROSS_PATH/arm-linux/sysroot/usr/lib/liblzo*.so* $INST_PATH

