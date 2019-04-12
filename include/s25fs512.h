#ifndef S25FS512_H_INCLUDED
#define S25FS512_H_INCLUDED




#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>





#define CMD_GET_IDENTIFICATION      0x9F
#define CMD_FORMAT_CHIP             0x60
#define CMD_WRITE_ENABLED           0x06
#define CMD_WRITE_DISABLED          0x04
#define CMD_GET_STATUS              0x05
#define CMD_CLEAR_STATUS            0x30
#define CMD_WRITE_512B_PAGE         0x02
#define CMD_READ_512B_PAGE          0x0B
#define CMD_ERASE_512B_PAGE         0xD8


//#define CMD_4FAST_READ_PAGE         0x0C
#define CMD_4FAST_READ_PAGE         0x13
#define CMD_4PROGRAM_PAGE           0x12
#define CMD_4ERASE_SECTOR           0xDC
#define CMD_4P4ERASE_SECTOR         0x21
#define CMD_BULK_ERASE              0x60
#define CMD_RSTEN                   0x66
#define CMD_RESET                   0x99

#define CMD_READ_CR                 0x35
#define CMD_READ_SR1                0x05
#define CMD_WRR                     0x01

#define PAGE_SIZE                   0x200       /*512 bytes => totale 131072 pagine*/
#define SECTOR_SIZE                 0x40000     /* 256 settori totale 256KBytes*/
#define FLASH_SIZE                  0X4000000   /* 512MBits => 64MBytes*/


#define PAGE_SIZE_512               0x01
#define PAGE_SIZE_256               0x00

#define WIP_BIT                     0x01
#define E_ERR_BIT                   0x20

#define RDAR                        0x65
#define WRAR                        0x71

#define SR1V                        0x00800000
#define SR2V                        0x00800001
#define CR1V                        0x00800002
#define CR2V                        0x00800003
#define CR3V                        0x00800004
#define CR4V                        0x00800005
#define VDLR                        0x00800010
#define PPBL                        0x00800040

#define PPBL_CMD                    0xA7
#define PPBAR_CMD                   0xFC
#define DYBAR_CMD                   0xFA
#define PASSWDR_CMD                 0xE7
#define ASPR_CMD                    0x2B
#define E4BAM_CMD                   0xB7
#define MAX_RETRY                   1000

unsigned char Flash_Read_File(unsigned int address, unsigned int len_buff, unsigned char *read_buffer);

unsigned char Flash_Write_File(const char * file_path);

uint8_t Flash_Init(FT_HANDLE handle);

unsigned char flash_write_enable(void);
unsigned char flash_write_disable(void);

unsigned char flash_busy_and_error_check(void);

unsigned char write_byte(unsigned char *buffer_out, unsigned int len_tx);
unsigned char read_byte(unsigned char *buffer_out, unsigned char *buffer_in, unsigned int len_tx, unsigned int len_rx);

unsigned char flash_WRAR(unsigned int addr_reg_to_read, unsigned char data_to_write);
unsigned char flash_RDAR(unsigned int addr_reg_to_read, unsigned char *reg_read);
void flash_SetPageSize(unsigned char page_size);


unsigned char print_file_in_buffer(const char *file_path, unsigned char *buffer_out, unsigned int offset);
unsigned char SectorErase(unsigned int addr);
unsigned char BulkErase(void);



#endif // S25FS512_H_INCLUDED
