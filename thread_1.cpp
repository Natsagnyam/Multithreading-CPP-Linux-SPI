
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
//#include <getopt.h>
#include <time.h>

//#include "inc/deviceMap.h"
 
#include "inc/linux_spi.h"
#include "inc/FastCRC32.h"
#include "inc/App_SLE6000_SPI_Protocol_V0_C.h"
#include "inc/globals.h"

    //Sys comm defines
#define ENABLED		255
#define DISABLED	0
#define DLENGTH1	1
#define DLENGTH2	2 //data length 2
#define DLENGTH3	3
#define DLENGTH4	4
#define DLENGTH5	5
#define DLENGTH6	6
#define DLENGTH7	7



#define TEST_MAJOR_VERSION  (1)
#define TEST_MINOR_VERSION  (0)
#define TEST_PATCH_VERSION  (0)
// #define SPI_RAW_RX_TX_BUFFER_SIZE      2060U    // outer frame Rx
#define MAX_NUM_OF_INNER_PACKETS_SPI_RX_TX         77U      // INNER frame Rx
#define TOTAL_NUMBER_OF_SPI_DEVICES    (1U)
#define PSU_COMMAND 0xA0U
#define PC_POWER_DOWN_EXECUTE 0x88U
//! Number of Bytes to the beginning of the CRC block, in the raw packet
#define SPI_RAW_CRC_POSITION 2034U
#define STACKSIZE					(0x00010000)
#define SPI_RAW_RX_TX_BUFFER_SIZE       2060U
#define SLE_THREAD_DATA_SIZE            100U
#define SLE_THREAD_NUMBER               3
#define SLE_TOTAL_THREAD_DATA_NUMBER    ((SLE_THREAD_NUMBER) * (SLE_THREAD_DATA_SIZE))

// #define IMX_GPIO_NR(port, index)    ((((port)-1)*32)+((index)&31))

void GenerateCRCandAddToOutboundPacket(CommandAndResponse::SPI_A_Outer_struct_Ascii_type*  g_SPI_UI_To_MMS_Packet);


enum { SLE_TOTAL_THREAD_DATA_NUMBER__ = SLE_TOTAL_THREAD_DATA_NUMBER };
enum { SLE_THREAD_DATA_SIZE__ = SLE_THREAD_DATA_SIZE };
enum { NULL__, SET___}; 


//unsigned char Tx_buff[SPI_RAW_RX_TX_BUFFER_SIZE];

//static    uint8_t		m_byTrackingByteNumberController = NULL__;	//!< The current tracking byte number for Controller CAN messages
static    uint8_t		m_byTrackingByteNumberMonitor = NULL__;		//!< The current tracking byte number for Monitor CAN Messages
static    uint8_t		m_byTrackingByteNumberMMS = NULL__;			//!< The current tracking byte number for MMS Local CAN Messages. // command counter++ for SPI_SUBS_ID_MMS_LOCAL
//static    uint8_t		m_byTrackingByteNumberESMO = NULL__;			//!< The current tracking byte number for MMS Local CAN Messages

// C_Linux_SPI             *ptr_SPIdev;



/* Rx Outer frame*/
typedef union SPI_OuterFrame {
    unsigned char outerBuff[sizeof(CommandAndResponse::SPI_A_Outer_struct_Ascii_type)];     // [SPI_RAW_RX_TX_BUFFER_SIZE]; 
    CommandAndResponse::SPI_A_Outer_struct_Ascii_type spi_OuterFrame;
} SPI_OuterFrame_t;
static SPI_OuterFrame_t g_SPI_Rx_outerFrame;
static SPI_OuterFrame_t g_SPI_Tx_outerFrame;


/* Rx inner frame */
typedef union SPI_InnerFrame {
    unsigned char InnerBuff[MAX_NUM_OF_INNER_PACKETS_SPI_RX_TX * sizeof(CommandAndResponse::Raw_UI_To_MMS_Data_struct_type)]; 
    CommandAndResponse::Raw_UI_To_MMS_Data_struct_type  spi_InnerFrame[MAX_NUM_OF_INNER_PACKETS_SPI_RX_TX];  // inner frame
} SPI_InnerFrame_t;
static SPI_InnerFrame_t g_SPI_Rx_InnerFrame;
static SPI_InnerFrame_t g_SPI_Tx_InnerFrame;
//static  unsigned char tempBuff[SPI_RAW_RX_TX_BUFFER_SIZE];  // temp



	//Define the raw SPI buffer for incoming traffic. This has a margin to allow for slippage from the MMS.
//	static CommandAndResponse::BYTE									g_SPI_MMS_To_UI_Packet_Byte_Array[SPI_RAW_RX_TX_BUFFER_SIZE]; // 12 byte margin  
	 // CommandAndResponse::SPI_A_Outer_struct_Ascii_type		g_SPI_UI_To_MMS_Packet; //The outgoing Command UI->MMS TX packet.
