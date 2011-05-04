#!/bin/sh

make
cp arch/arm/boot/zImage . -f
mkimage -A arm -O linux -T kernel -C none -a 20008000 -e 20008000 -n "Linux Kernel" -d zImage uImage.gz
rm -f zImage
set -x
cp uImage.gz /tftpboot/uImage-2.6.24-gr01.gz --reply=yes
