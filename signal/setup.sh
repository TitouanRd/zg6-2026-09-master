#!/bin/sh

export TOOLCHAIN=/opt/arm/arm-gnu-toolchain-14.3.rel1-x86_64-arm-none-eabi
export SDK_ELEC=../sdk
export PATH=$SDK_ELEC/bin:$TOOLCHAIN/bin:$PATH