//	static CommandAndResponse::SPI_A_Outer_struct_Ascii_type		g_SPI_UI_To_MMS_RetryCommandPacket; //The outgoing Retry UI->MMS TX packet.
	//static CommandAndResponse::SPI_A_Outer_struct_Ascii_type *  	g_SPI_MMS_To_UI_Packet; //The incoming MMS->UI SPI packet.  Outer frame
    
    CommandAndResponse::CFastCRC32 m_CFastCRC; //!< The fast CRC object
	
    //CommandAndResponse::Raw_UI_To_MMS_Data_struct_type m_atCANBinDataFrameBufferUI[MAX_NUM_OF_CAN_PACKETS]; //!< The CAN Packet buffer array 
	//CommandAndResponse::Raw_UI_To_MMS_Data_struct_type g_spiTx_innerFrame[MAX_NUM_OF_CAN_PACKETS]; // inner Tx spi !< The CAN Packet buffer array 

	static unsigned char g_add_cmd_ptr = NULL__;
    enum { BEGIN_TX = 0 };
   //const Raw_UI_To_MMS_Data_struct_type* paCANBinMessage; //The actual CAN Binary message buffer array,
		// to be encoded to an SPI frame. Refers t the fact that it is binary, as oppossed to ascii encoded HEX.
        CommandAndResponse::BYTE g_NumOf_Tx_cmd = BEGIN_TX; //The number of entrys within the SPI frame that is populated with CAN messages.
    
    
void spi_Rx(C_Linux_SPI *spi_dev)   
{        
    int ret = 0;
    uint8_t len = 0;
    printf("\t g_SPI_Rx_outerFrame   BEGIN \n ");
    // ret = spi_dev->read(&g_SPI_Rx_outerFrame.outerRx_buff, sizeof(g_SPI_Rx_outerFrame.outerRx_buff));
    /* Outer frame */
    ret = spi_dev->read(&g_SPI_Rx_outerFrame.outerBuff, sizeof(SPI_RAW_RX_TX_BUFFER_SIZE));
    printf("\t sle Rx outer frame len == %d;  \n\t g_SPI_Rx_outerFrame.outer_buff[] == %s\n", ret, g_SPI_Rx_outerFrame.outerBuff);
    printf("\t g_SPI_Rx_outerFrame[] = \n ");
    for (long unsigned int i = 0; i < sizeof(g_SPI_Rx_outerFrame.outerBuff); ++i)  {        
        if (g_SPI_Rx_outerFrame.outerBuff[i] == '[')   {    printf("\n");   }
        printf("%c",g_SPI_Rx_outerFrame.outerBuff[i]);
    }
   

    /* inner frame */
    printf("\n\t g_SPI_Rx_InnerFrame   BEGIN");
    bool bRxSPI_2GUI_ok =  CommandAndResponse::SPI_Decode_MMS_To_UI_Packet_fn(g_SPI_Rx_outerFrame.spi_OuterFrame, g_SPI_Rx_InnerFrame.spi_InnerFrame, len);
    if (bRxSPI_2GUI_ok == EXIT_FAILURE)   {
        printf("\t MMS to GUI. SPI_Decode_MMS_To_UI_Packet_fn FAILED ! = \n ");
    }
    for (long unsigned int i = 0; i < sizeof(g_SPI_Rx_InnerFrame.InnerBuff); ++i)  {        
        if (g_SPI_Rx_InnerFrame.InnerBuff[i] == 0x4D)   {    printf("\n");   }
        printf("%X, ", g_SPI_Rx_InnerFrame.InnerBuff[i]);
    }
    printf("\n\t sle inner frame Rx  len == %d; \n\t g_SPI_Rx_InnerFrame.InnerBuff[] == %s", len, g_SPI_Rx_InnerFrame.InnerBuff);
    printf("\n\t g_SPI_Rx_InnerFrame   DONE \n ");
}


void spi_Tx(C_Linux_SPI *spi_dev)   
{        
    int ret = 0;
 
    printf("\n\t g_SPI_Tx_outerFrame   BEGIN");
    // while (1)
    // {
    //     /* temp test */
    //     ret = spi_dev->write_SPI(&g_SPI_Tx_outerFrame.outerBuff, sizeof(g_SPI_Tx_outerFrame.outerBuff));
    //     usleep(3000);
        
    // }
    
    ret = spi_dev->write_SPI(&g_SPI_Tx_outerFrame.outerBuff, sizeof(g_SPI_Tx_outerFrame.outerBuff));

    printf("\t sle sends Tx buffer  %s;   ret == %d\n", g_SPI_Tx_outerFrame.outerBuff, ret);
    for (long unsigned int i = 0; i < sizeof(g_SPI_Tx_outerFrame.outerBuff); ++i)  {
        if (g_SPI_Tx_outerFrame.outerBuff[i] == '{')   {    
            printf("\n");   
        }
        printf("%c", g_SPI_Tx_outerFrame.outerBuff[i]);
    }
    printf("\n\tspi_Tx printed");
}

