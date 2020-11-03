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
./configure | tee -a ../../${LOG_FILE}
#make | tee -a ../../${LOG_FILE}
popd

echo -e "\nsrc package busybox-1.9.2.tar.gz..." | tee -a ../${LOG_FILE}
echo "________________________________________________________________________________" | tee -a ../${LOG_FILE}
pushd busybox-1.9.2
tar zxvf ../../src/579.11.114/busybox-1.9.2-local-patch.tgz
cp -rf ipa-diff/* .
#make oldconfig | tee -a ../../${LOG_FILE}
#make | tee -a ../../${LOG_FILE}
popd

echo -e "\nsrc package cramfs-1.1.tar.gz..." | tee -a ../${LOG_FILE}
echo "________________________________________________________________________________" | tee -a ../${LOG_FILE}
pushd cramfs-1.1/
tar zxvf ../../src/579.11.114/cramfs-1.1-local-patch.tar.gz
cp -rf ipa-diff/* .
#export OBJDIR=$PWD
#make -f GNUmakefile | tee -a ../../${LOG_FILE}
popd

echo -e "\nsrc package u-boot-1.3.4.tar.bz2..." | tee -a ../${LOG_FILE}
echo "________________________________________________________________________________" | tee -a ../${LOG_FILE}
pushd u-boot-1.3.4
cp ../../src/579.11.114/u-boot-1.3.4-local-patch-224.gz .
cp ../../src/579.11.114/u-boot-1.3.4-picochip-3.2.4-patch.gz .
gunzip *.gz
patch -p1 <u-boot-1.3.4-picochip-3.2.4-patch
patch -p1 <u-boot-1.3.4-local-patch-224
make ipaccessip302ff_config | tee -a ../../${LOG_FILE}
cat >>common/main.c<<EOF
unsigned long long get_ticks(void)
{
       return get_timer(0);
}
ulong get_tbclk (void)
{
       ulong tbclk;

       tbclk = CFG_HZ;
       return tbclk;
}
int raise() { return 0; }
EOF
#make | tee -a ../../${LOG_FILE}
popd

echo -e "\nsrc package strongswan-4.2.12.tar.gz..." | tee -a ../${LOG_FILE}
echo "________________________________________________________________________________" | tee -a ../${LOG_FILE}
pushd strongswan-4.2.12
tar zxvf ../../src/579.11.114/strongswan-4.2.12-official-patch.tar.gz
patch -p1 <patches_strongswan/04_swapped_ts_check_patch/strongswan-4.x.x._swapped_ts_check.patch
patch -p1 <patches_strongswan/03_invalid_ike_state_patch/strongswan-4.x.x_invalid_ike_state.patch
./configure | tee -a ../../${LOG_FILE}
#make | tee -a ../../${LOG_FILE}
popd

echo -e "\nsrc package ethtool-6.tar.gz..." | tee -a ../${LOG_FILE}
echo "________________________________________________________________________________" | tee -a ../${LOG_FILE}
pushd ethtool-6
./configure | tee -a ../../${LOG_FILE}
#make | tee -a ../../${LOG_FILE}
popd

echo -e "\nsrc package gcc-2008q3-72.tar.bz2..." | tee -a ../${LOG_FILE}
echo "________________________________________________________________________________" | tee -a ../${LOG_FILE}
pushd gcc-4.3
mkdir build
pushd build
../configure --with-mpfr=/usr/local --with-gmp=/usr/local --disable-multilib | tee -a ../../../${LOG_FILE}
#make | tee -a ../../../${LOG_FILE}
popd
popd

echo -e "\nsrc package glibc-2008q3-72.zip..." | tee -a ../${LOG_FILE}
echo "________________________________________________________________________________" | tee -a ../${LOG_FILE}
unzip ../src/glibc-2008q3-72.zip
pushd glibc-2008q3-72/
tar jxvf glibc-2008q3-72.tar.bz2
pushd glibc-2.8/
cp ../../../src/579.11.114/glibc-2008q3-72-ipa.patch.gz .
gunzip glibc-2008q3-72-ipa.patch.gz
patch -p1 <glibc-2008q3-72-ipa.patch
mkdir build && pushd build
../configure --disable-sanity-checks | tee -a ../../../../${LOG_FILE}
#make | tee -a ../../../../${LOG_FILE}
popd
popd
popd

echo -e "\nsrc package iproute2-2.6.26.tar.bz2..." | tee -a ../${LOG_FILE}
echo "________________________________________________________________________________" | tee -a ../${LOG_FILE}
pushd iproute2-2.6.26
tar zxvf ../../src/579.11.114/iproute2-2.6.26-local-patch.tar.gz
cp -rf patches/* .
./configure | tee -a ../../${LOG_FILE}
#make | tee -a ../../${LOG_FILE}
popd

echo -e "\nsrc package gmp-4.2.1.tar.gz..." | tee -a ../${LOG_FILE}
echo "________________________________________________________________________________" | tee -a ../${LOG_FILE}
pushd gmp-4.2.1
./configure | tee -a ../../${LOG_FILE}
#make | tee -a ../../${LOG_FILE}
popd

echo -e "\nsrc package linux-2.6.28.tar.gz..." | tee -a ../${LOG_FILE}
echo "________________________________________________________________________________" | tee -a ../${LOG_FILE}
pushd linux-2.6.28/
cp ../../src/579.11.114/linux* .
gunzip *.gz
cp ../../src/linux-2.6.28-dot-config .config
patch -p1 <linux-v2.6.28-picochip-3.2.4-patch
patch -p1 <linux-2.6.28-local-patch-224
# choose defaults
make oldconfig
#make | tee -a ../../${LOG_FILE}
popd
echo -e "\nsrc package iptables-1.4.2.tar.bz2..." | tee -a ../${LOG_FILE}
echo "________________________________________________________________________________" | tee -a ../${LOG_FILE}
pushd iptables-1.4.2
tar zxvf ../../src/579.11.114/iptables-1.4.2-local-patch.tar.gz
cp -rf patches/* .
./configure | tee -a ../../${LOG_FILE}
#make | tee -a ../../${LOG_FILE}
popd

echo -e "\nsrc package mtd-utils.tgz..." | tee -a ../${LOG_FILE}
echo "________________________________________________________________________________" | tee -a ../${LOG_FILE}
pushd mtd-utils-1.0.0
#make | tee -a ../../${LOG_FILE}
popd

echo -e "\nsrc package procps-3.2.7.tar.gz..." | tee -a ../${LOG_FILE}
echo "________________________________________________________________________________" | tee -a ../${LOG_FILE}
pushd procps-3.2.7
#make | tee -a ../../${LOG_FILE}
popd

echo -e "\nsrc package smartmontools-5.37.tar.gz..." | tee -a ../${LOG_FILE}
echo "________________________________________________________________________________" | tee -a ../${LOG_FILE}
pushd smartmontools-5.37
./configure | tee -a ../../${LOG_FILE}
#make | tee -a ../../${LOG_FILE}
popd

echo -e "\nsrc package i2c-tools-3.0.2.tar.bz2..." | tee -a ../${LOG_FILE}
echo "________________________________________________________________________________" | tee -a ../${LOG_FILE}
pushd i2c-tools-3.0.2
#make | tee -a ../../${LOG_FILE}
popd

echo -e "\nsrc package ipkg-0.99.163.tar.gz..." | tee -a ../${LOG_FILE}
echo "________________________________________________________________________________" | tee -a ../${LOG_FILE}
pushd ipkg-0.99.163
./configure | tee -a ../../${LOG_FILE}
#make | tee -a ../../${LOG_FILE}
popd

