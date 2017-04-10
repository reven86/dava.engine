#!/bin/sh

echo "*****************   COPY DATA STARTED   ****************"

echo "\$0=$0"
cd `dirname $0`

echo " "

echo "PWD=`pwd`"


echo "Remove Data"
rm -f -r -v `pwd`/assets/Data
echo "Remove Done"

echo "Copy Data"
ditto -v `pwd`/../Data `pwd`/assets/Data
echo "Copy Data Done"

echo "Remove .svn"
find ./assets/Data -name "*.svn*" -exec rm -rf {} \;
find ./assets -name ".DS_Store" -exec rm -rf {} \;
echo "Remove .svn Done"

echo "*****************   COPY DATA FINISHED   ****************"
