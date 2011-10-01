#!/bin/sh
PWD=`pwd`
PRJ_NAME=`basename $PWD`
#INST_PATH=/tftpboot
INST_PATH=~/winxp/images/
#ARCH=fl2440
ARCH=$1


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
   echo "+------------------------------------------+"
   echo "|      Build $PRJ_NAME for $ARCH            "
   echo "+------------------------------------------+"
   echo ""
}

if [ -z $ARCH ] ; then
   select_arch
fi


disp_banner

#If don't do "make $ARCH_config", it will configure for the board first
if [ ! -s include/config.mk ] ; then
   make ${ARCH}_config
fi

#Modify the u-boot.lds for sd2410/fl2440
UBOOT_LDS=arch/arm/cpu/arm920t/u-boot.lds
sed -i -e "s/sd2410/$ARCH/g" $UBOOT_LDS
sed -i -e "s/fl2440/$ARCH/g" $UBOOT_LDS


#Cross compile it now
make

IMAGE_NAME=u-boot-${ARCH}.bin

set -x
sudo rm -rf  ${INST_PATH}/${IMAGE_NAME}
sudo cp u-boot.bin ${INST_PATH}/${IMAGE_NAME}
sudo cp u-boot.bin /tftpboot/${IMAGE_NAME}

