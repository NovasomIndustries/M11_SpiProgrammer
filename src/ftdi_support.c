#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "ftd2xx.h"
#include "libMPSSE_spi.h"
#include "s25fs512.h"
#include "M11_SpiProgrammer.h"

extern  unsigned char AC_BUSbit_Value;
extern  FT_HANDLE ftHandle;
extern  ChannelConfig channelConf ;
extern  unsigned char read_buf[DATA_SIZE];

void set_OE_Buffer_pin(unsigned char out_value)
{
    if (out_value)
        AC_BUSbit_Value |= OE_BUFFER_PIN;
    else
        AC_BUSbit_Value &= ~OE_BUFFER_PIN;
    FT_WriteGPIO(ftHandle, GPIO_DIR_OUT, AC_BUSbit_Value);
}

void set_Fault_Led_pin(unsigned char out_value)
{
    if (out_value)
        AC_BUSbit_Value |= FAULT_LED_PIN;
    else
        AC_BUSbit_Value &= ~FAULT_LED_PIN;
    FT_WriteGPIO(ftHandle, GPIO_DIR_OUT, AC_BUSbit_Value);
}

void set_Reset_pin(unsigned char out_value)
{
    if (out_value)
        AC_BUSbit_Value |= RESET_PIN;
    else
        AC_BUSbit_Value &= ~RESET_PIN;
    FT_WriteGPIO(ftHandle, GPIO_DIR_OUT, AC_BUSbit_Value);
}

void ftdi_init(void)
{
unsigned char latency = 5;
unsigned int channels = 0;
unsigned int i;
int vflag = 0;

	FT_STATUS status = FT_OK;
	FT_DEVICE_LIST_INFO_NODE devList = {0};

	channelConf.ClockRate = 30000000;//3000000
	channelConf.LatencyTimer = latency;
	channelConf.configOptions = SPI_CONFIG_OPTION_MODE0 | SPI_CONFIG_OPTION_CS_DBUS3 | SPI_CONFIG_OPTION_CS_ACTIVELOW;
	channelConf.Pin = 0;    /*  FinalVal-FinalDir-InitVal-InitDir (for dir 0=in, 1=out) */

	status = SPI_GetNumChannels((uint32 *)&channels);
	APP_CHECK_STATUS(status);
    if(channels > 0)
    {
        printf("Found %d available SPI channel(s)\n",(int)channels);
        if ( vflag == 1 )
        {
            for(i=0;i<channels;i++)
            {
                status = SPI_GetChannelInfo(i,&devList);
                APP_CHECK_STATUS(status);
                if ( vflag == 1 )
                {
                    printf("Information on channel number %d:\n",i);
                    printf("		Flags=0x%x\n",devList.Flags);
                    printf("		Type=0x%x\n",devList.Type);
                    printf("		ID=0x%x\n",devList.ID);
                    printf("		LocId=0x%x\n",devList.LocId);
                    printf("		SerialNumber=%s\n",devList.SerialNumber);
                    printf("		Description=%s\n",devList.Description);
                    printf("		ftHandle=0x%x\n",(unsigned int)devList.ftHandle);// is 0 unless open
                }
            }
        }
    }
    else
    {
        printf("No channels found!\n");
        exit (1);
    }

    /* Open the first available channel */
    status = SPI_OpenChannel(CHANNEL_TO_OPEN,&ftHandle);
    APP_CHECK_STATUS(status);
    if ( vflag == 1 )
        printf("handle=0x%x status=0x%x\n",(unsigned int)ftHandle,status);
    status = SPI_InitChannel(ftHandle,&channelConf);
    APP_CHECK_STATUS(status);

    status = SPI_Write(ftHandle, read_buf, 0, (uint32 *)&i,SPI_TRANSFER_OPTIONS_SIZE_IN_BITS|SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE);
    APP_CHECK_STATUS(status);

    AC_BUSbit_Value = DEFAULT_PIN_VALUE;
    FT_WriteGPIO(ftHandle, GPIO_DIR_OUT, AC_BUSbit_Value); /* all out inactive */

}

void ftdi_close(int ret_val)
{
    if ( ret_val)
        FT_WriteGPIO(ftHandle, GPIO_DIR_OUT, FAILED_PIN_VALUE);     /* fail active */
    else
        FT_WriteGPIO(ftHandle, GPIO_DIR_OUT, DEFAULT_PIN_VALUE);    /* all out inactive */
    SPI_CloseChannel(ftHandle);
    exit(ret_val);
}
