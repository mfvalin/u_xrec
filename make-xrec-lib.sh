#!/bin/sh -x
for dir in autog gd_wgl gmp main select selfic util wgl x_wgl xdash xinit xplot88
do
    cd $dir
    make
    cd ..
done

