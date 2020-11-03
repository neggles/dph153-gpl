#!/bin/bash
LOG_FILE="extract.log"

>${LOG_FILE}
mkdir -p build && pushd build

echo -e "\nextract package bash-3.0.16.tar.gz..." | tee -a ../${LOG_FILE}
tar zxvf ../src/bash-3.0.16.tar.gz

echo -e "\nextract package busybox-1.9.2.tar.gz..." | tee -a ../${LOG_FILE}
tar zxvf ../src/busybox-1.9.2.tar.gz

echo -e "\nextract package cramfs-1.1.tar.gz..." | tee -a ../${LOG_FILE}
tar zxvf ../src/cramfs-1.1.tar.gz

echo -e "\nextract package u-boot-1.3.4.tar.bz2..." | tee -a ../${LOG_FILE}
tar jxvf ../src/u-boot-1.3.4.tar.bz2

echo -e "\nextract package strongswan-4.2.12.tar.gz..." | tee -a ../${LOG_FILE}
tar zxvf ../src/strongswan-4.2.12.tar.gz

echo -e "\nextract package ethtool-6.tar.gz..." | tee -a ../${LOG_FILE}
tar zxvf ../src/ethtool-6.tar.gz

echo -e "\nextract package gcc-2008q3-72.tar.bz2..." | tee -a ../${LOG_FILE}
tar jxvf ../src/gcc-2008q3-72.tar.bz2

echo -e "\nextract package glibc-2008q3-72.zip..." | tee -a ../${LOG_FILE}
unzip ../src/glibc-2008q3-72.zip

echo -e "\nextract package iproute2-2.6.26.tar.bz2..." | tee -a ../${LOG_FILE}
tar jxvf ../src/iproute2-2.6.26.tar.bz2

echo -e "\nextract package gmp-4.2.1.tar.gz..." | tee -a ../${LOG_FILE}
tar zxvf ../src/gmp-4.2.1.tar.gz

echo -e "\nextract package linux-2.6.28.tar.gz..." | tee -a ../${LOG_FILE}
tar zxvf ../src/linux-2.6.28.tar.gz

echo -e "\nextract package iptables-1.4.2.tar.bz2..." | tee -a ../${LOG_FILE}
tar jxvf ../src/iptables-1.4.2.tar.bz2

echo -e "\nextract package mtd-utils.tgz..." | tee -a ../${LOG_FILE}
tar zxvf ../src/mtd-utils.tgz

echo -e "\nextract package procps-3.2.7.tar.gz..." | tee -a ../${LOG_FILE}
tar zxvf ../src/procps-3.2.7.tar.gz

echo -e "\nextract package smartmontools-5.37.tar.gz..." | tee -a ../${LOG_FILE}
tar zxvf ../src/smartmontools-5.37.tar.gz

echo -e "\nextract package i2c-tools-3.0.2.tar.bz2..." | tee -a ../${LOG_FILE}
tar jxvf ../src/i2c-tools-3.0.2.tar.bz2

echo -e "\nextract package ipkg-0.99.163.tar.gz..." | tee -a ../${LOG_FILE}
tar zxvf ../src/ipkg-0.99.163.tar.gz

popd
