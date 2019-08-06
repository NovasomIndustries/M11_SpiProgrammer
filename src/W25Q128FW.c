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

extern  FT_HANDLE ftHandle;
extern  unsigned char read_buf[DATA_SIZE];
extern  int vflag;
#define SFDP_SIZE       256
unsigned char sr1 , sr2 , sr3 , sfdp[SFDP_SIZE];

unsigned char flash_JedecRead(unsigned char *reg_read)
{
unsigned char loc_tx_buff[16];
unsigned int sizeTransfered;

    loc_tx_buff[0] = CMD_GET_JEDEC_ID;                       //command
    if ( SPI_Write(ftHandle, loc_tx_buff, 1, (uint32 *)&sizeTransfered,SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES|SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE ) )
    {
        printf("Error sending CMD_GET_JEDEC_ID\n");
        ftdi_close(1);
    }

    if ( SPI_Read(ftHandle, reg_read, 3, (uint32 *)&sizeTransfered,SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES| SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE) )
        return 1;
    return 0;
}

unsigned char flash_MfgDevRead(unsigned char *reg_read)
{
unsigned char loc_tx_buff[3];
unsigned int sizeTransfered;

    loc_tx_buff[0] = CMD_GET_MFGDEV_ID;                       //command
    loc_tx_buff[1] = 0;
    loc_tx_buff[2] = 0;
    if ( SPI_Write(ftHandle, loc_tx_buff, 3, (uint32 *)&sizeTransfered,SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES|SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE ) )
    {
        printf("Error sending CMD_GET_JEDEC_ID\n");
        ftdi_close(1);
    }

    if ( SPI_Read(ftHandle, reg_read, 3, (uint32 *)&sizeTransfered,SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES| SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE) )
        return 1;
    return 0;
}



unsigned char flash_sfdpRead(void)
{
unsigned char loc_tx_buff[8];
unsigned int sizeTransfered;
int i;
    loc_tx_buff[0] = CMD_GET_SFDP;                       //command
    loc_tx_buff[1] = 0;                       //command
    loc_tx_buff[2] = 0;                       //command
    loc_tx_buff[3] = 0;                       //command
    loc_tx_buff[4] = 0;                       //command
    if ( SPI_Write(ftHandle, loc_tx_buff, 8, (uint32 *)&sizeTransfered,SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES|SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE ) )
    {
        printf("Error sending CMD_GET_JEDEC_ID\n");
        ftdi_close(1);
    }

    if ( SPI_Read(ftHandle, sfdp, SFDP_SIZE, (uint32 *)&sizeTransfered,SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES| SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE) )
        return 1;
        /*
    printf("cmd = 0x%02x\n",loc_tx_buff[0]);
    for(i=0;i<SFDP_SIZE;i++)
        printf("0x%02x,",sfdp[i]);
    printf("\n");
    */
    sr1 = flash_SingleRegRead(CMD_READ_SR1);
    sr2 = flash_SingleRegRead(CMD_READ_SR2);
    sr3 = flash_SingleRegRead(CMD_READ_SR3);
    printf("SR1 = 0x%02x\n",sr1);
    printf("SR2 = 0x%02x\n",sr2);
    printf("SR3 = 0x%02x\n",sr3);
    return 0;
}

unsigned char flash_write_enable(void)
{
unsigned int sizeTransfered;
unsigned char loc_tx_buff[2];
    loc_tx_buff[0] = CMD_WRITE_ENABLED;
    if (SPI_Write(ftHandle, loc_tx_buff, 1, (uint32 *)&sizeTransfered,SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES|SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE | SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE) )
        ftdi_close (1);//ko
    return (0);
}

unsigned char flash_write_disable(void)
{
unsigned int sizeTransfered;
unsigned char loc_tx_buff[2];
    loc_tx_buff[0] = CMD_WRITE_DISABLED;
    if (SPI_Write(ftHandle, loc_tx_buff, 1, (uint32 *)&sizeTransfered,SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES|SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE | SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE) )
        return (1);//ko
    return (0);
}

unsigned char flash_single_command(unsigned char command)
{
unsigned int sizeTransfered;
unsigned char loc_tx_buff[2];
    loc_tx_buff[0] = command;
    if (SPI_Write(ftHandle, loc_tx_buff, 1, (uint32 *)&sizeTransfered,SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES|SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE | SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE) )
        return (1);//ko
    return (0);
}

unsigned char flash_SingleRegRead(unsigned char single_reg)
{
unsigned char loc_tx_buff[10];
unsigned int sizeTransfered;

    loc_tx_buff[0] = single_reg;                       //command

    if ( SPI_Write(ftHandle, loc_tx_buff, 1, (uint32 *)&sizeTransfered,SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES|SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE ) )
    {
        printf("Error sending Single Reg Read CMD\n");
        ftdi_close(1);
    }

    if ( SPI_Read(ftHandle, loc_tx_buff, 1, (uint32 *)&sizeTransfered,SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES| SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE) )
        return 0;
    return loc_tx_buff[0];//okay
}

unsigned char flash_busy_and_error_check(void)
{
    return flash_SingleRegRead(CMD_READ_SR1) & 0x01;
}

unsigned char write_byte(unsigned char *buffer_out, unsigned int len_tx)
{
unsigned int sizeTransfered = 0;

    if (SPI_Write(ftHandle, buffer_out, len_tx, (uint32 *)&sizeTransfered,SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES|SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE | SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE) )
        return (1);//ko
    return (0);
}

