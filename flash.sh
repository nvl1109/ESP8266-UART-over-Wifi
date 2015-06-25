#!/bin/bash
flashrom=0
flashblank=0
flashconfig=0
runuart=0
usecom=/dev/ttyUSB0

if [[ "x${1}x" == "xx" ]]; then
  flashrom=1;
  runuart=1;
else
  if [[ `expr index "$1" r` > 0 ]]; then
    flashrom=1;
  fi
  if [[ `expr index "$1" u` > 0 ]]; then
    runuart=1;
  fi
  if [[ "x${2}x" != "xx" ]]; then
    usecom=${2}
  fi
fi

if [[ $flashrom -gt 0 ]]; then
  echo "============FLASH NEW ROM============"
  ./esptool.py --port ${usecom} write_flash 0x00000 ../bin/eagle.flash.bin 0x40000 ../bin/eagle.irom0text.bin;
fi

if [[ $runuart -gt 0 ]]; then
  echo "============RUN UART=============="
  ./miniterm.py ${usecom} 115200;
fi
