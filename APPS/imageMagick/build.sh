#!/bin/sh
# This shell script used to cross compile ImageMagick-6.6.5-4 for fl2440
# Before excute this shell script, please download the source code to here first.

APP_NAME=ImageMagick-6.6.5-4
PATCH_FILE=./patch/build.sh
PACK_SUFFIX=tar.bz2


if [ ! -d ${APP_NAME} ] ; then
   if [ -f ${APP_NAME}.${PACK_SUFFIX} -a -f $PATCH_FILE ] ; then
        echo "==================================================="
        echo "+           Decompress $APP_NAME now...              +"
        echo "==================================================="
        rm -rf ${APP_NAME}
        tar -xjf ${APP_NAME}.${PACK_SUFFIX}
        
        echo "===================================================" 
        echo "+                 Patching now...                 +" 
        echo "===================================================" 
        cp ${PATCH_FILE} ${APP_NAME}
  else 
          echo "================================================================" 
          echo "+       ERROR: Missing $APP_NAME Source Code Packet!              +" 
          echo "+ Plese download ${APP_NAME}.${PACK_SUFFIX} to here and try again! +" 
          echo "================================================================" 
          exit; 
  fi 
fi

cd $APP_NAME && sh build.sh