void log_print(int ret)    {  
    uint8_t len = 0;
    
    printf("\t sle sends Tx buffer  %s;   ret == %d\n", g_SPI_Tx_outerFrame.outerBuff, ret);
    for (long unsigned int i = 0; i < sizeof(g_SPI_Tx_outerFrame.outerBuff); ++i)  {
        if (g_SPI_Tx_outerFrame.outerBuff[i] == '{')   {    
            printf("\n");   
        }
        printf("%c ", g_SPI_Tx_outerFrame.outerBuff[i]);
    }
    printf("\n\tspi_Tx printed\n");

    printf("\n\t outer Rx buffer  => %s;   ret == %d\n", g_SPI_Rx_outerFrame.outerBuff, ret);
    for (long unsigned int i = 0; i < sizeof(g_SPI_Rx_outerFrame.outerBuff); ++i)  {
        if (g_SPI_Rx_outerFrame.outerBuff[i] == '[')   {    
            printf("\n");   
        }
        printf("%x ", g_SPI_Rx_outerFrame.outerBuff[i]);
    }
    printf("\n\t outer spi_Rx printed\n");




    //  for now decode Rx outer to inner
        /* inner frame */
    printf("\n\t g_SPI_Rx_InnerFrame decoded from outer  BEGIN");
    bool bRxSPI_2GUI_ok =  CommandAndResponse::SPI_Decode_MMS_To_UI_Packet_fn(g_SPI_Rx_outerFrame.spi_OuterFrame, g_SPI_Rx_InnerFrame.spi_InnerFrame, len);
    if (bRxSPI_2GUI_ok == EXIT_FAILURE)   {
        printf("\t MMS to GUI. SPI_Decode_MMS_To_UI_Packet_fn FAILED ! = \n ");
    }
    for (long unsigned int i = 0; i < sizeof(g_SPI_Rx_InnerFrame.InnerBuff); ++i)  {        
        if (g_SPI_Rx_InnerFrame.InnerBuff[i] == 0x4D)   {    printf("\n");   }
        printf("%X ", g_SPI_Rx_InnerFrame.InnerBuff[i]);
    }
    printf("\n\t sle inner frame Rx  len == %d; \n\t g_SPI_Rx_InnerFrame.InnerBuff[] == %s", len, g_SPI_Rx_InnerFrame.InnerBuff);
    printf("\n\t g_SPI_Rx_InnerFrame decoded from outer   DONE \n ");

    printf("\n\t g_SPI_Rx_InnerFrame in char  BEGIN");
     bRxSPI_2GUI_ok =  CommandAndResponse::SPI_Decode_MMS_To_UI_Packet_fn(g_SPI_Rx_outerFrame.spi_OuterFrame, g_SPI_Rx_InnerFrame.spi_InnerFrame, len);
    if (bRxSPI_2GUI_ok == FALSE)   {
        printf("\t MMS to GUI. SPI_Decode_MMS_To_UI_Packet_fn FAILED ! = \n ");
    }
    for (long unsigned int i = 0; i < sizeof(g_SPI_Rx_InnerFrame.InnerBuff); ++i)  {        
        if (g_SPI_Rx_InnerFrame.InnerBuff[i] == 0x4D)   {    printf("\n");   }
        printf("%c ", g_SPI_Rx_InnerFrame.InnerBuff[i]);
    }
    printf("\n\t sle inner frame Rx  len == %d; \n\t g_SPI_Rx_InnerFrame.InnerBuff[] == %s", len, g_SPI_Rx_InnerFrame.InnerBuff);
    printf("\n\t g_SPI_Rx_InnerFrame in char decoded from outer  DONE \n ");

}
static uint16_t delay;
void SPI_synch_run_Rx_Tx(C_Linux_SPI *spi_dev)     /* synch of  read & write */
{
    enum { SPI_ENABLE = 0, SPI_DISABLE = 1 };
    struct spi_ioc_transfer spi_config_msg[TOTAL_NUMBER_OF_SPI_DEVICES];
    int ret;

    spi_config_msg[0].tx_buf        = (uintptr_t)g_SPI_Tx_outerFrame.outerBuff;
    spi_config_msg[0].rx_buf        = (uintptr_t)g_SPI_Rx_outerFrame.outerBuff;
    spi_config_msg[0].len           = SPI_RAW_RX_TX_BUFFER_SIZE;
    spi_config_msg[0].cs_change     = SPI_DISABLE;
    spi_config_msg[0].delay_usecs   = delay;
    //spi_config_msg[0].speed_hz      = 4000000;
    

/*
    spi_config_msg[1].bits_per_word = 8;
    spi_config_msg[1].rx_buf        = (uintptr_t)buf;
    spi_config_msg[1].tx_buf        = (uintptr_t)NULL;
    spi_config_msg[1].len           = 1056;
    spi_config_msg[1].cs_change     = 0;
*/



    ret = spi_dev->send_tr(spi_config_msg, TOTAL_NUMBER_OF_SPI_DEVICES);


    ret = ret;
    log_print(ret);

}


