#ifndef W25Q128JV_H_INCLUDED
#define W25Q128JV_H_INCLUDED

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define CMD_GET_MFGDEV_ID           0x90
#define CMD_GET_JEDEC_ID            0x9F
#define CMD_GET_SFDP                0x5a
#define CMD_WRITE_ENABLED           0x06
#define CMD_WRITE_DISABLED          0x04
#define CMD_GET_STATUS              0x05
#define CMD_CLEAR_STATUS            0x30
#define CMD_READ_PAGE               0x03
#define CMD_PROGRAM_PAGE            0x02

#define CMD_BLOCK64K_ERASE          0xD8
#define CMD_BULK_ERASE              0x60
#define CMD_READ_SR1                0x05
#define CMD_READ_SR2                0x35
#define CMD_READ_SR3                0x15

#define PAGE_SIZE                   256         /* 256 bytes */
#define SECTOR32K_SIZE              32768       /* 64k sector size*/
#define SECTOR64K_SIZE              65536       /* 64k sector size*/
#define FLASH_SIZE                  (8192*1024)  /* 8 MBytes*/
#define CMD_LEN     4


unsigned char Flash_Read_File(unsigned int address, unsigned int len_buff, unsigned char *read_buffer);

unsigned char Flash_Write_File(const char * file_path);

uint8_t Flash_Init(FT_HANDLE handle);

unsigned char flash_write_enable(void);
unsigned char flash_write_disable(void);

unsigned char flash_busy_and_error_check(void);
unsigned char flash_SingleRegRead(unsigned char single_reg);

unsigned char write_byte(unsigned char *buffer_out, unsigned int len_tx);
unsigned char read_byte(unsigned char *buffer_out, unsigned char *buffer_in, unsigned int len_tx, unsigned int len_rx);

void flash_SetPageSize(unsigned char page_size);


unsigned char print_file_in_buffer(const char *file_path, unsigned char *buffer_out, unsigned int offset);
unsigned char SectorErase(unsigned int addr);
unsigned char BulkErase(void);



#endif // MT25QU256ABA_H_INCLUDED