void flash_getId(void)
{
int i;
    flash_sfdpRead();
    flash_JedecRead(read_buf);
    // MF7-MF0 , ID15-ID8 , ID7-ID0
    for (i=0;i<3;i++)
        printf("%d : 0x%02x\n",i,read_buf[i]);
    if ( read_buf[0] == 0xef)
    {
        if (( read_buf[1] == 0x60) && ( read_buf[2] == 0x18))
            printf("Found Winbond W25Q128FW 128 Mbit / 16 MByte flash\n");
    }
    else
    {
        printf("Wrong flash type\n");
        ftdi_close(1);
    }
    flash_MfgDevRead(read_buf);
    printf("MfgDev & ID\n");
    for (i=0;i<3;i++)
        printf("%d : 0x%02x\n",i,read_buf[i]);
}

void flash_setup(void)
{
}

unsigned char flash_SectorErase(int sector)
{
unsigned char loc_tx_buff[6];
unsigned int addr;
unsigned int    sizeTransfered;

    addr = sector * SECTOR64K_SIZE;

    loc_tx_buff[0] = CMD_BLOCK64K_ERASE;
    loc_tx_buff[1] = (unsigned char)(addr >> 16);
    loc_tx_buff[2] = (unsigned char)(addr >> 8 );
    loc_tx_buff[3] = (unsigned char)(addr);
    flash_write_enable();
    if (SPI_Write(ftHandle, loc_tx_buff, 4, (uint32 *)&sizeTransfered,SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES|SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE | SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE) )
        ftdi_close (1);
    while( flash_busy_and_error_check() )
        usleep(100);
    flash_SingleRegRead(CMD_GET_STATUS);
    flash_write_disable();
    printf(" Sector %d erased\r",sector);
    fflush(stdout);
    return 0;
}

unsigned char flash_Write(unsigned char *write_buf , int len_data , unsigned int write_address)
{
unsigned char   loc_tx_buff[PAGE_SIZE+CMD_LEN];
unsigned int    i,sizeTransfered;
char strc = '|';

	while (len_data > 0)
	{
        switch (strc)
        {
            case '|' : strc = '/';break;
            case '/' : strc = '-';break;
            case '-' : strc = '\\';break;
            case '\\' : strc = '|';break;
            default : strc = '|' ; break;
        }
        printf(" Writing ... %c\r", strc);
        fflush(stdout);
        loc_tx_buff[0] = CMD_PROGRAM_PAGE;
        loc_tx_buff[1] = (unsigned char)(write_address >> 16);
        loc_tx_buff[2] = (unsigned char)(write_address >> 8 );
        loc_tx_buff[3] = (unsigned char)(write_address);

        for (i = 0; i < PAGE_SIZE; i++)
            loc_tx_buff[i + CMD_LEN] = *write_buf++;

        while( flash_busy_and_error_check() )
            usleep(100);

        flash_write_enable();
        if (SPI_Write(ftHandle, loc_tx_buff, PAGE_SIZE+CMD_LEN, (uint32 *)&sizeTransfered,SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES|SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE | SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE) )
            ftdi_close (1);
        while( flash_busy_and_error_check() )
            usleep(100);
		write_address += PAGE_SIZE;
		len_data -= PAGE_SIZE;
	}
    return 0;
}

unsigned char flash_Read(unsigned int address, unsigned int len_buff, unsigned char *read_buffer)
{
unsigned char  cmd_buf[6];
unsigned int sizeTransfered = 0 , k = 0;

    printf("Start Read %d\n",len_buff);
    while(flash_busy_and_error_check())
        usleep(100);
    while( len_buff > FTDI_BLOCK_SIZE )
    {
        cmd_buf[0] = CMD_READ_PAGE;
        cmd_buf[1] = (unsigned char)(address >> 16);
        cmd_buf[2] = (unsigned char)(address >> 8);
        cmd_buf[3] = (unsigned char)address;
        if (SPI_Write(ftHandle, cmd_buf, CMD_LEN, (uint32 *)&sizeTransfered,SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES|SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE ) != FT_OK)
            ftdi_close (1);
        printf("Read at sector %d ( 0x%08x )\n",address/SECTOR64K_SIZE,address);
        for( k=0; k < (SECTOR64K_SIZE/FTDI_BLOCK_SIZE)-1; k++)
        {
            if (SPI_Read(ftHandle, read_buffer, FTDI_BLOCK_SIZE, (uint32 *)&sizeTransfered,SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES|SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE) != FT_OK)
                ftdi_close (1);
            address += FTDI_BLOCK_SIZE;
            read_buffer += FTDI_BLOCK_SIZE;
            len_buff -= FTDI_BLOCK_SIZE;
        }
        if (SPI_Read(ftHandle, read_buffer, FTDI_BLOCK_SIZE, (uint32 *)&sizeTransfered,SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES|SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE) != FT_OK)
            ftdi_close (1);
        address += FTDI_BLOCK_SIZE;
        read_buffer += FTDI_BLOCK_SIZE;
        len_buff -= FTDI_BLOCK_SIZE;
    }

    printf("Read done\n");
    return 0;
}

unsigned char flash_BulkErase(void)
{
unsigned char loc_tx_buff[6];
unsigned int rd_wr_len  = 0;

    /* write enable */
    flash_write_enable();

    /* ERASE */
    loc_tx_buff[0] = CMD_BULK_ERASE;
    rd_wr_len      = 1;
    if (write_byte(loc_tx_buff, rd_wr_len))
         return 1;

    flash_write_disable();
    return 0;
}

