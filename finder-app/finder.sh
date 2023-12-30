#!/usr/bin/bash

filesdir=$1
searchstr=$2

if [ $# -eq 2 ] ;then
    if [ -d $filesdir ]; then
        x="$(find . -type f | wc -l)"
        y=$(grep -r $searchstr $filesdir | wc -l )
        echo "The number of files are $x and the number of matching lines are $y"
        exit 0
    else
        exit 1
    fi
else
    exit 1
fi
