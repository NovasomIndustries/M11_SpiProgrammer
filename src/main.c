#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "ftd2xx.h"
#include "libMPSSE_spi.h"
#include "MT25QU256ABA.h"
#include "M11_SpiProgrammer.h"
/* use with
sudo rmmod ftdi_sio ; sleep 1; sudo LD_LIBRARY_PATH=libs:$LD_LIBRARY_PATH bin/Debug/SpiIf ->command>
*/
FT_HANDLE ftHandle;
unsigned char read_buf[DATA_SIZE];
ChannelConfig channelConf = {0};
unsigned char AC_BUSbit_Value = DEFAULT_PIN_VALUE;
int vflag = 0;

spi_files flash_files[4] =
{
    {
        .file_name = "APLI_IFWI_X64_R_151_25.bin",
        .file_len = 0,
        .sector_num = 0,
        .address = 0x00000000,
        .max_len = (4096*1024)
    },
    {
        .file_name = "closing_flag",
        .file_len = 0,
        .sector_num = 0,
        .address = 0xffa00000,
        .max_len = (0*0)
    },
};


extern  unsigned char MT25QU256ABA_id[8];

struct timeval t1, t2;
double elapsedTime;

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

int main(int argc, char **argv)
{
unsigned int i = 0 , j , k;
int opflag=0 , file_len , block_num=-1 ;
char *cvalue = NULL;
int c;
FILE    *fp;
char    strc = '|' , filearg[256];;
int file_number=0, closing_flag=0;
char version[32] = "1.0.0";

    printf("M11_SpiProgrammer : M11 USB flasher V%s\n",version);
    system("sudo rmmod ftdi_sio > /dev/null 2>&1 ; sleep 1");

    while ((c = getopt (argc, argv, "bw:r:s:v?")) != -1)
    {
        switch (c)
        {
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
        printf("Otions are : b (bulk erase )  , w ( write file ), r <file> ( read file ) , s <sector_erase number> ( erase sectors ) , v ( verbose )\n");
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
    flash_getId();
    //ftdi_close(0);

    flash_setup();

    file_number=0;
    closing_flag=0;

    switch ( opflag )
    {
        case OP_SECTORERASE :
                                printf("Erasing sector %d ( address 0x%08x )\n",flash_files[i].address / SECTOR_SIZE,flash_files[i].address);
                                measure_time_start();
                                flash_SectorErase(block_num);
                                measure_time_end();
                                printf("Sector %d erased in %f mSec.\n",flash_files[i].address / SECTOR_SIZE,elapsedTime);
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
                                        printf("Opening %s with %d\n",flash_files[file_number].file_name,file_number);
                                        fp = fopen(flash_files[file_number].file_name, "rb");
                                        if ( fp )
                                        {
                                            fseek(fp, 0L, SEEK_END);
                                            flash_files[file_number].file_len = ftell(fp);
                                            flash_files[file_number].sector_num = (flash_files[file_number].file_len / SECTOR_SIZE) + 1;
                                            rewind(fp);
                                            file_len = fread(read_buf, 1 , flash_files[file_number].file_len, fp);
                                            fclose(fp);
                                            printf("File %s @         %d\n",flash_files[file_number].file_name,file_number);
                                            printf("File %s size      %d\n",flash_files[file_number].file_name,flash_files[file_number].file_len);
                                            printf("File %s sectors   %d\n",flash_files[file_number].file_name,flash_files[file_number].sector_num);
                                            printf("Flash page size   %d\n",PAGE_SIZE);
                                            printf("Flash sector size %d\n",SECTOR_SIZE);
                                        }
                                        else
                                        {
                                            printf("File %s not found\n",flash_files[file_number].file_name);
                                            exit(1);
                                        }
                                        for ( k=0; k<flash_files[file_number].sector_num; k++)
                                        {
                                            printf("Erasing sector %d ( address 0x%08x )\n",flash_files[file_number].address / SECTOR_SIZE + k,flash_files[file_number].address+(SECTOR_SIZE*k));
                                            measure_time_start();
                                            flash_SectorErase(flash_files[file_number].address / SECTOR_SIZE + k);
                                            measure_time_end();
                                            printf("Sector %d erased in %f mSec.\n",flash_files[file_number].address / SECTOR_SIZE + k,elapsedTime);
                                        }
                                        printf("Writing file %s at 0x%08x, %d bytes ( %d sectors ) , ETA %d sec.\n",flash_files[file_number].file_name,flash_files[file_number].address,file_len,flash_files[file_number].sector_num,((flash_files[file_number].sector_num*256)*11)/1000);
                                        measure_time_start();
                                        if (flash_Write(read_buf , file_len , flash_files[file_number].address ) )
                                        {
                                            printf("flash_Write error\n");
                                            exit(1);
                                        }
                                        measure_time_end();
                                        printf("File %s written at address 0x%08x, took %f sec.\n",flash_files[file_number].file_name,flash_files[file_number].address,elapsedTime/1000);
                                        file_number++;
                                    }
                                }
                                break;
        case OP_READFLASH :
                                for(j=0;j<DATA_SIZE;j++)
                                    read_buf[j] = 0xff;
                                if (flash_Read(0x00, (unsigned int )DATA_SIZE, read_buf))
                                //if (flash_Read(0x00, 8192, read_buf))
                                {
                                    printf("\n Error reading Flash!!!\n");
                                    exit(1);
                                }
                                fp = fopen(cvalue, "wb");
                                fwrite(read_buf, sizeof(read_buf), 1, fp);
                                fclose(fp);
                                break;
        default :               exit (1);

    }


    ftdi_close(0);
	printf("Closed\n");
}
