#!/bin/bash

writefile=$1
writestr=$2

if [ $# -eq 2 ];then
    filedir=${writefile%/*}
    if [ -d $filedir ]; then
        echo $writestr > $writefile || { echo "File could not be created";exit 1; }
        exit 0
    else
        mkdir -p "$filedir" && echo $writestr > $writefile 
        exit 0
    fi
else
    exit 1
fi
