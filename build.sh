#!/bin/bash

export PATH=`pwd`/../../xtensa-lx106-elf/bin:$PATH
touch user/user_main.c

make COMPILE=gcc BOOT=none APP=0 SPI_SPEED=40 SPI_MODE=QIO SPI_SIZE_MAP=0
