#!/bin/bash
LOG_FILE="buildall.log"

export ARCH=arm
export CROSS_COMPILE=arm-none-eabi-
export TYPE=xc
export IPV=420.69 # ha ha, yes, very funny, self

>${LOG_FILE}
mkdir build && pushd build

echo -e "\nsrc package bash-3.0.16.tar.gz..." | tee -a ../${LOG_FILE}
echo "________________________________________________________________________________" | tee -a ../${LOG_FILE}
pushd bash-3.0.16
make | tee -a ../../${LOG_FILE}
popd

echo -e "\nsrc package busybox-1.9.2.tar.gz..." | tee -a ../${LOG_FILE}
echo "________________________________________________________________________________" | tee -a ../${LOG_FILE}
pushd busybox-1.9.2
make | tee -a ../../${LOG_FILE}
popd

echo -e "\nsrc package cramfs-1.1.tar.gz..." | tee -a ../${LOG_FILE}
echo "________________________________________________________________________________" | tee -a ../${LOG_FILE}
pushd cramfs-1.1/
export OBJDIR=$PWD
make -f GNUmakefile | tee -a ../../${LOG_FILE}
popd

echo -e "\nsrc package u-boot-1.3.4.tar.bz2..." | tee -a ../${LOG_FILE}
echo "________________________________________________________________________________" | tee -a ../${LOG_FILE}
pushd u-boot-1.3.4
make | tee -a ../../${LOG_FILE}
popd

echo -e "\nsrc package strongswan-4.2.12.tar.gz..." | tee -a ../${LOG_FILE}
echo "________________________________________________________________________________" | tee -a ../${LOG_FILE}
pushd strongswan-4.2.12
make | tee -a ../../${LOG_FILE}
popd

echo -e "\nsrc package ethtool-6.tar.gz..." | tee -a ../${LOG_FILE}
echo "________________________________________________________________________________" | tee -a ../${LOG_FILE}
pushd ethtool-6
make | tee -a ../../${LOG_FILE}
popd

echo -e "\nsrc package gcc-2008q3-72.tar.bz2..." | tee -a ../${LOG_FILE}
echo "________________________________________________________________________________" | tee -a ../${LOG_FILE}
pushd gcc-4.3
pushd build
make | tee -a ../../../${LOG_FILE}
popd
popd

echo -e "\nsrc package glibc-2008q3-72.zip..." | tee -a ../${LOG_FILE}
echo "________________________________________________________________________________" | tee -a ../${LOG_FILE}
pushd glibc-2008q3-72/
pushd glibc-2.8/
pushd build
make | tee -a ../../../../${LOG_FILE}
popd
popd
popd

echo -e "\nsrc package iproute2-2.6.26.tar.bz2..." | tee -a ../${LOG_FILE}
echo "________________________________________________________________________________" | tee -a ../${LOG_FILE}
pushd iproute2-2.6.26
make | tee -a ../../${LOG_FILE}
popd

echo -e "\nsrc package gmp-4.2.1.tar.gz..." | tee -a ../${LOG_FILE}
echo "________________________________________________________________________________" | tee -a ../${LOG_FILE}
pushd gmp-4.2.1
make | tee -a ../../${LOG_FILE}
popd

echo -e "\nsrc package linux-2.6.28.tar.gz..." | tee -a ../${LOG_FILE}
echo "________________________________________________________________________________" | tee -a ../${LOG_FILE}
pushd linux-2.6.28/
make | tee -a ../../${LOG_FILE}
popd
echo -e "\nsrc package iptables-1.4.2.tar.bz2..." | tee -a ../${LOG_FILE}
echo "________________________________________________________________________________" | tee -a ../${LOG_FILE}
pushd iptables-1.4.2
make | tee -a ../../${LOG_FILE}
popd

echo -e "\nsrc package mtd-utils.tgz..." | tee -a ../${LOG_FILE}
echo "________________________________________________________________________________" | tee -a ../${LOG_FILE}
pushd mtd-utils-1.0.0
make | tee -a ../../${LOG_FILE}
popd

echo -e "\nsrc package procps-3.2.7.tar.gz..." | tee -a ../${LOG_FILE}
echo "________________________________________________________________________________" | tee -a ../${LOG_FILE}
pushd procps-3.2.7
make | tee -a ../../${LOG_FILE}
popd

echo -e "\nsrc package smartmontools-5.37.tar.gz..." | tee -a ../${LOG_FILE}
echo "________________________________________________________________________________" | tee -a ../${LOG_FILE}
pushd smartmontools-5.37
make | tee -a ../../${LOG_FILE}
popd

echo -e "\nsrc package i2c-tools-3.0.2.tar.bz2..." | tee -a ../${LOG_FILE}
echo "________________________________________________________________________________" | tee -a ../${LOG_FILE}
pushd i2c-tools-3.0.2
make | tee -a ../../${LOG_FILE}
popd

echo -e "\nsrc package ipkg-0.99.163.tar.gz..." | tee -a ../${LOG_FILE}
echo "________________________________________________________________________________" | tee -a ../${LOG_FILE}
pushd ipkg-0.99.163
make | tee -a ../../${LOG_FILE}
popd

