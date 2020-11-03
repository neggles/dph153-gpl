#!/bin/bash

# save current PATH
#OLDPATH=$PATH
#export PATH=$PATH:$(pwd)/arm-2014.05/bin

export ARCH=arm
export CROSS_COMPILE=$(pwd)/arm-2014.05/bin/arm-none-linux-gnueabi-
export TYPE=xc
export IPV=420.69 # ha ha, yes, very funny, self
