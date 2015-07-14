#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "mincrypt/sha.h"
#include "bootimg.h"

typedef unsigned char byte;

int read_padding(FILE* f, unsigned itemsize, int pagesize)
{
    byte* buf = (byte*)malloc(sizeof(byte) * pagesize);
    unsigned pagemask = pagesize - 1;
    unsigned count;
    
    if((itemsize & pagemask) == 0) {
        free(buf);
        return 0;
    }
    
    count = pagesize - (itemsize & pagemask);
    
    fread(buf, count, 1, f);
    free(buf);
    return count;
}

void write_string_to_file(char* file, char* string)
{
    FILE* f = fopen(file, "a");
    fwrite(string, strlen(string), 1, f);
    fwrite("\n", 1, 1, f);
    fclose(f);
}

int usage() {
    printf("usage: unpackbootimg\n");
    printf("\t-i|--input boot.img\n");
    printf("\t[ -o|--output output_directory]\n");
    printf("\t[ -p|--pagesize <size-in-hexadecimal> ]\n");
    return 0;
}

int main(int argc, char** argv)
{
    char tmp[PATH_MAX];
    char* directory = "./";
    char* filename = NULL;
    int pagesize = 0;
    int base = 0;
    
    argc--;
    argv++;
    while(argc > 0){
        char *arg = argv[0];
        char *val = argv[1];
        argc -= 2;
        argv += 2;
        if(!strcmp(arg, "--input") || !strcmp(arg, "-i")) {
            filename = val;
        } else if(!strcmp(arg, "--output") || !strcmp(arg, "-o")) {
            directory = val;
        } else if(!strcmp(arg, "--pagesize") || !strcmp(arg, "-p")) {
            pagesize = strtoul(val, 0, 16);
        } else {
            return usage();
        }
    }
    
    if (filename == NULL) {
        return usage();
    }
    
    int total_read = 0;
    FILE* f = fopen(filename, "rb");
    boot_img_hdr header;
    
    //printf("Reading header...\n");
    int i;
    for (i = 0; i <= 512; i++) {
        fseek(f, i, SEEK_SET);
        fread(tmp, BOOT_MAGIC_SIZE, 1, f);
        if (memcmp(tmp, BOOT_MAGIC, BOOT_MAGIC_SIZE) == 0)
            break;
    }
    total_read = i;
    if (i > 512) {
        printf("Android boot magic not found.\n");
        return 1;
    }
    fseek(f, i, SEEK_SET);
    //printf("Android magic found at: %d\n", i);
    
    fread(&header, sizeof(header), 1, f);
    base = header.kernel_addr - 0x00008000;
    
    printf("  %-15s : %d\n", "page_size", header.page_size);
    printf("  %-15s : 0x%08x\n", "base_addr", base);
    //printf("BOARD_NAME %s\n", header.name);
    printf("  %-15s : 0x%08x\n", "kernel_offset", header.kernel_addr - base);
    printf("  %-15s : %d\n", "kernel_size", header.kernel_size);
    printf("  %-15s : 0x%08x\n", "ramdisk_offset", header.ramdisk_addr - base);
    printf("  %-15s : %d\n", "ramdisk_size", header.ramdisk_size);
    if (header.second_size != 0) {
        printf("  %-15s : 0x%08x\n", "second_offset", header.second_addr - base);
        printf("  %-15s : %d\n", "second_size", header.second_size);
    }
    //printf("BOARD_TAGS_OFFSET %08x\n", header.tags_addr - base);
    printf("  %-15s : %s\n", "cmdline", header.cmdline);
    
    if (pagesize == 0) {
        pagesize = header.page_size;
    }

    struct stat st = {0};

    if (stat(directory, &st) == -1) {
        mkdir(directory, 0755);
    }
    
    //printf("cmdline...\n");
    sprintf(tmp, "%s/cmdline", directory);
    remove(tmp);
    write_string_to_file(tmp, (char *)header.cmdline);
    
    sprintf(tmp, "%s/image_info", directory);
    remove(tmp);
    
    //printf("pagesize...\n");
    char pagesizetmp[200];
    sprintf(pagesizetmp, "page_size=%d", header.page_size);
    write_string_to_file(tmp, pagesizetmp);
    
    //printf("base...\n");
    char basetmp[200];
    sprintf(basetmp, "base_addr=0x%08x", base);
    write_string_to_file(tmp, basetmp);
    
    //printf("kerneloff...\n");
    char kernelofftmp[200];
    sprintf(kernelofftmp, "kernel_offset=0x%08x", header.kernel_addr - base);
    write_string_to_file(tmp, kernelofftmp);

    //printf("kernelsize...\n");
    char kernelsizetmp[200];
    sprintf(kernelsizetmp, "kernel_size=%d", header.kernel_size);
    write_string_to_file(tmp, kernelsizetmp);
    
    //printf("ramdiskoff...\n");
    char ramdiskofftmp[200];
    sprintf(ramdiskofftmp, "ramdisk_offset=0x%08x", header.ramdisk_addr - base);
    write_string_to_file(tmp, ramdiskofftmp);
    
    //printf("ramdisksize...\n");
    char ramdisksizetmp[200];
    sprintf(ramdisksizetmp, "ramdisk_size=%d", header.ramdisk_size);
    write_string_to_file(tmp, ramdisksizetmp);
    
    if (header.second_size != 0) {
        //printf("secondoff...\n");
        char secondofftmp[200];
        sprintf(secondofftmp, "second_offset=0x%08x", header.second_addr - base);
        write_string_to_file(tmp, secondofftmp);

        //printf("secondsize...\n");
        char secondsizetmp[200];
        sprintf(secondsizetmp, "second_size=%d", header.second_size);
        write_string_to_file(tmp, secondsizetmp);
    }
    
    total_read += sizeof(header);
    //printf("total read: %d\n", total_read);
    total_read += read_padding(f, sizeof(header), pagesize);
    
    sprintf(tmp, "%s/zImage", directory);
    FILE *k = fopen(tmp, "wb");
    byte* kernel = (byte*)malloc(header.kernel_size);
    //printf("Reading kernel...\n");
    fread(kernel, header.kernel_size, 1, f);
    total_read += header.kernel_size;
    fwrite(kernel, header.kernel_size, 1, k);
    fclose(k);
    
    //printf("total read: %d\n", header.kernel_size);
    total_read += read_padding(f, header.kernel_size, pagesize);
    
    sprintf(tmp, "%s/ramdisk.cpio.gz", directory);
    FILE *r = fopen(tmp, "wb");
    byte* ramdisk = (byte*)malloc(header.ramdisk_size);
    //printf("Reading ramdisk...\n");
    fread(ramdisk, header.ramdisk_size, 1, f);
    total_read += header.ramdisk_size;
    fwrite(ramdisk, header.ramdisk_size, 1, r);
    fclose(r);
    
    //printf("total read: %d\n", header.ramdisk_size);
    total_read += read_padding(f, header.ramdisk_size, pagesize);
    
    if (header.second_size != 0) {
        sprintf(tmp, "%s/second.bin", directory);
        FILE *s = fopen(tmp, "wb");
        byte* second = (byte*)malloc(header.second_size);
        //printf("Reading second...\n");
        fread(second, header.second_size, 1, f);
        total_read += header.second_size;
        fwrite(second, header.second_size, 1, s);
        fclose(s);
    }
    
    //printf("total read: %d\n", header.second_size);
    total_read += read_padding(f, header.second_size, pagesize);

#if 0
    sprintf(tmp, "%s/signature", directory);
    FILE *fsig = fopen(tmp, "wb");
    byte* bsig = (byte*)malloc(728);
    //printf("Reading signature...\n");
    fread(bsig, 728, 1, f);
    total_read += 728;
    fwrite(bsig, 728, 1, r);
    fclose(fsig);
#endif
        
    fclose(f);
    
    //printf("Total Read: %d\n", total_read);
    return 0;
}