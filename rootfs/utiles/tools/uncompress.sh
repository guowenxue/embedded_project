#!/bin/sh
#      Author:   GuoWenxue<guowenxue@gmail.com>
#        Date:   2011.04.24
# Description:   This shell script used to auto uncompress the *.gz, *.tar, 
#                *.zip *.tar.gz *.tar.bz2 format compressed file

if [ $# != 1 ] ; then
    echo "This shell script used to uncompress linux common compressed packet."
    echo "current support: *.gz *.bz2 *.tar *.zip *.tar.bz2 *.tar.gz "
    echo "$0 [filename]"
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

uncompress $1
