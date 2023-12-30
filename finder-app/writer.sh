#!/bin/bash

writefile=$1
writestr=$2

if [ $# -eq 2 ];then
    echo $writestr > $writefile || { echo "File could not be created"; exit 1; }
    exit 0
else
    exit 1
fi
