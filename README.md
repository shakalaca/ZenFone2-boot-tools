# ZenFone2-boot-tools
Pack and unpack boot.img or recovery.img of ZenFone2
source code from: 
* https://android.googlesource.com/platform/system/core/mkbootimg
* https://android.googlesource.com/platform/system/core/libmincrypt
* https://android.googlesource.com/platform/system/core/include/mincrypt

### unpack boot.img or recovery.img
```
$ unpackbootimg -i boot.img 
```

### pack boot.img or recovery.img
```
$ mkbootimg --kernel zImage \
            --ramdisk ramdisk.cpio.gz \
            --cmdline "$(cat cmdline)" \
            --second second.bin \
            --signature signature \
            -o boot.img
```
