#!/bin/sh

MNT_POINT=mnt

if [ 1 != $# ] ; then
   echo "$0 [rootfs]"
   exit
fi

function uncompress()
{
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
}

#Rootfs packet name
RF_PACK=$1

if [ ! -s $RF_PACK ] ; then
   echo "ERROR: Rootfs \"$RF_PACK\" doesn't exist"
   exit
fi 

#Rootfs uncompressed name
RF_NAME=`echo $RF_PACK | awk -F '.' '{print   $RF_PACK}'`
uncompress $RF_PACK

if [ ! -d $MNT_POINT ] ; then
   mkdir $MNT_POINT
fi

mountpoint $MNT_POINT
if [ 0 == $? ] ; then
   echo "$MNT_POINT already mounted" 
   exit;
fi

mount -o loop $RF_NAME $MNT_POINT

