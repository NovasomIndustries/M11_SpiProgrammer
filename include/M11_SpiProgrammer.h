/******************************************************************************/
/*								Macro and type defines							   */
/******************************************************************************/
/* Helper macros */

#define APP_CHECK_STATUS(exp) {if(exp!=FT_OK){printf("%s:%d:%s(): status(0x%x) \
!= FT_OK\n",__FILE__, __LINE__, __FUNCTION__,exp);exit(1);}else{;}};
#define CHECK_NULL(exp){if(exp==NULL){printf("%s:%d:%s():  NULL expression \
encountered \n",__FILE__, __LINE__, __FUNCTION__);exit(1);}else{;}};

/* Application specific macro definitions */
#define SPI_DEVICE_BUFFER_SIZE		256
#define SPI_WRITE_COMPLETION_RETRY		10
#define START_ADDRESS_EEPROM 	0x00 /*read/write start address inside the EEPROM*/
#define END_ADDRESS_EEPROM		0x10
#define RETRY_COUNT_EEPROM		10	/* number of retries if read/write fails */
#define CHANNEL_TO_OPEN			0	/*0 for first available channel, 1 for next... */
#define SPI_SLAVE_0				0
#define SPI_SLAVE_1				1
#define SPI_SLAVE_2				2
#define DATA_OFFSET				4
#define USE_WRITEREAD			0

#define FAULT_LED_PIN   0x02
#define OE_BUFFER_PIN   0x04
#define RESET_PIN       0x08
#define DEFAULT_PIN_VALUE   0x06
#define FAILED_PIN_VALUE    0x04

#define GPIO_DIR_OUT    0x0E  //ACBUS0 IN, ACBUS1/2/3 OUT

#define OP_BULKERASE                1
#define OP_WRITEBLOB                2
#define OP_READFLASH                3
#define OP_WRITERCW                 4
#define OP_READRCW                  5
#define OP_SECTORERASE              6

#define DATA_SIZE                   (4096*1024)

#define FTDI_BLOCK_SIZE      8192


typedef struct file_struct {
    char  file_name[1024];
    int   file_len;
    int   sector_num;
    int   address;
    int   max_len;
} spi_files;

extern  void ftdi_init(void);
extern  void flash_setup(void);
extern  unsigned char flash_SectorErase(int sector);
extern  unsigned char flash_BulkErase(void);
extern  unsigned char flash_Write(unsigned char *write_buf , int len_data , unsigned int write_address);
extern  unsigned char flash_Read(unsigned int address, unsigned int len_buff, unsigned char *read_buffer);
extern  void ftdi_close(int ret_val);
extern  void set_Reset_pin(unsigned char out_value);
extern  void set_Fault_Led_pin(unsigned char out_value);
extern  void set_OE_Buffer_pin(unsigned char out_value);
extern  void flash_getId(void);