void init_spi_dev(C_Linux_SPI *spi_dev)
{

    #ifdef DART_DEV_KIT 
        if( spi_dev->dev_open("/dev/spidev1.0") != 0 )      // dev kit
    #else  
        if( spi_dev->dev_open("/dev/spidev0.2") != 0 )  // sle7000
    #endif
    {
        printf("Error: dev_open %s\n", spi_dev->strerror(spi_dev->get_errno()));  /* Error: dev_open device is not open */
        exit(-1);
    }


    if( spi_dev->set_mode(SPI_MODE_1) != 0 )
    {
        printf("Error: %s\n", spi_dev->strerror(spi_dev->get_errno()));
        exit(-1);
    }


    if( spi_dev->set_bits_per_word(8) != 0 )
    {
        printf("Error: %s\n", spi_dev->strerror(spi_dev->get_errno()));
        exit(-1);
    }


    if( spi_dev->set_max_speed_hz(SPI_SPEED_HZ) != 0 )       // 4 MHz ==> 200nsec
    {
        printf("Error: %s\n", spi_dev->strerror(spi_dev->get_errno()));
        exit(-1);
    }

printf("\n init spi dev successfully done ");
    
}

	/*!********************************************************************
	* \brief Prepares a CAN packet containing the shut down request to PSU
	***********************************************************************/
	//void CPowerDownPanel::RequestPSUToShutDown() const
    void RequestPSUToShutDown(void)
	{	        
        g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Dest_Subsystem_ID_u8 = SPI_SUBS_ID_MONITOR;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].CAN_ID_u8 = PSU_COMMAND;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].CAN_DLC_u8 = 4U;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Payload_u8[0] = PC_POWER_DOWN_EXECUTE;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Payload_u8[1] = 0U;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Payload_u8[2] = 1U;
        m_byTrackingByteNumberMonitor++;
        g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Payload_u8[3] = m_byTrackingByteNumberMonitor;
        
        ++g_add_cmd_ptr;
	}

    /*!********************************************************************
    * \brief Initialise the the Backup Alarm Sounder on the MMS.
    * \details When the MMS turns on, it sounds the alarm controller at full
    * volume as a Power On, Pre Test. We need to tell the Alarm Sounder to
    * stop.  192.168.32.1
    * Also set the Volume to Maximum.
    ***********************************************************************/
    void Test_Tx_commands()
    {
        g_add_cmd_ptr = 0;
        //Update the volume to the MMS Alarm Controller, this should always be at maximum volume
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Dest_Subsystem_ID_u8 = (CommandAndResponse::BYTE)  SPI_SUBS_ID_MMS_LOCAL; // 0x4C
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].CAN_ID_u8 = (CommandAndResponse::BYTE) MMS_CAN_ID_ALARM;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].CAN_DLC_u8 = (CommandAndResponse::BYTE) 3U;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Payload_u8[0] = (CommandAndResponse::BYTE)  MMS_LOCAL_CMD_SET_ALARM;  // MMS_LOCAL_CMD_SET_ALARM;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Payload_u8[1] = (CommandAndResponse::BYTE) 1U;        // AlarmVolume max = 7U;;
        m_byTrackingByteNumberMMS = 0;                                                                  // command counter++ for SPI_SUBS_ID_MMS_LOCAL
        g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Payload_u8[2] = (CommandAndResponse::BYTE) m_byTrackingByteNumberMMS;  // command counter to be sent over Tx
        		
        g_add_cmd_ptr = 1;  // command counter to be used to add msg on the Tx buffer 


        //Construct High Priority Alarm Request
        g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Dest_Subsystem_ID_u8 = (CommandAndResponse::BYTE) SPI_SUBS_ID_MMS_LOCAL;  // 0x4C
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].CAN_ID_u8 = (CommandAndResponse::BYTE) MMS_CAN_ID_ALARM;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].CAN_DLC_u8 = (CommandAndResponse::BYTE) 3U;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Payload_u8[0] = (CommandAndResponse::BYTE) MMS_LOCAL_CMD_SET_ALARM;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Payload_u8[1] = (CommandAndResponse::BYTE) MMS_CE_START_HIGH_PRIORITY_ALARM;
        m_byTrackingByteNumberMMS++;                                                                  // command counter++ for SPI_SUBS_ID_MMS_LOCAL
        g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Payload_u8[2] = (CommandAndResponse::BYTE) m_byTrackingByteNumberMMS;  // command counter to be sent over Tx

        g_add_cmd_ptr = 2;

        //Update the volume to the MMS Alarm Controller, this should always be at maximum volume
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Dest_Subsystem_ID_u8 = (CommandAndResponse::BYTE) SPI_SUBS_ID_MMS_LOCAL;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].CAN_ID_u8 = (CommandAndResponse::BYTE) MMS_CAN_ID_ALARM;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].CAN_DLC_u8 = (CommandAndResponse::BYTE)  3U;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Payload_u8[0] = (CommandAndResponse::BYTE)  MMS_LOCAL_CMD_SET_ALARM;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Payload_u8[1] = (CommandAndResponse::BYTE)  1U;        // AlarmVolume max = (CommandAndResponse::BYTE) 7U;		
        m_byTrackingByteNumberMMS++;                                                                  // command counter++ for SPI_SUBS_ID_MMS_LOCAL for SPI_SUBS_ID_MMS_LOCAL
        g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Payload_u8[2] = (CommandAndResponse::BYTE) m_byTrackingByteNumberMMS;  // command counter to be sent over Tx

        g_add_cmd_ptr = 3;        

        //Construct High Priority Alarm Request
        g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Dest_Subsystem_ID_u8 = (CommandAndResponse::BYTE) SPI_SUBS_ID_MMS_LOCAL;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].CAN_ID_u8 = (CommandAndResponse::BYTE) MMS_CAN_ID_ALARM;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].CAN_DLC_u8 = (CommandAndResponse::BYTE)  3U;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Payload_u8[0] = (CommandAndResponse::BYTE)  MMS_LOCAL_CMD_SET_ALARM;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Payload_u8[1] = (CommandAndResponse::BYTE) MMS_CE_START_HIGH_PRIORITY_ALARM;
        m_byTrackingByteNumberMMS++;                                                                  // command counter++ for SPI_SUBS_ID_MMS_LOCAL
        g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Payload_u8[2] = (CommandAndResponse::BYTE) m_byTrackingByteNumberMMS;  // command counter to be sent over Tx

        g_add_cmd_ptr = 4;

        //Update the volume to the MMS Alarm Controller, this should always be at maximum volume
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Dest_Subsystem_ID_u8 = (CommandAndResponse::BYTE) SPI_SUBS_ID_MMS_LOCAL;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].CAN_ID_u8 = (CommandAndResponse::BYTE) MMS_CAN_ID_ALARM;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].CAN_DLC_u8 = (CommandAndResponse::BYTE)  3U;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Payload_u8[0] = (CommandAndResponse::BYTE)  MMS_LOCAL_CMD_SET_ALARM;  // MMS_LOCAL_CMD_SET_ALARM;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Payload_u8[1] = (CommandAndResponse::BYTE)  6U;        // AlarmVolume max = (CommandAndResponse::BYTE) 7U;;
        m_byTrackingByteNumberMMS++;                                                                  // command counter++ for SPI_SUBS_ID_MMS_LOCAL
        g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Payload_u8[2] = (CommandAndResponse::BYTE)  m_byTrackingByteNumberMMS;  // command counter to be sent over Tx
        		
        g_add_cmd_ptr = 5;  // command counter to be used to add msg on the Tx buffer 

        //Tell the alarm sounder to stop
        g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Dest_Subsystem_ID_u8 = (CommandAndResponse::BYTE) SPI_SUBS_ID_MMS_LOCAL;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].CAN_ID_u8 = (CommandAndResponse::BYTE) MMS_CAN_ID_ALARM;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].CAN_DLC_u8 = (CommandAndResponse::BYTE) 3U;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Payload_u8[0] = (CommandAndResponse::BYTE)  MMS_LOCAL_CMD_SET_ALARM;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Payload_u8[1] = (CommandAndResponse::BYTE)  MMS_CE_STOP_ALARM_IMMEDIATELY;	
        m_byTrackingByteNumberMMS++;                                                                  // command counter++ for SPI_SUBS_ID_MMS_LOCAL
        g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Payload_u8[2] = (CommandAndResponse::BYTE) m_byTrackingByteNumberMMS;  // command counter to be sent over Tx

        g_add_cmd_ptr = 6;
        //Construct High Priority Alarm Request
        g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Dest_Subsystem_ID_u8 = (CommandAndResponse::BYTE) SPI_SUBS_ID_MMS_LOCAL;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].CAN_ID_u8 = (CommandAndResponse::BYTE) MMS_CAN_ID_ALARM;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].CAN_DLC_u8 = (CommandAndResponse::BYTE)  3U;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Payload_u8[0] = (CommandAndResponse::BYTE)  MMS_LOCAL_CMD_SET_ALARM;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Payload_u8[1] = (CommandAndResponse::BYTE) MMS_CE_START_HIGH_PRIORITY_ALARM;
        m_byTrackingByteNumberMMS++;                                                                  // command counter++ for SPI_SUBS_ID_MMS_LOCAL
        g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Payload_u8[2] = (CommandAndResponse::BYTE) m_byTrackingByteNumberMMS;  // command counter to be sent over Tx

        g_add_cmd_ptr++;        
                //Construct High Priority Alarm Request
        g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Dest_Subsystem_ID_u8 = (CommandAndResponse::BYTE) SPI_SUBS_ID_MMS_LOCAL;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].CAN_ID_u8 = (CommandAndResponse::BYTE) MMS_CAN_ID_ALARM;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].CAN_DLC_u8 = (CommandAndResponse::BYTE)  3U;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Payload_u8[0] = (CommandAndResponse::BYTE)  MMS_LOCAL_CMD_SET_ALARM;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Payload_u8[1] = (CommandAndResponse::BYTE) MMS_CE_START_HIGH_PRIORITY_ALARM;
        m_byTrackingByteNumberMMS++;                                                                  // command counter++ for SPI_SUBS_ID_MMS_LOCAL
        g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Payload_u8[2] = (CommandAndResponse::BYTE) m_byTrackingByteNumberMMS;  // command counter to be sent over Tx

        g_add_cmd_ptr++;        
                //Construct High Priority Alarm Request
        g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Dest_Subsystem_ID_u8 = (CommandAndResponse::BYTE) SPI_SUBS_ID_MMS_LOCAL;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].CAN_ID_u8 = (CommandAndResponse::BYTE) MMS_CAN_ID_ALARM;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].CAN_DLC_u8 = (CommandAndResponse::BYTE)  3U;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Payload_u8[0] = (CommandAndResponse::BYTE)  MMS_LOCAL_CMD_SET_ALARM;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Payload_u8[1] = (CommandAndResponse::BYTE) MMS_CE_START_HIGH_PRIORITY_ALARM;
        m_byTrackingByteNumberMMS++;                                                                  // command counter++ for SPI_SUBS_ID_MMS_LOCAL
        g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Payload_u8[2] = (CommandAndResponse::BYTE) m_byTrackingByteNumberMMS;  // command counter to be sent over Tx

        g_add_cmd_ptr++;        
                //Construct High Priority Alarm Request
        g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Dest_Subsystem_ID_u8 = (CommandAndResponse::BYTE) SPI_SUBS_ID_MMS_LOCAL;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].CAN_ID_u8 = (CommandAndResponse::BYTE) MMS_CAN_ID_ALARM;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].CAN_DLC_u8 = (CommandAndResponse::BYTE)  3U;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Payload_u8[0] = (CommandAndResponse::BYTE)  MMS_LOCAL_CMD_SET_ALARM;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Payload_u8[1] = (CommandAndResponse::BYTE) MMS_CE_START_HIGH_PRIORITY_ALARM;
        m_byTrackingByteNumberMMS++;                                                                  // command counter++ for SPI_SUBS_ID_MMS_LOCAL
        g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Payload_u8[2] = (CommandAndResponse::BYTE) m_byTrackingByteNumberMMS;  // command counter to be sent over Tx

        g_add_cmd_ptr++;        
                //Construct High Priority Alarm Request
        g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Dest_Subsystem_ID_u8 = (CommandAndResponse::BYTE) SPI_SUBS_ID_MMS_LOCAL;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].CAN_ID_u8 = (CommandAndResponse::BYTE) MMS_CAN_ID_ALARM;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].CAN_DLC_u8 = (CommandAndResponse::BYTE)  3U;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Payload_u8[0] = (CommandAndResponse::BYTE)  MMS_LOCAL_CMD_SET_ALARM;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Payload_u8[1] = (CommandAndResponse::BYTE) MMS_CE_START_HIGH_PRIORITY_ALARM;
        m_byTrackingByteNumberMMS++;                                                                  // command counter++ for SPI_SUBS_ID_MMS_LOCAL
        g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Payload_u8[2] = (CommandAndResponse::BYTE) m_byTrackingByteNumberMMS;  // command counter to be sent over Tx

        g_add_cmd_ptr++;        
                //Construct High Priority Alarm Request
        g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Dest_Subsystem_ID_u8 = (CommandAndResponse::BYTE) SPI_SUBS_ID_MMS_LOCAL;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].CAN_ID_u8 = (CommandAndResponse::BYTE) MMS_CAN_ID_ALARM;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].CAN_DLC_u8 = (CommandAndResponse::BYTE)  3U;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Payload_u8[0] = (CommandAndResponse::BYTE)  MMS_LOCAL_CMD_SET_ALARM;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Payload_u8[1] = (CommandAndResponse::BYTE) MMS_CE_START_HIGH_PRIORITY_ALARM;
        m_byTrackingByteNumberMMS++;                                                                  // command counter++ for SPI_SUBS_ID_MMS_LOCAL
        g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Payload_u8[2] = (CommandAndResponse::BYTE) m_byTrackingByteNumberMMS;  // command counter to be sent over Tx

        g_add_cmd_ptr++;        
                //Construct High Priority Alarm Request
        g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Dest_Subsystem_ID_u8 = (CommandAndResponse::BYTE) SPI_SUBS_ID_MMS_LOCAL;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].CAN_ID_u8 = (CommandAndResponse::BYTE) MMS_CAN_ID_ALARM;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].CAN_DLC_u8 = (CommandAndResponse::BYTE)  3U;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Payload_u8[0] = (CommandAndResponse::BYTE)  MMS_LOCAL_CMD_SET_ALARM;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Payload_u8[1] = (CommandAndResponse::BYTE) MMS_CE_START_HIGH_PRIORITY_ALARM;
        m_byTrackingByteNumberMMS++;                                                                  // command counter++ for SPI_SUBS_ID_MMS_LOCAL
        g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Payload_u8[2] = (CommandAndResponse::BYTE) m_byTrackingByteNumberMMS;  // command counter to be sent over Tx

        g_add_cmd_ptr++;        
                //Construct High Priority Alarm Request
        g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Dest_Subsystem_ID_u8 = (CommandAndResponse::BYTE) SPI_SUBS_ID_MMS_LOCAL;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].CAN_ID_u8 = (CommandAndResponse::BYTE) MMS_CAN_ID_ALARM;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].CAN_DLC_u8 = (CommandAndResponse::BYTE)  3U;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Payload_u8[0] = (CommandAndResponse::BYTE)  MMS_LOCAL_CMD_SET_ALARM;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Payload_u8[1] = (CommandAndResponse::BYTE) MMS_CE_START_HIGH_PRIORITY_ALARM;
        m_byTrackingByteNumberMMS++;                                                                  // command counter++ for SPI_SUBS_ID_MMS_LOCAL
        g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Payload_u8[2] = (CommandAndResponse::BYTE) m_byTrackingByteNumberMMS;  // command counter to be sent over Tx

        g_add_cmd_ptr++;        
                //Construct High Priority Alarm Request
        g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Dest_Subsystem_ID_u8 = (CommandAndResponse::BYTE) SPI_SUBS_ID_MMS_LOCAL;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].CAN_ID_u8 = (CommandAndResponse::BYTE) MMS_CAN_ID_ALARM;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].CAN_DLC_u8 = (CommandAndResponse::BYTE)  3U;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Payload_u8[0] = (CommandAndResponse::BYTE)  MMS_LOCAL_CMD_SET_ALARM;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Payload_u8[1] = (CommandAndResponse::BYTE) MMS_CE_START_HIGH_PRIORITY_ALARM;
        m_byTrackingByteNumberMMS++;                                                                  // command counter++ for SPI_SUBS_ID_MMS_LOCAL
        g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Payload_u8[2] = (CommandAndResponse::BYTE) m_byTrackingByteNumberMMS;  // command counter to be sent over Tx

        g_add_cmd_ptr++;        
                //Construct High Priority Alarm Request
        g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Dest_Subsystem_ID_u8 = (CommandAndResponse::BYTE) SPI_SUBS_ID_MMS_LOCAL;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].CAN_ID_u8 = (CommandAndResponse::BYTE) MMS_CAN_ID_ALARM;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].CAN_DLC_u8 = (CommandAndResponse::BYTE)  3U;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Payload_u8[0] = (CommandAndResponse::BYTE)  MMS_LOCAL_CMD_SET_ALARM;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Payload_u8[1] = (CommandAndResponse::BYTE) MMS_CE_START_HIGH_PRIORITY_ALARM;
        m_byTrackingByteNumberMMS++;                                                                  // command counter++ for SPI_SUBS_ID_MMS_LOCAL
        g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Payload_u8[2] = (CommandAndResponse::BYTE) m_byTrackingByteNumberMMS;  // command counter to be sent over Tx

        g_add_cmd_ptr++;        
                //Construct High Priority Alarm Request
        g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Dest_Subsystem_ID_u8 = (CommandAndResponse::BYTE) SPI_SUBS_ID_MMS_LOCAL;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].CAN_ID_u8 = (CommandAndResponse::BYTE) MMS_CAN_ID_ALARM;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].CAN_DLC_u8 = (CommandAndResponse::BYTE)  3U;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Payload_u8[0] = (CommandAndResponse::BYTE)  MMS_LOCAL_CMD_SET_ALARM;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Payload_u8[1] = (CommandAndResponse::BYTE) MMS_CE_START_HIGH_PRIORITY_ALARM;
        m_byTrackingByteNumberMMS++;                                                                  // command counter++ for SPI_SUBS_ID_MMS_LOCAL
        g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Payload_u8[2] = (CommandAndResponse::BYTE) m_byTrackingByteNumberMMS;  // command counter to be sent over Tx

        g_add_cmd_ptr++;        
        
    
        g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Dest_Subsystem_ID_u8 = (CommandAndResponse::BYTE) SPI_SUBS_ID_MMS_LOCAL;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].CAN_ID_u8 = (CommandAndResponse::BYTE) MMS_CAN_ID;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].CAN_DLC_u8 = (CommandAndResponse::BYTE) 1U;
		g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Payload_u8[0] = (CommandAndResponse::BYTE) MMS_GET_SERIAL_NUMBER;
        m_byTrackingByteNumberMMS = 1;                                                                  // command counter++ for SPI_SUBS_ID_MMS_LOCAL
        g_SPI_Tx_InnerFrame.spi_InnerFrame[g_add_cmd_ptr].Payload_u8[1] = (CommandAndResponse::BYTE) m_byTrackingByteNumberMMS;  // command counter to be sent over Tx

        //g_add_cmd_ptr++;

    }

    
    /*!********************************************************************
    * \brief Generate the CRC for the constructed packet.
    * \details Generates a CRC for the packet, and places the CRC is the
    * dedicated slot within the packet
    * \param g_SPI_UI_To_MMS_Packet The Packet to be CRC'ed.
    ***********************************************************************/
    void GenerateCRCandAddToOutboundPacket(CommandAndResponse::SPI_A_Outer_struct_Ascii_type*  g_SPI_UI_To_MMS_Packet) 
    {
        //Compute CRC
        //Fails 5-2-7, so wrap with exclusion.
        //lint -e927
        CommandAndResponse::DWORD* pdwRawPacket = (CommandAndResponse::DWORD*)(&g_SPI_UI_To_MMS_Packet->O_Start_Delimiter);
        //lint +e927
        CommandAndResponse::DWORD dwSCM_CRC_u32 = m_CFastCRC.Crc32FastBlock_fn(0xFFFFFFFFUL, (SPI_RAW_CRC_POSITION / 4U), pdwRawPacket);

        CommandAndResponse::WORD wHighWord = (CommandAndResponse::WORD)((dwSCM_CRC_u32) >> 16U);
        CommandAndResponse::WORD wLowWord = (CommandAndResponse::WORD)dwSCM_CRC_u32;

        //Place CRC in outward packet
        (void)CommandAndResponse::SPI_Bin_u16_To_ASCII_Quad_fn(wHighWord, &g_SPI_UI_To_MMS_Packet->O_CRC[0]); 
        (void)CommandAndResponse::SPI_Bin_u16_To_ASCII_Quad_fn(wLowWord, &g_SPI_UI_To_MMS_Packet->O_CRC[4]); 
    }
    


