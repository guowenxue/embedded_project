#!/bin/sh

PRJ_PATH=`pwd`
rootfs=mnt
SYSLIB_PATH=$rootfs/lib
USERLIB_PATH=$rootfs/usr/lib
USERBIN_PATH=$rootfs/usr/bin

#Check the priviledge, must excute as "root"
user=`whoami`
if [ $user != root ]; then 
        echo "This script must excute as root priviledge, please use \"sudo\" to excute." 
        exit; 
fi

#Remove the old one.
rm -f ${USERLIB_PATH}/*
rm -f ${SYSLIB_PATH}/*
rm -f ${USERBIN_PATH}/*


#Install Crosstool library or tools
cp -af /opt/buildroot-eabi/staging/usr/arm-unknown-linux-uclibcgnueabi/lib/lib*so* ${SYSLIB_PATH}
cp -af /opt/buildroot-eabi/staging/lib/*so* ${SYSLIB_PATH}
cp /opt/buildroot-eabi/staging/usr/lib/libc.so ${SYSLIB_PATH}
cp /opt/buildroot-eabi/output/target/etc/ld.so.cache $rootfs/etc/ 
cp /opt/buildroot-eabi/output/target/usr/bin/ldd ${USERBIN_PATH}

#Install some application tools
cp apps/dropbear/dropbear-0.52/dropbear ${USERBIN_PATH}
cp apps/dropbear/dropbear-0.52/ssh ${USERBIN_PATH}/ssh
cp apps/file/file-5.04/src/file ${USERBIN_PATH}
cp apps/tree/tree-1.5.2/tree ${USERBIN_PATH}
cp apps/wireless/wireless_tools.29/iwconfig ${USERBIN_PATH}
cp apps/wireless/wireless_tools.29/iwlist ${USERBIN_PATH}
cp apps/wireless/wireless_tools.29/iwpriv ${USERBIN_PATH}

#Generate the rootfs image
cd ${PRJ_PATH}
umount mnt && gzip ramdisk 

