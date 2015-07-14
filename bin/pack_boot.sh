#!/bin/bash

if [ -f boot.img ]; then 
  rm boot.img
fi

if [ -d boot/ramdisk ]; then
  cd boot
  rm -f ramdisk.*
  find . -name "*~" -exec rm -f {} \;
  
  if [ "$(uname)" == "Darwin" ]; then
    mkbootfs ramdisk | gzip > ramdisk.cpio.gz
  else
    cd ramdisk
    find | fakeroot cpio -H newc -o 2>/dev/null >../ramdisk.cpio
    cd ..
    gzip -n -9 ramdisk.cpio
  fi
  cd ..
fi

CMDLINE="$(cat boot/cmdline)"
if [[ "$CMDLINE" != *"androidboot.selinux=permissive"* ]]; then
  CMDLINE="$CMDLINE androidboot.selinux=permissive"
fi

./mkbootimg --kernel boot/zImage --ramdisk boot/ramdisk.cpio.gz --second boot/second.bin --cmdline "$CMDLINE" -o boot.img
