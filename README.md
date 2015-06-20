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
$ mkbootimg --kernel boot.img-zImage \
            --ramdisk boot.img-ramdisk.gz \
            --cmdline "$(cat boot.img-cmdline)" \
            --second boot.img-second \
            --signature boot.img-signature \
            -o boot.img
```
