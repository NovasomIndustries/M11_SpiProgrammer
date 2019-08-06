#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "ftd2xx.h"
#include "libMPSSE_spi.h"
#include "W25Q128FW.h"
#include "M11_SpiProgrammer.h"
/* use with
sudo rmmod ftdi_sio ; sleep 1; sudo LD_LIBRARY_PATH=libs:$LD_LIBRARY_PATH bin/Debug/SpiIf ->command>
*/
FT_HANDLE ftHandle;
unsigned char read_buf[DATA_SIZE] , buffer_buf[DATA_SIZE];
ChannelConfig channelConf = {0};
unsigned char AC_BUSbit_Value = DEFAULT_PIN_VALUE;
int vflag = 0;
#define TIME_FOR_SECTOR_WRITE   3339

spi_files flash_files[4] =
{
    {
        .file_name = "/Devel/IntelBios/IntelAtomE3900SoCFamilyIFWI151_25ReleasePackage/APLI_IFWI_X64_R_151_25.bin",
        .file_len = 0,
        .blocks64k_num = 0,
        .address = 0x00000000,
        .max_len = DATA_SIZE
    },
    {
        .file_name = "closing_flag",
        .file_len = 0,
        .blocks64k_num = 0,
        .address = 0xffa00000,
        .max_len = (0*0)
    },
};

struct timeval t1, t2;
double elapsedTime,erasetime,writetime;

void measure_time_start(void)
{
    gettimeofday(&t1, NULL);
}

void measure_time_end(void)
{
    gettimeofday(&t2, NULL);
    elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
    elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;
}

void swap_endianness(int len)
{
int i;
    for(i=0;i<len;i++)
        read_buf[i] = __builtin_bswap32(read_buf[i]);
}