// The function to be executed by  thread1 -------------------------------------------------------------------------------
/*-----------------------------------------------------------------------------------------------------------------------*/
__attribute__((noinline)) int exec()
{
   // uint16_t                max_len;

    unsigned long int Length_SPI_Tx_InnerFrameBuff = 0;
    unsigned long int i = 0;
    
    C_Linux_SPI             *ptr_SPIdev;
    FILE *fd = freopen("logfile_SPI.txt","w",stdout);


    if (fd == NULL) {
        printf("\n\tfreopen Failed!\n");
    }
    //unsigned long int i = 0;
    Length_SPI_Tx_InnerFrameBuff = sizeof(g_SPI_Tx_InnerFrame.InnerBuff);

    printf("\n\tg_spiTx_innerFrame[] = \n");
     for (i = 0; i < Length_SPI_Tx_InnerFrameBuff - 1; ++i)  {
        if (g_SPI_Tx_InnerFrame.InnerBuff[i] == SPI_SUBS_ID_MMS_LOCAL) { printf("\n");   }
        printf("%x", g_SPI_Tx_InnerFrame.InnerBuff[i]);
    }
    printf("\nhow many bytes g_SPI_Tx_InnerFrame printed %ld", i);
   

    // if (fd == nullptr)   {
    //     printf("\n\t Error = Logfile_SPI.txt was not created !!! \n");
    // }
    printf("\nL1");
    ptr_SPIdev = getSPI_dev1();
    printf("\nL2");
     // use arg1_, arg2_....sponse::Ra



    Test_Tx_commands();
    printf("\nL3");
    printf("\n\tg_spiTx_innerFrame[] = \n");
     for (i = 0; i < Length_SPI_Tx_InnerFrameBuff - 1; ++i)  {
        if (g_SPI_Tx_InnerFrame.InnerBuff[i] == SPI_SUBS_ID_MMS_LOCAL) { printf("\n");   }
        printf("%x", g_SPI_Tx_InnerFrame.InnerBuff[i]);
    }
    printf("\nhow many bytes g_SPI_Tx_InnerFrame printed %ld", i);
   
   // RequestPSUToShutDown();
    printf("\nL4");


    printf("\n\tg_spiTx_innerFrame[] = \n");
    printf("%s", g_SPI_Tx_InnerFrame.InnerBuff);
    
    Length_SPI_Tx_InnerFrameBuff = sizeof(g_SPI_Tx_InnerFrame.InnerBuff);
    printf("\n\tsizeof(g_SPI_Tx_InnerFrame.InnerBuff) = %ld; \n", Length_SPI_Tx_InnerFrameBuff);
    Length_SPI_Tx_InnerFrameBuff = sizeof(g_SPI_Tx_InnerFrame.spi_InnerFrame);
    printf("sizeof(g_SPI_Tx_InnerFrame.InnerFrame) = %ld; \n", Length_SPI_Tx_InnerFrameBuff);




    g_NumOf_Tx_cmd = g_add_cmd_ptr;
    printf("\n How many cmd added ?  g_add_cmd_ptr = %d;\nL5", g_add_cmd_ptr);
    /* log the inner Tx frame */
    // pthread_mutex_lock(&mutex);
    // unsigned char line = 0;
    // for (u_int16_t i = 0; i < g_NumOf_Tx_cmd * sizeof(CommandAndResponse::Raw_UI_To_MMS_Data_struct_type); ++i)   {
    //    //printf("\t g_spiTx_innerFrame_buff[%d] = %d; ", i, g_SPI_Tx_InnerFrame.spi_InnerFrame[i]);
    //     ++line;
    //     if (line > sizeof(CommandAndResponse::Raw_UI_To_MMS_Data_struct_type))   {     
    //         line = 0;
    //         printf("\n");
    //     }
    // }   
    // pthread_mutex_unlock(&mutex);


                
    //Populate ASCII Hex Buffer for Tx.
    //unsigned char byNumOfPopulatedEntrys = pCallingClass->GetuiNumOfPopulatedEntrysUI();  // max = 77 commands / messages

    /* convert inner frame into outer Tx frame g_SPI_UI_To_MMS_Packet in order to Tx over spi , CAN*/
    (void) CommandAndResponse::SPI_Construct_UI_To_MMS_Packet(g_SPI_Tx_InnerFrame.spi_InnerFrame, g_NumOf_Tx_cmd, g_SPI_Tx_outerFrame.spi_OuterFrame);

    //Compute CRC
    GenerateCRCandAddToOutboundPacket(&g_SPI_Tx_outerFrame.spi_OuterFrame);
  
    printf("\n thread1 spi_Tx begin \n");

   spi_Tx(ptr_SPIdev);      /* run the spi dart Tx*/

   // for (int i = 0; i < 3; ++i)
    // while (1)
    // {
    //     spi_Tx(ptr_SPIdev);      /* run the spi dart Tx*/
    //     //sleep(3);  // usec   
    //     usleep(300);
    //     spi_Rx(ptr_SPIdev);     
    // }
       
    // printf("\n thread1 spi_Tx end\n");


printf("\n thread1 SPI_run_Rx_Tx begin \n");
    while (1)
   // for (int i = 0; i < 6; ++i)
    {
        /* synch */
        SPI_synch_run_Rx_Tx(ptr_SPIdev);  
       // sleep(1);  
       usleep(16);  // 300    mms has 16.666 msec
    }
printf("\n thread1 SPI_synch_run_Rx_Tx end\n");



    /* read and write SPI with g_SPI_Rx_outerFrame.outerRx_buff[] and Tx_buff */
    

    /* store / update data in the shared memory */
    // pthread_mutex_lock(&mutex);
    // for (uint16_t i = 0; i < SPI_RAW_RX_TX_BUFFER_SIZE; ++i)   {
    //     //ptr_sle7k_dev_map->m_thread1[i] = g_SPI_Rx_outerFrame.outerRx_buff[i];          // (unsigned char) i + 'A';
    //    printf("\tThread1 runing. g_SPI_Rx_outerFrame.g_SPI_Rx_outerFrame.outerRx_buff[%l] = %X; \n", i, ptr_sle7k_dev_map->m_thread1[i]);
    // }   
    // pthread_mutex_unlock(&mutex);


    /* decode from outer frame into the inner frame of Rx  */
/*    SPI_Decode_MMS_To_UI_Packet_fn(g_SPI_Rx_outerFrame.g_SPI_Rx_outerFrame.outerRx_buff, innerRxBuffMsg, SPI_RAW_RX_TX_BUFFER_SIZE);
    uint16_t max_len = SPI_RAW_RX_TX_BUFFER_SIZE;
    // g_SPI_MMS_To_UI_Packet = &g_SPI_Rx_outerFrame.spi_Rx_outerFrame;
    bool bSPIOK = SPI_Decode_MMS_To_UI_Packet_fn(g_SPI_Rx_outerFrame.spi_Rx_outerFrame, &g_SPI_Rx_InnerFrame.spi_Rx_InnerFrame, max_len);
*/
    /* add the new frame, innerRxBuffMsg, in the queue */
  /*  bool bSPI_innerAdded2Queue = ptrQueueSPI->addArray2queue(g_SPI_Rx_InnerFrame.InnerRx_buff, sizeof(g_SPI_Rx_InnerFrame.InnerRx_buff));        

    pthread_mutex_lock(&mutex);
    for (int i = 0; i < MAX_NUM_OF_INNER_PACKETS_SPI_RX_TX; ++i)   {
        printf("\t QueueSPI Rx_buff[%d] = %X; \n", i, ptrQueueSPI->queue[i] );
    }   
    pthread_mutex_unlock(&mutex);
   */
    

    fclose (stdout);
    return EXIT_SUCCESS;
}

