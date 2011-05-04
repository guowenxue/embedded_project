#!/bin/sh

#+--------------------------------------------------------------------------------------------
#|Description:  This shell script used to cross compile all the apps in current folder.
#|     Author:  GuoWenxue <guowenxue@gmail.com>
#|  ChangeLog:
#|           1, Initialize 1.0.0 on 2011.04.21
#+--------------------------------------------------------------------------------------------

PRJ_PATH=`pwd`


ARCH=$1
sup_arch=("" "s3c2410" "s3c2440" "at91sam9260")

function select_arch()
{
   echo "Current support ARCH: "
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


for i in `ls ${APPS_PATH}`; do
        if [ -f $i/build.sh ] ; then
                cd $i
                sh build.sh $ARCH
                cd -
        fi
done