int main(int argc, char **argv)
{
unsigned int i = 0 , j , k;
int opflag=0 , block_num=-1 ;
char *cvalue = NULL;
int c;
FILE    *fp;
char    strc = '|' , filearg[256];;
int file_number=0, closing_flag=0;
char version[32] = "1.0.0";

    printf("M11_SpiProgrammer : M11 USB flasher V%s\n",version);
    system("sudo rmmod ftdi_sio > /dev/null 2>&1 ; sleep 1");
    while ((c = getopt (argc, argv, "cbw:r:s:v?")) != -1)
    {
        switch (c)
        {
            case 'c':   opflag = OP_CHECKFLASH;
                        break;
            case 'b':   opflag = OP_BULKERASE;
                        break;
            case 'w':   opflag = OP_WRITEBLOB;
                        sprintf(filearg,"%s\n", optarg);
                        break;
            case 'r':   opflag = OP_READFLASH;
                        cvalue = optarg;
                        break;
            case 's':   opflag = OP_SECTORERASE;
                        block_num = atoi(optarg);
                        break;
            case 'v':   vflag = 1;
                        break;
            case '?':   printf ("Options ?\nwas %c\n",c);
                        exit (1);
            default:
                        exit(1);
        }
    }
    if ( opflag == 0 )
    {
        printf("Otions are : b (bulk erase )  , w ( write file ), r <file> ( read file ) , s <sector_erase number> ( erase sectors ) , c ( check flash ) , v ( verbose )\n");
        printf ("c = %d , opflag = %d , cvalue = %s\n", c , opflag, cvalue);
        exit(1);
    }

    if (( opflag == OP_READFLASH ) && (strlen(cvalue) < 2))
    {
        printf("Unspecified read file!\n");
        exit (1);
    }
    if (( opflag == OP_SECTORERASE ) && (block_num == -1))
    {
        printf("Wrong sector number!\n");
        exit (1);
    }
    if (( opflag == OP_WRITEBLOB ) && (strlen(filearg) > 0))
    {
        if ( strncmp(filearg,"default",sizeof("default")-1) == 0)
            printf("Using default files\n");
        else
        {
            char delim[] = " ";
            char *ptr = strtok(filearg, delim);

            i=0;
            while(ptr != NULL)
            {
                snprintf(flash_files[i].file_name,strlen(ptr),"%s",ptr);
                printf("file %s --\n",flash_files[i].file_name);
                ptr = strtok(NULL, delim);
                i++;
            }
        }
    }
    ftdi_init();
    set_OE_Buffer_pin(0);

    flash_setup();

    file_number=0;
    closing_flag=0;

    switch ( opflag )
    {
        case OP_CHECKFLASH :
                                flash_getId();
                                break;
        case OP_SECTORERASE :
                                printf("Erasing sector %d ( address 0x%08x )\n",flash_files[i].address / SECTOR64K_SIZE,flash_files[i].address);
                                measure_time_start();
                                flash_SectorErase(block_num);
                                measure_time_end();
                                printf("Sector %d erased in %f mSec.\n",flash_files[i].address / SECTOR64K_SIZE,elapsedTime);
                                break;
        case OP_BULKERASE :
                                printf("Bulk erasing\n");
                                measure_time_start();
                                flash_BulkErase();
                                while (flash_busy_and_error_check())
                                {
                                    switch (strc)
                                    {
                                        case '|' : strc = '/';break;
                                        case '/' : strc = '-';break;
                                        case '-' : strc = '\\';break;
                                        case '\\' : strc = '|';break;
                                        default : strc = '|' ; break;
                                    }
                                    printf("%c\r", strc); fflush(stdout);
                                    usleep(500000);
                                }
                                measure_time_end();
                                printf("Bulk erased in %f mSec.\n",elapsedTime);
                                break;
        case OP_WRITEBLOB :
                                while(closing_flag == 0)
                                {
                                    if ( strcmp(flash_files[file_number].file_name , "closing_flag") == 0 )
                                    {
                                        closing_flag = 1;
                                        break;
                                    }
                                    else
                                    {
                                        for(j=0;j<flash_files[file_number].max_len;j++)
                                            read_buf[j] = 0xff;
                                        fp = fopen(flash_files[file_number].file_name, "rb");
                                        if ( fp )
                                        {
                                            fseek(fp, 0L, SEEK_END);
                                            flash_files[file_number].file_len = ftell(fp);
                                            flash_files[file_number].blocks64k_num = (flash_files[file_number].file_len / SECTOR64K_SIZE) + 1;
                                            rewind(fp);
                                            fread(read_buf, 4 , flash_files[file_number].file_len/4, fp);
                                            fclose(fp);
                                            printf("File                    %s\n",flash_files[file_number].file_name);
                                            printf("File  size              %d\n",flash_files[file_number].file_len);
                                            printf("Index                   %d\n",file_number);
                                            printf("Flash page write size   %d\n",PAGE_SIZE);
                                            printf("Flash sector erase size %d\n",SECTOR64K_SIZE);
                                            printf("ETA                     %d sec.\n",((flash_files[file_number].blocks64k_num*TIME_FOR_SECTOR_WRITE)/1000)+16);
                                        }
                                        else
                                        {
                                            printf("File %s not found\n",flash_files[file_number].file_name);
                                            ftdi_close(1);

                                        }
                                        //swap_endianness(flash_files[file_number].file_len);
                                        printf("Writing at 0x%08x, %d bytes ( %d sectors )\n",flash_files[file_number].address,flash_files[file_number].file_len,flash_files[file_number].blocks64k_num);
                                        printf("Erasing %d sectors\n",flash_files[file_number].blocks64k_num);
                                        measure_time_start();
                                        for ( k=0; k<flash_files[file_number].blocks64k_num; k++)
                                        {
                                            flash_SectorErase(flash_files[file_number].address / SECTOR64K_SIZE + k);
                                        }
                                        measure_time_end();
                                        printf("Erased %d sectors in %d sec.\n",flash_files[file_number].blocks64k_num,(int )elapsedTime/1000);
                                        erasetime = elapsedTime;
                                        measure_time_start();
                                        if (flash_Write((unsigned char *)read_buf , flash_files[file_number].file_len , flash_files[file_number].address ) )
                                        {
                                            printf("flash_Write error\n");
                                            exit(1);
                                        }
                                        measure_time_end();
                                        writetime = elapsedTime;
                                        printf("File %s written at address 0x%08x\nTimings : %f sec. for erase and %f sec. for write, total %f sec.\n",flash_files[file_number].file_name,flash_files[file_number].address,erasetime/1000,writetime/1000,erasetime/1000+writetime/1000);
                                        file_number++;
                                    }
                                }
                                break;
        case OP_READFLASH :
                                for(j=0;j<DATA_SIZE;j++)
                                    read_buf[j] = 0xff;
                                if (flash_Read(0x00, (unsigned int )DATA_SIZE, (unsigned char *)read_buf))
//                                if (flash_Read(0x00, 65536, (unsigned char *)read_buf))
                                {
                                    printf("\n Error reading Flash!!!\n");
                                    exit(1);
                                }
                                printf("read_buf : %d\n",sizeof(read_buf));
                                fp = fopen(cvalue, "wb");
                                fwrite(read_buf, sizeof(read_buf), 1, fp);
                                fclose(fp);
                                break;
        default :               exit (1);

    }


    ftdi_close(0);
	printf("Closed\n");
}
