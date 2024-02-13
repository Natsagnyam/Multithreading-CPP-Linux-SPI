/*!********************************************************************
 * \file
 * \brief This file contains the functionality for system CCommsTransport
 * \details
 * (C)opyright 2017 SLE Ltd
 *
 * SVN Revision: $Revision: 2637 $
 *
 * Modification Record
 * -------------------
 *  Reference   |    Date     | By             | Description
 *  ----------: | :---------: | :------------: | :------------
 *		-   	| 13-Feb-2017 | S. Easton      | Initial implementation
 *      -       | 30-08-2018  | L. Gatyasova   | RID 5722 (FDD1 - Engineering utility) 
 * MANTIS 2266	| 06-09-2018  | L. Gatyasova   | The user needs to constantly cycle between other modes until the desired mode is working. 
 * MANTIS 2221  | 02-10-2018  | L. Gatyasova   | EtCO2 Calibration Does Not Initiate
 * MANTIS 2468  | 15-02-2019  | L. Gatyasova   | As soon as you enter engineering utility, valves are clicking at high frequency for about 20seconds
 * TFS 6479     | 12-06-2019  | L. Gatyasova   | FDD15 - SLE6000 connectivity to SLE6000C pneumatics 
 * TFS 6487		| 21-06-2019  | L. Gatyasova   | Engineering Mode (FDD15) 
 * TFS 6488     | 21-06-2019  | L. Gatyasova   | Manually Setting controller to control pneumatics (FDD15)
 * CR98 		| 02-07-2019  | L. Gatyasova   | Rename parameter variable to something more accurate
 ***********************************************************************/
//// #include "stdafx.h"
#include "EngUtility/CommsTransport.h"


#define COMMS_TRANSPORT_CALLBACK_DEBUG TRUE

#define PSI_VALUE_DOWN  1	//decrement gain of 1PSI or 2.5PSI transducer
#define PSI_VALUE_UP  0		//increment gain of 1PSI or 2.5PSI transducer

namespace SystemCommunications
{
    /*!***************************************************************************************************
    * \brief Constructor
    ******************************************************************************************************/
    CCommsTransport::CCommsTransport( void )
    {
        m_uiCANBufferIndex = 0U;
        m_byTrackingByteNumberController = 0U;
        m_byTrackingByteNumberMonitor = 0U;
        m_byTrackingByteNumberMMS = 0U;
        m_byTrackingByteNumberESMO = 0U;
        //lint -esym(1960, 5-2-12)
        memset( m_aCANBinDataBuffer, 0, sizeof( SystemCommunications::Raw_UI_To_MMS_Data_struct_type ) *MAX_NUM_OF_CAN_PACKETS );
        //lint +esym(1960, 5-2-12)
        m_ICANToData = NULL;
        //Create the Critical Section
        InitializeCriticalSection( &m_CS_Write_To_SPI );

        Controller_Update_Request_Commands[0]	=	( BYTE )CD_BLENDER_AIR_FLOW;
        Controller_Update_Request_Commands[1]	=	( BYTE )CD_BLENDER_O2_FLOW;
        Controller_Update_Request_Commands[2]	=	( BYTE )CD_FRESH_GAS_FLOW;
        Controller_Update_Request_Commands[3]	=	( BYTE )CD_AIR_INPUT_PRESSURE;
        Controller_Update_Request_Commands[4]	=	( BYTE )CD_O2_INPUT_PRESSURE;
        Controller_Update_Request_Commands[5]	=	( BYTE )CD_FRESH_GAS_PRESSURE;
        Controller_Update_Request_Commands[6]	=	( BYTE )CD_PROXIMAL_PRESSURE;
        Controller_Update_Request_Commands[7]	=	( BYTE )CD_ELAPSED_TIME_MINUTES;
        Controller_Update_Request_Commands[8]	=	( BYTE )CD_ELAPSED_TIME_DAYS;
        Controller_Update_Request_Commands[9]	=	( BYTE )CD_FRESH_GAS_RELIEF_VALVE_PRESSURE;
        Controller_Update_Request_Commands[10]	=	( BYTE )CD_FRESH_GAS_BYPASS_FLOW;
        Controller_Update_Request_Commands[11]	=	( BYTE )CD_FRESH_GAS_DUMP_BACK_PRESSURE;
        Controller_Update_Request_Commands[12]	=	( BYTE )CD_NEBULISER_FLOW;
        Controller_Update_Request_Commands[13]	=	( BYTE )CD_BLENDER_FG_FLOW_DIFF;
        Controller_Update_Request_Commands[14]	=	( BYTE )CD_NEBULISER_SELF_TEST_STATE;
        Controller_Update_Request_Commands[15]	=	( BYTE )CD_BLENDER_SELF_TEST_STATE;
        Controller_Update_Request_Commands[16]	=	( BYTE )CD_BREATHJET_SELF_TEST_STATE;
			
        ESMO_ETCO2_Update_Request_Commands[0]	=	( BYTE )ESMODefinitions::ESD_HW_VERSION_ID;
        ESMO_ETCO2_Update_Request_Commands[1]	= 	( BYTE )ESMODefinitions::ESD_FIRMWARE_ID;
        ESMO_ETCO2_Update_Request_Commands[2]	= 	( BYTE )ESMODefinitions::ESD_SPO2_MODULE_INFO;
        ESMO_ETCO2_Update_Request_Commands[3]	= 	( BYTE )ESMODefinitions::ESD_SPO2_CURRENT_CONFIG;
        ESMO_ETCO2_Update_Request_Commands[4]	= 	( BYTE )ESMODefinitions::ESD_SPO2_DIAGNOSTIC_CODES;
        ESMO_ETCO2_Update_Request_Commands[5]	= 	( BYTE )ESMODefinitions::ESD_ETCO2_SOFTWARE_VER;
        ESMO_ETCO2_Update_Request_Commands[6]	= 	( BYTE )ESMODefinitions::ESD_ETCO2_SW_RELEASE_DATE;
        ESMO_ETCO2_Update_Request_Commands[7]	= 	( BYTE )ESMODefinitions::ESD_ETCO2_HARDWARE_VER;
        ESMO_ETCO2_Update_Request_Commands[8]	= 	( BYTE )ESMODefinitions::ESD_ETCO2_DEVICE_SN;
        ESMO_ETCO2_Update_Request_Commands[9]	= 	( BYTE )ESMODefinitions::ESD_ETCO2_MODES;
        ESMO_ETCO2_Update_Request_Commands[10]	= 	( BYTE )ESMODefinitions::ESD_ETCO2_MEASURE_MODE;
        ESMO_ETCO2_Update_Request_Commands[11]	= 	( BYTE )ESMODefinitions::ESD_ETCO2_CALIBRATION_DATE;
        ESMO_ETCO2_Update_Request_Commands[12]	= 	( BYTE )ESMODefinitions::ESD_ETCO2_CALIB_SERVICE_HOURS;

        Monitor_Update_Request_Commands[0]		=	( BYTE )MD_O2_CALIBRATION_STATUS;
        Monitor_Update_Request_Commands[1]		=	( BYTE )MD_FLOW_ERROR_GET;
        Monitor_Update_Request_Commands[2]		=	( BYTE )MD_OXYGEN_100_PERCENT_CALIB_VALUE;

        //Ventilator control initialisation commands
        One_Byte_VentCont_Init_Commands[0]		=	( BYTE ) CC_SET_FIO2;
        One_Byte_VentCont_Init_Commands[1]		=	( BYTE ) CC_SET_BPM;
        One_Byte_VentCont_Init_Commands[2]		=	( BYTE ) CC_SET_BASE_JET_PRESSURE;
        One_Byte_VentCont_Init_Commands[3]		=	( BYTE ) CC_SET_PIP_JET_PRESSURE;
        One_Byte_VentCont_Init_Commands[4]		=	( BYTE ) CC_SET_HFO_RATE;
        One_Byte_VentCont_Init_Commands[5]		=	( BYTE ) CC_SET_HFO_INSP_DELTA_PRESSURE;

        Two_Byte_VentCont_Init_Commands[0]		=	( BYTE ) CC_SET_INSP_TIME;
        Two_Byte_VentCont_Init_Commands[1]		=	( BYTE ) CC_SET_FRESH_GAS_FLOW;
        Two_Byte_VentCont_Init_Commands[2]		=	( BYTE ) CC_SET_RISE_TIME;
        Two_Byte_VentCont_Init_Commands[3]		=	( BYTE ) CC_SET_FALL_TIME;

        //Ventilator control initialisation data
        One_Byte_VentCont_Init_Data[0]			=	( BYTE ) 42;//in 7q1 format = 21%
        One_Byte_VentCont_Init_Data[1]			=	( BYTE ) 0;
        One_Byte_VentCont_Init_Data[2]			=	( BYTE ) 0;
        One_Byte_VentCont_Init_Data[3]			=	( BYTE ) 0;
        One_Byte_VentCont_Init_Data[4]			=	( BYTE ) 20;//3hz
        One_Byte_VentCont_Init_Data[5]			=	( BYTE ) 0;

        Two_Byte_VentCont_Init_Data[0][LO_BYTE]			=	( BYTE ) 0;
        Two_Byte_VentCont_Init_Data[0][HI_BYTE]			=	( BYTE ) 0;

        Two_Byte_VentCont_Init_Data[1][LO_BYTE]			=	( BYTE ) 80;//8lpm
        Two_Byte_VentCont_Init_Data[1][HI_BYTE]			=	( BYTE ) 0;

        Two_Byte_VentCont_Init_Data[2][LO_BYTE]			=	( BYTE ) 0;
        Two_Byte_VentCont_Init_Data[2][HI_BYTE]			=	( BYTE ) 0;

        Two_Byte_VentCont_Init_Data[3][LO_BYTE]			=	( BYTE ) 0;
        Two_Byte_VentCont_Init_Data[3][HI_BYTE]			=	( BYTE ) 0;

        //Alarm limit commands
        One_Byte_AlarmLimit_Init_Commands[CONTINUING_PRESSURE]	= ( BYTE ) MC_SET_CONTINUING_PRESSURE_ALARM;
        One_Byte_AlarmLimit_Init_Commands[APNEA_TIME]			= ( BYTE ) MC_SET_APNEA_TIME ;
        One_Byte_AlarmLimit_Init_Commands[LEAK]					= ( BYTE ) MC_SET_PATIENT_LEAK_ALARM ;
        One_Byte_AlarmLimit_Init_Commands[MAX_CPAP]				= ( BYTE ) MC_SET_MAX_CPAP_ALARM ;
        One_Byte_AlarmLimit_Init_Commands[HIGH_PEEP]			= ( BYTE ) MC_SET_HIGH_PEEP_ALARM;
        One_Byte_AlarmLimit_Init_Commands[LOW_PIP]				= ( BYTE ) MC_SET_LOW_PIP_ALARM ;

        Two_Byte_AlarmLimit_Init_Commands[LO_PRESSURE_ALARM]	= ( BYTE ) MC_SET_LO_PRESSURE_ALARM ;
        Two_Byte_AlarmLimit_Init_Commands[HI_PRESSURE_ALARM]	= ( BYTE ) MC_SET_HI_PRESSURE_ALARM ;
        Two_Byte_AlarmLimit_Init_Commands[CYCLE_FAIL]			= ( BYTE ) MC_SET_CYCLE_FAIL_ALARM ;
        Two_Byte_AlarmLimit_Init_Commands[LO_MIN_VOL_ALARM]		= ( BYTE ) MC_SET_LO_MIN_VOL_ALARM ;
        Two_Byte_AlarmLimit_Init_Commands[HI_MIN_VOL_ALARM]		= ( BYTE ) MC_SET_HI_MIN_VOL_ALARM ;
        Two_Byte_AlarmLimit_Init_Commands[LO_TIDAL_VOL_ALARM]	= ( BYTE ) MC_SET_LO_TIDAL_VOL_ALARM;
        Two_Byte_AlarmLimit_Init_Commands[HI_TIDAL_VOL_ALARM]	= ( BYTE ) MC_SET_HI_TIDAL_VOL_ALARM ;

        //Alarm limit data
        One_Byte_AlarmLimit_Init_Data[CONTINUING_PRESSURE]	= ( BYTE ) 120; //value from eng mode - and mon code
        One_Byte_AlarmLimit_Init_Data[APNEA_TIME]			= ( BYTE ) 255 ; //mbar
        One_Byte_AlarmLimit_Init_Data[LEAK]					= ( BYTE ) 51 ; //Above 50 == off
        One_Byte_AlarmLimit_Init_Data[MAX_CPAP]				= ( BYTE ) 250 ; //mbar
        One_Byte_AlarmLimit_Init_Data[HIGH_PEEP]			= ( BYTE ) 255; //spec says max == 450, however it is one byte so 255
        One_Byte_AlarmLimit_Init_Data[LOW_PIP]				= ( BYTE ) 0 ; //Set to lowest, not max

        Two_Byte_AlarmLimit_Init_Data[LO_PRESSURE_ALARM][LO_BYTE]	= ( BYTE )0x88; //value from eng mode
        Two_Byte_AlarmLimit_Init_Data[LO_PRESSURE_ALARM][HI_BYTE]	= ( BYTE )0xFF  ;

        Two_Byte_AlarmLimit_Init_Data[HI_PRESSURE_ALARM][LO_BYTE]	= ( BYTE ) 0x91 ; //value from eng mode
        Two_Byte_AlarmLimit_Init_Data[HI_PRESSURE_ALARM][HI_BYTE]	= ( BYTE ) 0 ;

        Two_Byte_AlarmLimit_Init_Data[CYCLE_FAIL][LO_BYTE]			= ( BYTE ) 0xFF ; //value from eng mode
        Two_Byte_AlarmLimit_Init_Data[CYCLE_FAIL][HI_BYTE]			= ( BYTE ) 0xFF ;

        Two_Byte_AlarmLimit_Init_Data[LO_MIN_VOL_ALARM][LO_BYTE]	= ( BYTE ) 0 ; //min from spec
        Two_Byte_AlarmLimit_Init_Data[LO_MIN_VOL_ALARM][HI_BYTE]	= ( BYTE ) 0 ;

        Two_Byte_AlarmLimit_Init_Data[HI_MIN_VOL_ALARM][LO_BYTE]	= ( BYTE ) 0xFF ; //max from spec
        Two_Byte_AlarmLimit_Init_Data[HI_MIN_VOL_ALARM][HI_BYTE]	= ( BYTE ) 0xFF ;

        Two_Byte_AlarmLimit_Init_Data[LO_TIDAL_VOL_ALARM][LO_BYTE]	= ( BYTE ) 0; //min from spec
        Two_Byte_AlarmLimit_Init_Data[LO_TIDAL_VOL_ALARM][HI_BYTE]	= ( BYTE ) 0;

        Two_Byte_AlarmLimit_Init_Data[HI_TIDAL_VOL_ALARM][LO_BYTE]	= ( BYTE ) 0xFF ; //max from spec
        Two_Byte_AlarmLimit_Init_Data[HI_TIDAL_VOL_ALARM][HI_BYTE]	= ( BYTE ) 0xFF ;

        TempTime.wDay			= 0U;
        TempTime.wDayOfWeek		= 0U;
        TempTime.wHour			= 0U;
        TempTime.wMilliseconds	= 0U;
        TempTime.wMinute		= 0U;
        TempTime.wMonth			= 0U;
        TempTime.wSecond		= 0U;
        TempTime.wYear			= 0U;

        SendMessagesImmediately = FALSE;
        EnableO2CalLoop			= FALSE;				
    }
    /*!***************************************************************************************************
    * \brief Destructor
    ******************************************************************************************************/
    CCommsTransport::~CCommsTransport( void )
    {

    }

	void CCommsTransport::RequestSerialNumbers(void)
	{
		RequestControllerSN();
		RequestMonitorSN();
		RequestMmsSN();
		RequestESSN();
	}

	void CCommsTransport::RequestControllerSN(void)
	{
		Raw_UI_To_MMS_Data_struct_type CAN_Message;
		memset(&CAN_Message, 0, sizeof(Raw_UI_To_MMS_Data_struct_type));

        CAN_Message.CAN_DLC_u8				= ( BYTE )DLENGTH2;
        CAN_Message.CAN_ID_u8				= ( BYTE )CONTROLLER_COMMAND;
        CAN_Message.Dest_Subsystem_ID_u8	= ( BYTE )SPI_SUBS_ID_CONTROLLER;
        CAN_Message.Payload_u8[0]			= ( BYTE )CC_GET_CONFIG;

		//request low word
        CAN_Message.Payload_u8[1]			= ( BYTE )CD_VENTILATOR_SERIAL_NUMBER_LOW_WORD ;
        AddCANMessageToBuffer( ( Raw_UI_To_MMS_Data_struct_type )CAN_Message,( UINT )NOTB, ( const bool )FALSE );

		//request high word
        CAN_Message.Payload_u8[1]			= ( BYTE )CD_VENTILATOR_SERIAL_NUMBER_HIGH_WORD ;
        AddCANMessageToBuffer( ( Raw_UI_To_MMS_Data_struct_type )CAN_Message, ( UINT )NOTB, ( const bool )FALSE );
	}
   void CCommsTransport::RequestMonitorSN(void)
    {
        Raw_UI_To_MMS_Data_struct_type CAN_Message;
		memset(&CAN_Message, 0, sizeof(Raw_UI_To_MMS_Data_struct_type));

        CAN_Message.CAN_DLC_u8				= ( BYTE )DLENGTH2;
        CAN_Message.CAN_ID_u8				= ( BYTE )MONITOR_COMMAND;
        CAN_Message.Dest_Subsystem_ID_u8	= ( BYTE )SPI_SUBS_ID_MONITOR;

        CAN_Message.Payload_u8[0]			= ( BYTE )MC_GET_CONFIG;
        CAN_Message.Payload_u8[1]			= ( BYTE )MD_GET_SERIAL_NUMBER;

        AddCANMessageToBuffer( ( Raw_UI_To_MMS_Data_struct_type )CAN_Message, ( UINT )NOTB, ( const bool )FALSE );
    }

    void CCommsTransport::RequestMmsSN(void)
    {
        Raw_UI_To_MMS_Data_struct_type CAN_Message;
		memset(&CAN_Message, 0, sizeof(Raw_UI_To_MMS_Data_struct_type));

        CAN_Message.CAN_DLC_u8				= ( BYTE )DLENGTH1;
        CAN_Message.CAN_ID_u8				= ( BYTE )MMS_CAN_ID;
        CAN_Message.Dest_Subsystem_ID_u8	= ( BYTE )SPI_SUBS_ID_MMS_LOCAL;
        CAN_Message.Payload_u8[0]			= ( BYTE )MMS_LOCAL_CMD_GET_SERIAL_NUM;

        AddCANMessageToBuffer( ( Raw_UI_To_MMS_Data_struct_type )CAN_Message, ( UINT )NOTB, ( const bool )FALSE );
    }

    void CCommsTransport::RequestESSN(void)
    {
		Raw_UI_To_MMS_Data_struct_type CAN_Message;
		memset(&CAN_Message, 0, sizeof(Raw_UI_To_MMS_Data_struct_type));

		CAN_Message.CAN_DLC_u8				= ( BYTE )DLENGTH2;
        CAN_Message.CAN_ID_u8				= ( BYTE )EXTERNAL_SENSOR_COMMAND;
        CAN_Message.Dest_Subsystem_ID_u8	= ( BYTE )SPI_SUBS_ID_MONITOR;
		CAN_Message.Payload_u8[0]			= ( BYTE )ESMODefinitions::ESC_GET_CONFIG;			
		CAN_Message.Payload_u8[1]			= ( BYTE )ESMODefinitions::ESD_SERIAL_NUMBER;		

        AddCANMessageToBuffer( ( Raw_UI_To_MMS_Data_struct_type )CAN_Message,( UINT )NOTB, ( const bool )FALSE );
    }
    void CCommsTransport::SetSubsystemSerialNumbers( const  uint NewSerialNumber )
    {
		SetControllerSN( NewSerialNumber );
		SetMonitorSN( NewSerialNumber );
        SetMmsSN( NewSerialNumber );
		SetESSN( NewSerialNumber );
		
		//Call function to request the serial numbers from the subsystems 	
		Sleep(500U);
		RequestSerialNumbers();		
    }

	void CCommsTransport::SetControllerSN( const uint NewSerialNumber )
	{
		Raw_UI_To_MMS_Data_struct_type CAN_Message;
		
		memset(&CAN_Message, 0, sizeof(Raw_UI_To_MMS_Data_struct_type));

		CAN_Message.CAN_DLC_u8				= ( BYTE )DLENGTH6;
		CAN_Message.CAN_ID_u8				= ( BYTE )CONTROLLER_COMMAND;
		CAN_Message.Dest_Subsystem_ID_u8	= ( BYTE )SPI_SUBS_ID_CONTROLLER;
		CAN_Message.Payload_u8[0]			= ( BYTE )CC_SET_VENTILATOR_SERIAL_NUMBER;	

		CAN_Message.Payload_u8[4]			= ( BYTE ) NewSerialNumber & 0xFFU;										
		CAN_Message.Payload_u8[3]			= ( BYTE )( NewSerialNumber >> 8U ) & 0xFFU;												
		CAN_Message.Payload_u8[2]			= ( BYTE )( NewSerialNumber >> 16U ) & 0xFFU;			
		CAN_Message.Payload_u8[1]			= ( BYTE )( NewSerialNumber >> 24U ) & 0xFFU;

		AddCANMessageToBuffer( ( Raw_UI_To_MMS_Data_struct_type )CAN_Message, ( UINT )TBPOS6, ( const bool )FALSE );
	}

	
    void CCommsTransport::SetMonitorSN( const uint NewSerialNumber )
    {
        Raw_UI_To_MMS_Data_struct_type CAN_Message;
		
		memset(&CAN_Message, 0, sizeof(Raw_UI_To_MMS_Data_struct_type));

        CAN_Message.CAN_DLC_u8				= ( BYTE )DLENGTH6;
        CAN_Message.CAN_ID_u8				= ( BYTE )MONITOR_COMMAND;
        CAN_Message.Dest_Subsystem_ID_u8	= ( BYTE )SPI_SUBS_ID_MONITOR;
        CAN_Message.Payload_u8[0]			= ( BYTE )MC_SET_SERIAL_NUMBER;

        CAN_Message.Payload_u8[4]			= ( BYTE ) NewSerialNumber & 0xFFU ;			
        CAN_Message.Payload_u8[3]			= ( BYTE )( NewSerialNumber >> 8U ) & 0xFFU;					
        CAN_Message.Payload_u8[2]			= ( BYTE )( NewSerialNumber >> 16U ) & 0xFFU;			
        CAN_Message.Payload_u8[1]			= ( BYTE )( NewSerialNumber >> 24U ) & 0xFFU;

        AddCANMessageToBuffer( ( Raw_UI_To_MMS_Data_struct_type )CAN_Message, ( UINT )TBPOS6, ( const bool )FALSE );

    }

    void CCommsTransport::SetMmsSN( const uint NewSerialNumber )
    {
        Raw_UI_To_MMS_Data_struct_type CAN_Message;
		
		memset(&CAN_Message, 0, sizeof(Raw_UI_To_MMS_Data_struct_type));

        CAN_Message.CAN_DLC_u8				= ( BYTE )DLENGTH6;
        CAN_Message.CAN_ID_u8				= ( BYTE )MMS_CAN_ID;
        CAN_Message.Dest_Subsystem_ID_u8	= ( BYTE )SPI_SUBS_ID_MMS_LOCAL;
        CAN_Message.Payload_u8[0]			= ( BYTE )MMS_LOCAL_CMD_SET_SERIAL_NUM; 

        CAN_Message.Payload_u8[4]			= ( BYTE ) NewSerialNumber & 0xFFU ;			
        CAN_Message.Payload_u8[3]			= ( BYTE )( NewSerialNumber >> 8U ) & 0xFFU;					
        CAN_Message.Payload_u8[2]			= ( BYTE )( NewSerialNumber >> 16U ) & 0xFFU;			
        CAN_Message.Payload_u8[1]			= ( BYTE )( NewSerialNumber >> 24U ) & 0xFFU;

        AddCANMessageToBuffer( ( Raw_UI_To_MMS_Data_struct_type )CAN_Message, ( UINT )TBPOS6, ( const bool )FALSE );
    }

    void CCommsTransport::SetESSN( const uint NewSerialNumber )
    {		
        Raw_UI_To_MMS_Data_struct_type CAN_Message;

		memset(&CAN_Message, 0, sizeof(Raw_UI_To_MMS_Data_struct_type));

        CAN_Message.CAN_DLC_u8				= ( BYTE )DLENGTH6;
        CAN_Message.CAN_ID_u8				= ( BYTE )EXTERNAL_SENSOR_COMMAND;
        CAN_Message.Dest_Subsystem_ID_u8	= ( BYTE )SPI_SUBS_ID_MONITOR;
        CAN_Message.Payload_u8[0]			= ( BYTE )61; //to be addressed LG

        CAN_Message.Payload_u8[4]			= ( BYTE ) NewSerialNumber & 0xFFU ;			
        CAN_Message.Payload_u8[3]			= ( BYTE )( NewSerialNumber >> 8U ) & 0xFFU;					
        CAN_Message.Payload_u8[2]			= ( BYTE )( NewSerialNumber >> 16U ) & 0xFFU;			
        CAN_Message.Payload_u8[1]			= ( BYTE )( NewSerialNumber >> 24U ) & 0xFFU;

        AddCANMessageToBuffer( ( Raw_UI_To_MMS_Data_struct_type )CAN_Message,( UINT )TBPOS6, ( const bool )FALSE );
    }

    /*!***************************************************************************************************
    * \brief Sets the comms to model interface so that the comms can communciate with the rest of the system
    * \param ModelPointer A pointer to the model interface
    ******************************************************************************************************/
    void CCommsTransport::SetCommsToDataInterface( CMVInterfaces::ICANToData* const ModelPointer )
    {
        m_ICANToData	=	ModelPointer;
    }
    /*!***************************************************************************************************
    * \brief Function which deals with sending a set ventilation mode message to the controller
    * \param SetMode The can number of the mode which the controller should be set to
    ******************************************************************************************************/
    void CCommsTransport::SetVentilationMode( const int SetMode )
    {
        //Tell the controller
        Raw_UI_To_MMS_Data_struct_type CAN_Message;

        CAN_Message.CAN_DLC_u8				= ( BYTE )DLENGTH3;
        CAN_Message.CAN_ID_u8				= ( BYTE )CONTROLLER_COMMAND;
        CAN_Message.Dest_Subsystem_ID_u8	= ( BYTE )SPI_SUBS_ID_CONTROLLER;
        CAN_Message.Payload_u8[0]			= ( BYTE )CC_SET_BREATH_MODE;
        CAN_Message.Payload_u8[1]			= ( BYTE )SetMode;
        //CAN_Message.Payload_u8[2]			= ( BYTE )N.B Tracking byte

        AddCANMessageToBuffer( ( Raw_UI_To_MMS_Data_struct_type )CAN_Message, ( UINT )TBPOS3, ( const bool )TRUE );		

        //Tell the monitor were in standby mode to avoid some alarms
        CAN_Message.CAN_DLC_u8				= ( BYTE )DLENGTH3;
        CAN_Message.CAN_ID_u8				= ( BYTE )MONITOR_COMMAND;
        CAN_Message.Dest_Subsystem_ID_u8	= ( BYTE )SPI_SUBS_ID_MONITOR;
        CAN_Message.Payload_u8[0]			= ( BYTE )MC_SET_BREATH_MODE ;
        CAN_Message.Payload_u8[1]			= ( BYTE )BM_STANDBY;
        //CAN_Message.Payload_u8[2]			= ( BYTE )N.B Tracking byte

        AddCANMessageToBuffer( ( Raw_UI_To_MMS_Data_struct_type )CAN_Message, ( UINT )TBPOS3, ( const bool )FALSE );
    }

    void CCommsTransport::SetVentilationControlOneByte( const int SysID, const int Data )
    {
        Raw_UI_To_MMS_Data_struct_type CAN_Message;		

        CAN_Message.CAN_DLC_u8				= ( BYTE )DLENGTH3;
        CAN_Message.CAN_ID_u8				= ( BYTE )CONTROLLER_COMMAND;
        CAN_Message.Dest_Subsystem_ID_u8	= ( BYTE )SPI_SUBS_ID_CONTROLLER;
        CAN_Message.Payload_u8[0]			= ( BYTE )MapVentControlSysIDToCommandCANID( SysID );
        CAN_Message.Payload_u8[1]			= ( BYTE )Data;
        //CAN_Message.Payload_u8[2]			= ( BYTE )SetMode; N.B Tracking byte

        AddCANMessageToBuffer( ( Raw_UI_To_MMS_Data_struct_type )CAN_Message, ( UINT )TBPOS3, ( const bool )FALSE );

    }

    void CCommsTransport::SetVentilationControlTwoByte( const int SysID, const int Data )
    {
        Raw_UI_To_MMS_Data_struct_type CAN_Message;

        CAN_Message.CAN_DLC_u8				= ( BYTE )DLENGTH4;
        CAN_Message.CAN_ID_u8				= ( BYTE )CONTROLLER_COMMAND;
        CAN_Message.Dest_Subsystem_ID_u8	= ( BYTE )SPI_SUBS_ID_CONTROLLER;
        CAN_Message.Payload_u8[0]			= ( BYTE )MapVentControlSysIDToCommandCANID( SysID );
        CAN_Message.Payload_u8[1]			= ( BYTE )ConvertIntToTwoBytes_LowByte( Data ); //low byte
        CAN_Message.Payload_u8[2]			= ( BYTE )ConvertIntToTwoBytes_HighByte( Data ); //high byte
        //CAN_Message.Payload_u8[3]			= ( BYTE ); N.B Tracking byte

        AddCANMessageToBuffer( ( Raw_UI_To_MMS_Data_struct_type )CAN_Message, ( UINT )TBPOS4, ( const bool )FALSE );

    }

    BYTE CCommsTransport::ConvertIntToTwoBytes_HighByte( int DataIn )const
    {
        //Signedness doesnt matter in this bitwise operation.

        //lint -esym(1960, 5-0-21)
        DataIn >>= 8;//shift 8 bits
        //lint +esym(1960, 5-0-21)
        return ( BYTE ) DataIn;
    }

    BYTE CCommsTransport::ConvertIntToTwoBytes_LowByte( const int DataIn ) const
    {
        return ( BYTE ) DataIn;
    }

    BYTE CCommsTransport::MapVentControlSysIDToCommandCANID( const int SysID )const
    {
        BYTE TempData = ( BYTE )NULL;

        switch ( SysID )
        {
            case e_SET_PIP:
            {
                TempData	= ( BYTE ) CC_SET_PIP_JET_PRESSURE;
                break;
            }

            case e_SET_BPM:
            {
                TempData	= ( BYTE ) CC_SET_BPM;
                break;
            }

            case e_SET_HFO_FREQ:
            {
                TempData	= ( BYTE ) CC_SET_HFO_RATE;
                break;
            }

            case e_SET_DELTAP:
            {
                TempData	= ( BYTE ) CC_SET_HFO_DELTA_P_JET_PRESSURE;
                break;
            }

            case e_SET_CPAP_PRESSURE:
            {
                TempData	= ( BYTE ) CC_SET_BASE_JET_PRESSURE;
                break;
            }

            case e_SET_FG_FLOW:
            {
                TempData	= ( BYTE ) CC_SET_FRESH_GAS_FLOW;
                break;
            }

            case e_SET_INSP_T:
            {
                TempData	= ( BYTE ) CC_SET_INSP_TIME;
                break;
            }

            case e_SET_PIPCOMP:
            {
                TempData	= ( BYTE ) CC_SET_PIP_COMPENSATION;
                break;
            }

            case e_SET_FIO2:
            {
                TempData	= ( BYTE ) CC_SET_FIO2;
                break;
            }

            default:
                break;
        }

        return TempData;
    }


    int CCommsTransport::ConvertToQFormat( const int DataIn, const int QValue ) const
    {
        return DataIn * ( 2 * ( QValue * QValue ) ); //Data in multiplied by 2 to the power QValue
    }

    int CCommsTransport::ConvertToFloatFormat( const int DataIn, const int QValue ) const
    {
        int DataOut;
        double Intermediary = ( double )( ( double )DataIn / ( double )100 );
        double Intermediary2 =	( double )Intermediary * ( double )pow( ( double )2, ( double )QValue );
        DataOut = ( int )ceil( Intermediary2 );
        return DataOut;
    }

    void CCommsTransport::SendDefaultAlarmValues()
    {
        Raw_UI_To_MMS_Data_struct_type CAN_Message;

        //Start on the one byte messages
        CAN_Message.CAN_DLC_u8				= ( BYTE )DLENGTH3;
        CAN_Message.CAN_ID_u8				= ( BYTE )MONITOR_COMMAND;
        CAN_Message.Dest_Subsystem_ID_u8	= ( BYTE )SPI_SUBS_ID_MONITOR;

        //Init one byte vent cont messages
        for ( int i = NULL; i < TOTAL_ONE_BYTE_ALARMLIMIT_INITIALISATION_COMMANDS ; i++ )
        {
            CAN_Message.Payload_u8[0]			= ( BYTE )One_Byte_AlarmLimit_Init_Commands[i];
            CAN_Message.Payload_u8[1]			= ( BYTE )One_Byte_AlarmLimit_Init_Data[i];
            //CAN_Message.Payload_u8[2]			= TRACKING BYTE
            AddCANMessageToBuffer( ( Raw_UI_To_MMS_Data_struct_type )CAN_Message, ( UINT )TBPOS3, ( BOOL )TRUE );
        }

        //Init two byte vent cont messages
        CAN_Message.CAN_DLC_u8				= ( BYTE )DLENGTH4;

        for ( int i = NULL; i < TOTAL_TWO_BYTE_ALARMLIMIT_INITIALISATION_COMMANDS ; i++ )
        {
            CAN_Message.Payload_u8[0]			= ( BYTE )Two_Byte_AlarmLimit_Init_Commands[i];
            CAN_Message.Payload_u8[1]			= ( BYTE )Two_Byte_AlarmLimit_Init_Data[i][HI_BYTE];
            CAN_Message.Payload_u8[2]			= ( BYTE )Two_Byte_AlarmLimit_Init_Data[i][LO_BYTE];
            //CAN_Message.Payload_u8[3]			= TRACKING BYTE
            AddCANMessageToBuffer( ( Raw_UI_To_MMS_Data_struct_type )CAN_Message, ( UINT )TBPOS4, ( BOOL )TRUE );
        }
    }

    void CCommsTransport::SetMonitorHFOFreq( const int NewHFOFreq )
    {
        Raw_UI_To_MMS_Data_struct_type CAN_Message;

        //Start on the one byte messages
        CAN_Message.CAN_DLC_u8				= ( BYTE )DLENGTH3;
        CAN_Message.CAN_ID_u8				= ( BYTE )MONITOR_COMMAND;
        CAN_Message.Dest_Subsystem_ID_u8	= ( BYTE )SPI_SUBS_ID_MONITOR;
        CAN_Message.Payload_u8[0]			= ( BYTE )MC_SET_HFO_RATE;
        CAN_Message.Payload_u8[1]			= ( BYTE )NewHFOFreq;
        //CAN_Message.Payload_u8[2]			= TRACKING BYTE
        AddCANMessageToBuffer( ( Raw_UI_To_MMS_Data_struct_type )CAN_Message, ( UINT )TBPOS3, ( BOOL )TRUE );
    }

    void CCommsTransport::SetMonitorBPM( const int NewBPM )
    {
        Raw_UI_To_MMS_Data_struct_type CAN_Message;

        //Start on the one byte messages
        CAN_Message.CAN_DLC_u8				= ( BYTE )DLENGTH3;
        CAN_Message.CAN_ID_u8				= ( BYTE )MONITOR_COMMAND;
        CAN_Message.Dest_Subsystem_ID_u8	= ( BYTE )SPI_SUBS_ID_MONITOR;
        CAN_Message.Payload_u8[0]			= ( BYTE )MC_SET_BPM;
        CAN_Message.Payload_u8[1]			= ( BYTE )NewBPM;
        //CAN_Message.Payload_u8[2]			= TRACKING BYTE
        AddCANMessageToBuffer( ( Raw_UI_To_MMS_Data_struct_type )CAN_Message, ( UINT )TBPOS3, ( BOOL )TRUE );
    }

    void CCommsTransport::AutoSetHFOAlarms()
    {
        Raw_UI_To_MMS_Data_struct_type CAN_Message;

        //Start on the one byte messages
        CAN_Message.CAN_DLC_u8				= ( BYTE )DLENGTH2;
        CAN_Message.CAN_ID_u8				= ( BYTE )MONITOR_COMMAND;
        CAN_Message.Dest_Subsystem_ID_u8	= ( BYTE )SPI_SUBS_ID_MONITOR;
        CAN_Message.Payload_u8[0]			= ( BYTE )MC_AUTOSET_HFO_ALARMS;
        //CAN_Message.Payload_u8[1]			= TRACKING BYTE
        AddCANMessageToBuffer( ( Raw_UI_To_MMS_Data_struct_type )CAN_Message, ( UINT )TBPOS2, ( BOOL )TRUE );
    }

    void CCommsTransport::SetPressureTimeConstant( int const Data )
    {
        Raw_UI_To_MMS_Data_struct_type CAN_Message;

        //Start on the one byte messages
        CAN_Message.CAN_DLC_u8				= ( BYTE )DLENGTH3;
        CAN_Message.CAN_ID_u8				= ( BYTE )MONITOR_COMMAND;
        CAN_Message.Dest_Subsystem_ID_u8	= ( BYTE )SPI_SUBS_ID_MONITOR;
        CAN_Message.Payload_u8[0]			= ( BYTE )MC_PRESSURE_TIMECTE_SET ;
        CAN_Message.Payload_u8[1]			= ( BYTE ) Data;
        //CAN_Message.Payload_u8[2]			= TRACKING BYTE
        AddCANMessageToBuffer( ( Raw_UI_To_MMS_Data_struct_type )CAN_Message, ( UINT )TBPOS3, ( BOOL )TRUE );

    }

    void CCommsTransport::SetMonitorEngineeringMode( int const Data )
    {
        Raw_UI_To_MMS_Data_struct_type CAN_Message;
        CAN_Message.CAN_DLC_u8							= ( BYTE )DLENGTH3;
        CAN_Message.CAN_ID_u8							= ( BYTE )MONITOR_COMMAND;
        CAN_Message.Dest_Subsystem_ID_u8				= ( BYTE )SPI_SUBS_ID_MONITOR;
        CAN_Message.Payload_u8[0]						= ( BYTE )MC_ENGINEERING_MODE;
        CAN_Message.Payload_u8[1]						= ( BYTE )Data;


        //Send the message
        AddCANMessageToBuffer( ( Raw_UI_To_MMS_Data_struct_type )CAN_Message, ( UINT )TBPOS3, ( const bool )TRUE );
    }

	void CCommsTransport::SetHFOMeanCompensation( int const Data )
    {
        Raw_UI_To_MMS_Data_struct_type CAN_Message;
        CAN_Message.CAN_DLC_u8							= ( BYTE )DLENGTH3;
        CAN_Message.CAN_ID_u8							= ( BYTE )CONTROLLER_COMMAND;
        CAN_Message.Dest_Subsystem_ID_u8				= ( BYTE )SPI_SUBS_ID_CONTROLLER;
        CAN_Message.Payload_u8[0]						= ( BYTE )CC_SET_HFO_MEAN_COMPENSATION;
        CAN_Message.Payload_u8[1]						= ( BYTE )Data;


        //Send the message
        AddCANMessageToBuffer( ( Raw_UI_To_MMS_Data_struct_type )CAN_Message, ( UINT )TBPOS3, ( const bool )TRUE );
    }

    void CCommsTransport::RequestPressTimeConstant()
    {
        Raw_UI_To_MMS_Data_struct_type CAN_Message;

        CAN_Message.CAN_DLC_u8							= ( BYTE )DLENGTH2;
        CAN_Message.CAN_ID_u8							= ( BYTE )MONITOR_COMMAND;
        CAN_Message.Dest_Subsystem_ID_u8				= ( BYTE )SPI_SUBS_ID_MONITOR;
        CAN_Message.Payload_u8[0]						= ( BYTE )MC_GET_CONFIG ;
        CAN_Message.Payload_u8[1]						= ( BYTE )MD_PRESSURE_TIMECTE_GET ;


        //Send the message
        AddCANMessageToBuffer( ( Raw_UI_To_MMS_Data_struct_type )CAN_Message, ( UINT )NOTB, ( const bool )TRUE );

    }
    void CCommsTransport::Shutdown()
    {
        Raw_UI_To_MMS_Data_struct_type CAN_Message;

        CAN_Message.CAN_DLC_u8							= ( BYTE )DLENGTH3;
        CAN_Message.CAN_ID_u8							= ( BYTE )PSU_COMMAND;
        CAN_Message.Dest_Subsystem_ID_u8				= ( BYTE )SPI_SUBS_ID_MONITOR;
        CAN_Message.Payload_u8[0]						= ( BYTE )PC_POWER_DOWN_EXECUTE;
        CAN_Message.Payload_u8[1]						= ( BYTE )NULL;
        CAN_Message.Payload_u8[2]						= ( BYTE )1;

        SendMessagesImmediately = TRUE;

        //Send the message
        AddCANMessageToBuffer( ( Raw_UI_To_MMS_Data_struct_type )CAN_Message, ( UINT )NOTB, ( const bool )TRUE );
    }

    void CCommsTransport::SendOnePointO2Cal()
    {
        Raw_UI_To_MMS_Data_struct_type CAN_Message;

        CAN_Message.CAN_DLC_u8				= ( BYTE )DLENGTH2;
        CAN_Message.CAN_ID_u8				= ( BYTE )MONITOR_COMMAND;
        CAN_Message.Dest_Subsystem_ID_u8	= ( BYTE )SPI_SUBS_ID_MONITOR;
        CAN_Message.Payload_u8[0]			= ( BYTE )MC_CALIBRATE_OXYGEN_SENSOR;

        AddCANMessageToBuffer( ( Raw_UI_To_MMS_Data_struct_type )CAN_Message, ( UINT )TBPOS2, ( const bool )TRUE );

    }

    /*!***************************************************************************************************
    * \brief The interface from model > CAN. Allows the model to send can messages
    * \param SysID system id for the message being sent. sysid's can be found in globals.h
    * \param Data the data associated with the message.
    ******************************************************************************************************/
    int CCommsTransport::SetCANFromData( const int SysID, const int Data )
    {
        int Temp_Error_Code = 0;
        Raw_UI_To_MMS_Data_struct_type CAN_Message;

        switch ( SysID )
        {
            //Sys controls
            case e_INIT:
            {
				RequestInitialiseControllerModel( this );
                RequestInitialiseMonitorModel( this );
                RequestInitialiseESMOModel( this );
                RequestInitialiseVentContModel ( this );               
                Temp_Error_Code = 2;
                break;
            }
			case e_PNEUMATICS_TYPE:
			{
				RequestPneumaticType();
				break;
			}

			case e_CHANGE_PNEUMATIC_TYPE:
			{
				SetPneumaticType( Data );
				RequestPneumaticType();
				break;
			}

		    case e_CONTROLLER_UPDATE:
            {
                RequestUpdateController();
                break;
            }

            case e_ESMO_UPDATE:
            {
				RequestUpdateESMO();				
                break;
            }

            case e_MONITOR_UPDATE:
            {
                RequestUpdateMonitor();
                SendDefaultAlarmValues();
                MuteAlarm();

                if ( EnableO2CalLoop == TRUE )
                {
                    SendOnePointO2Cal();
                }
                break;
            }

            case e_SHUTDOWN_INVOKE:
            {
                Shutdown();
                break;
            }

            case e_OXYGEN_ONE_POINT_CAL_LOOP:
            {
                if ( Data == ON )
                {
                    EnableO2CalLoop = TRUE;
                }

                else
                {
                    EnableO2CalLoop = FALSE;
                }
                break;
            }

			case e_SET_SERIAL_NUMBER:
			{
				SetSubsystemSerialNumbers((uint)Data);
				break;
			}

			case e_REQUEST_ALL_SERIAL_NUMBERS:
			{
				RequestSerialNumbers();							
				break;
			}	
			case e_REQUEST_MON_SERIAL_NUMBER:
			{
				RequestMonitorSN();								
				break;
			}
			case e_REQUEST_MMS_SERIAL_NUMBER:
			{
				RequestMmsSN();								
				break;
			}

			case e_REQUEST_CON_SERIAL_NUMBER:
			{
				RequestControllerSN();								
				break;
			}
			case e_REQUEST_ES_SERIAL_NUMBER:
			{
				RequestESSN();								
				break;
			}

            //Vent control messages
            //monitor
            case e_SET_BPM_MON:
            {
                SetMonitorBPM( Data );
                break;
            }

            case e_SET_HFO_FREQ_MON:
            {
                SetMonitorHFOFreq( Data );
                break;
            }

            case e_SET_HFO_FREQ://8Q0
            case e_SET_PIP://8Q0
            case e_SET_DELTAP://8Q0
            case e_SET_CPAP_PRESSURE://8Q0
            case e_SET_BPM://8Q0
            {
                SetVentilationControlOneByte( SysID, Data );
                AutoSetHFOAlarms();				
                break;
            }

            case e_SET_FIO2://7Q1
            {
                SendMessagesImmediately = TRUE;
                SetVentilationControlOneByte( SysID, ConvertToQFormat( Data, 1 ) );
                break;
            }

            //two byte commands
            case e_SET_FG_FLOW://1/10
            case e_SET_INSP_T:
            {
                SetVentilationControlTwoByte( SysID, Data );
                break;
            }

            case e_SET_STANDBY:
            {
                SetVentilationMode( BM_STANDBY );
                Temp_Error_Code = 1;
                break;
            }

            case e_SET_CMV:
            {
                SetVentilationMode( BM_CMV );
				m_SpiToCanbusManager.SetPressureTimeBase(e_SET_CMV);
                SendDefaultAlarmValues();
                Temp_Error_Code = 1;
                break;
            }

            case e_SET_CPAP_MODE:
            {
                SetVentilationMode( BM_CPAP );
				m_SpiToCanbusManager.SetPressureTimeBase(e_SET_CPAP_MODE);
                SendDefaultAlarmValues();
                Temp_Error_Code = 1;
                break;
            }

            case e_SET_HFO:
            {
                SetVentilationMode( BM_HFO_PURE );
				m_SpiToCanbusManager.SetPressureTimeBase(e_SET_HFO);
                SendDefaultAlarmValues();
                Temp_Error_Code = 1;
                break;
            }

            case e_PMEASFILT:
            {
                SetMonitorEngineeringMode( ON );
                SetPressureTimeConstant( Data );
                SetPressureTimeConstant( Data );
                SetMonitorEngineeringMode( OFF );
                SendMessagesImmediately	= TRUE;
                RequestPressTimeConstant();
                break;
            }

            case e_SET_PIPCOMP:
            {
                CAN_Message.CAN_DLC_u8				= ( BYTE )DLENGTH3;
                CAN_Message.CAN_ID_u8				= ( BYTE )CONTROLLER_COMMAND;
                CAN_Message.Dest_Subsystem_ID_u8	= ( BYTE )SPI_SUBS_ID_CONTROLLER;
                CAN_Message.Payload_u8[0]			= ( BYTE )CC_SET_PIP_COMPENSATION;

                if ( Data == ON )
                {
                    CAN_Message.Payload_u8[1]		= ( BYTE )ENABLED;
                }

                else
                {
                    CAN_Message.Payload_u8[1]		= ( BYTE )DISABLED;
                }

                AddCANMessageToBuffer( ( Raw_UI_To_MMS_Data_struct_type )CAN_Message, ( UINT )TBPOS3, ( const bool )FALSE );
                break;

            }

            case e_OXYGEN_TWO_POINT_CAL:
            {
                CAN_Message.CAN_DLC_u8				= ( BYTE )DLENGTH2;
                CAN_Message.CAN_ID_u8				= ( BYTE )MONITOR_COMMAND;
                CAN_Message.Dest_Subsystem_ID_u8	= ( BYTE )SPI_SUBS_ID_MONITOR;
                CAN_Message.Payload_u8[0]			= ( BYTE )MC_CALIBRATE_OXYGEN_SYSTEM;

                if ( Data == ON )
                {
                    AddCANMessageToBuffer( ( Raw_UI_To_MMS_Data_struct_type )CAN_Message, ( UINT )TBPOS2, ( const bool )TRUE );
                    Temp_Error_Code = 4;
                }

                else
                {
                    //satisfies misra
                }
                break;
            }


            case e_FLOW_SYSTEM_CAL:
            {
                CAN_Message.CAN_DLC_u8				= ( BYTE )DLENGTH2;
                CAN_Message.CAN_ID_u8				= ( BYTE )MONITOR_COMMAND;
                CAN_Message.Dest_Subsystem_ID_u8	= ( BYTE )SPI_SUBS_ID_MONITOR;
                CAN_Message.Payload_u8[0]			= ( BYTE )MC_CALIBRATE_FLOW_SYSTEM;

                if ( Data == ON )
                {
                    AddCANMessageToBuffer( ( Raw_UI_To_MMS_Data_struct_type )CAN_Message, ( UINT )TBPOS2, ( const bool )TRUE );
                }

                else
                {
                    //satisfies misra
                }
                break;
            }

            case e_ENGINNERING_MODE:
            {
                SetMonitorEngineeringMode( Data );
                break;
            }

			case e_HFO_MEAN_COMP:
			{
				SetHFOMeanCompensation (Data);
				break;
			}

            case e_ZERO_PTR4_AND_PTR5_SENSORS:
            {
                CAN_Message.CAN_DLC_u8				= ( BYTE )DLENGTH2;
                CAN_Message.CAN_ID_u8				= ( BYTE )MONITOR_COMMAND;
                CAN_Message.Dest_Subsystem_ID_u8	= ( BYTE )SPI_SUBS_ID_MONITOR;
                CAN_Message.Payload_u8[0]			= ( BYTE )MC_ZERO_PRESSURE;

                if ( Data == ON )
                {
                    AddCANMessageToBuffer( ( Raw_UI_To_MMS_Data_struct_type )CAN_Message, ( UINT )TBPOS2, ( const bool )TRUE );
                }
                else
                {
                    //satisfies misra
                }

                break;
            }

            case e_RESET_ELAPSED_TIME_COUNTERS:
            {
                CAN_Message.CAN_DLC_u8				= ( BYTE )DLENGTH2;
                CAN_Message.CAN_ID_u8				= ( BYTE )CONTROLLER_COMMAND;
                CAN_Message.Dest_Subsystem_ID_u8	= ( BYTE )SPI_SUBS_ID_CONTROLLER;
                CAN_Message.Payload_u8[0]			= ( BYTE )CC_RESET_ELAPSED_TIME_COUNTERS;

                if ( Data == ON )
                {
                    AddCANMessageToBuffer( ( Raw_UI_To_MMS_Data_struct_type )CAN_Message, ( UINT )TBPOS2, ( const bool )TRUE );
                }
                else
                {
                    //satisfies misra
                }

                break;
            }

            case e_VTV_COARSE:
            {
                SendVTV_CoarseMessage( ConvertToFloatFormat( ( int )( Data ), 8 ) );
                RequestUpdateCalibrationConstant( SysID );
                break;
            }

            case e_VTV_FINE:
            {
                SendVTV_FineMessage( ConvertToFloatFormat( ( int )( Data ), 8 ) );
                RequestUpdateCalibrationConstant( SysID );
                break;
            }


            case e_LEAK_COMP:
            {
                SendMax_LeakCompensationMessage( ConvertToFloatFormat( ( int )( Data ), 8 ) );
                break;
            }  


            //Controller calibration constants
            case e_FS1_GAIN:
            case e_FS1_OFFSET:
            case e_FS2_GAIN:
            case e_FS2_OFFSET:
            case e_FS3_GAIN:
            case e_FS3_OFFSET:
            case e_PTR1_GAIN:
            case e_PTR1_OFFSET:
            case e_PTR2_GAIN:
            case e_PTR2_OFFSET:
            case e_PTR3_GAIN:
            case e_PTR3_OFFSET:
            case e_PTR6_GAIN:
            case e_PTR6_OFFSET:

            //jets + waveshaping
            case e_MEANJET_GAIN:
            case e_MEANJET_CMV:
            case e_FORWARDJET_GAIN:
            case e_FORWARDJET_HFO:
            case e_FORWARDJET_CMV:
            case e_REVERSEJET_GAIN:
            case e_REVERSEJET_HFO:
            case e_REVERSEJET_CMV:
            case e_RISEJET_FAST:
            case e_RISEJET_SLOW:
            case e_RISEJET_BALANCE:
            case e_FALLJET_FAST:
            case e_FALLJET_SLOW:
            case e_FALLJET_BALANCE: 
			case e_FG_P:
            case e_FG_I:
            case e_FG_D:				
			case e_HFO_P:
			case e_HFO_I:
			case e_HFO_D:
            {
                SendControllerCalibrationConstant( SysID, Data );
                RequestUpdateCalibrationConstant( SysID );
                RequestUpdateCalibrationConstant( SysID );
                Temp_Error_Code = 3;
                break;
            }

			
            case e_MON_PTR4_GAIN:
            case e_MON_PTR5_GAIN:
            {
                SendMonitorCalibrationMessage( SysID, Data );
                SendMessagesImmediately = TRUE;
                RequestUpdateMonitorCalibrationConstant( SysID ); //Update UI
                break;
            }

            case e_ESMO_ETCO2_START_CAL_CONFIRMED:
            {
                if ( Data == ON )
                {
					//calibration check procedure
                    SetETCO2_ServiceMode();
                    SetETCO2_BTPS();
                    SendESMO_ETCO2StartCal();										
                }
                break;
            }

            case e_ESMO_ETCO2_CHECK_CAL_CONFIRMED:
            {
                if ( Data == ON )
                {
                    SendESMOETCO2CalCheck();									
                }
                break;
            }

            case e_ESMO_ETCO2_CAL_ABORT:
            {
                if ( Data == ON )
                {
                    SendESMOETCO2CalAbort();
                }
                break;
            }

			case e_ETCO2_NORMAL_MODE:
            {
                if ( Data == ON )
                {
					SetETCO2_NormalMode(); 					
                }
                break;
            }

			case e_SET_ETCO2_PUMP:
			{
				SetETCO2_PumpState(Data);
				break;
			}
			

            //Graph display control
            case e_GRAPH_PRES_DISPLAY:
            {
                if ( Data == ON )
                {
                    m_SpiToCanbusManager.ShowPresGraph();
                }
                else
                {
                    m_SpiToCanbusManager.HidePresGraph();
                }
                break;
            }

            case e_GRAPH_FLOW_DISPLAY:
            {
                if ( Data == ON )
                {
                    m_SpiToCanbusManager.ShowFlowGraph();
                }

                else
                {
                    m_SpiToCanbusManager.HideFlowGraph();
                }
                break;
            }

            case e_GRAPH_PRESSURE_LIMITS_DISPLAY:
            {
                if ( Data == ON )
                {
                    m_SpiToCanbusManager.ShowPressureLimits();
                }
                else
                {
                    m_SpiToCanbusManager.HidePressureLimits();
                }

                break;
            }

            case e_GRAPH_PRESSURE_LIMITS_EDIT:
            {
                m_SpiToCanbusManager.SetLimitValue( Data );
                break;
            }

            case e_SET_NEBULISER:
            {
                if ( Data == ON )
                {
                    SetNebuliser( ON );
                }
                else
                {
                    SetNebuliser( OFF );
                }
                break;
            }

            case e_INIT_STBJ:
            {
                InitBreathJetSelfTest();
                break;
            }

            default:
                break;
        }

        return Temp_Error_Code;
    }

	void CCommsTransport::SetETCO2_PumpState(const int bPumpState)
	{
		BYTE	byParamVal = 0x00U;

		if (bPumpState == TRUE)
		{
			byParamVal = 0x01U; //Value for ON (RS0092 Set Pump State)
		}		 

		Raw_UI_To_MMS_Data_struct_type		CAN_Message;
		memset( &CAN_Message, 0, sizeof( Raw_UI_To_MMS_Data_struct_type ) );

        CAN_Message.CAN_DLC_u8				= ( BYTE )DLENGTH3;
        CAN_Message.CAN_ID_u8				= ( BYTE )MONITOR_COMMAND;
        CAN_Message.Dest_Subsystem_ID_u8	= ( BYTE )SPI_SUBS_ID_MONITOR;
        CAN_Message.Payload_u8[0]			= ( BYTE )ESMODefinitions::MC_SET_ETCO2_PUMP_STATE;
		CAN_Message.Payload_u8[1]			= ( BYTE )byParamVal;
        AddCANMessageToBuffer( ( Raw_UI_To_MMS_Data_struct_type )CAN_Message, ( UINT )NOTB, ( const bool )FALSE );
	}

    void CCommsTransport::SendESMOETCO2CalCheck()
    {
        Raw_UI_To_MMS_Data_struct_type		CAN_Message;
        CAN_Message.CAN_DLC_u8				= ( BYTE )DLENGTH1;
        CAN_Message.CAN_ID_u8				= ( BYTE )EXTERNAL_SENSOR_COMMAND;
        CAN_Message.Dest_Subsystem_ID_u8	= ( BYTE )SPI_SUBS_ID_MONITOR;
        CAN_Message.Payload_u8[0]			= ( BYTE )ESMODefinitions::ESC_ETCO2_CALIB_CHECK;
        AddCANMessageToBuffer( ( Raw_UI_To_MMS_Data_struct_type )CAN_Message, ( UINT )NOTB, ( const bool )FALSE );
    }

    void CCommsTransport::SendESMOETCO2CalAbort()
    {
        Raw_UI_To_MMS_Data_struct_type		CAN_Message;
        CAN_Message.CAN_DLC_u8				= ( BYTE )DLENGTH1;
        CAN_Message.CAN_ID_u8				= ( BYTE )EXTERNAL_SENSOR_COMMAND;
        CAN_Message.Dest_Subsystem_ID_u8	= ( BYTE )SPI_SUBS_ID_MONITOR;
        CAN_Message.Payload_u8[0]			= ( BYTE )ESMODefinitions::ESC_ABORT_ETCO2_CALIB;
        AddCANMessageToBuffer( ( Raw_UI_To_MMS_Data_struct_type )CAN_Message, ( UINT )NOTB, ( const bool )FALSE );
    }

    void CCommsTransport::SetNebuliser( int const NewState )
    {
        Raw_UI_To_MMS_Data_struct_type CAN_Message;
        //Assemble Message constants
        CAN_Message.CAN_DLC_u8				= ( BYTE )DLENGTH3;
        CAN_Message.CAN_ID_u8				= ( BYTE )CONTROLLER_COMMAND;
        CAN_Message.Dest_Subsystem_ID_u8	= ( BYTE )SPI_SUBS_ID_CONTROLLER;
        CAN_Message.Payload_u8[0]			= ( BYTE )CC_SET_NEBULISER ;
        //Fill in message variables
        CAN_Message.Payload_u8[1]			= ( BYTE )NewState;
        //CAN_Message.Payload_u8[2]			= ( BYTE )N.B Tracking Byte;

        //CAN_Message.Payload_u8[2]			=(BYTE);	TRACKING BYTE
        AddCANMessageToBuffer( ( Raw_UI_To_MMS_Data_struct_type )CAN_Message, ( UINT )TBPOS3, ( const bool )TRUE );
    }

    void CCommsTransport::InitBreathJetSelfTest(  )
    {
        Raw_UI_To_MMS_Data_struct_type CAN_Message;
        //Assemble Message constants
        CAN_Message.CAN_DLC_u8				= ( BYTE )DLENGTH3;
        CAN_Message.CAN_ID_u8				= ( BYTE )CONTROLLER_COMMAND;
        CAN_Message.Dest_Subsystem_ID_u8	= ( BYTE )SPI_SUBS_ID_CONTROLLER;
        CAN_Message.Payload_u8[0]			= ( BYTE )CC_START_CONTROLLER_SELF_TEST ;
        //Fill in message variables
        CAN_Message.Payload_u8[1]			= ( BYTE )1;//1 = breath jet
        //CAN_Message.Payload_u8[2]			= ( BYTE )N.B Tracking Byte;

        //CAN_Message.Payload_u8[2]			=(BYTE);	TRACKING BYTE
        AddCANMessageToBuffer( ( Raw_UI_To_MMS_Data_struct_type )CAN_Message, ( UINT )TBPOS3, ( const bool )TRUE );
    }


    void CCommsTransport::SetETCO2_BTPS()
    {
        Raw_UI_To_MMS_Data_struct_type CAN_Message;
        //Assemble Message constants
        CAN_Message.CAN_DLC_u8				= ( BYTE )DLENGTH5;
        CAN_Message.CAN_ID_u8				= ( BYTE )EXTERNAL_SENSOR_COMMAND;
        CAN_Message.Dest_Subsystem_ID_u8	= ( BYTE )SPI_SUBS_ID_MONITOR;
        CAN_Message.Payload_u8[0]			= ( BYTE )ESMODefinitions::ESC_SET_ETCO2_PARAMETER;
        //Fill in message variables
        CAN_Message.Payload_u8[1]			= ( BYTE )ETCO2_BTPS_DISABLED;
        CAN_Message.Payload_u8[2]			= ( BYTE )ETCO2_DEFAULT_PEAK_PICKING;
        CAN_Message.Payload_u8[3]			= ( BYTE )ETCO2_DEFAULT_PATIENT_MODE;
        CAN_Message.Payload_u8[4]			= ( BYTE )ETCO2_DEFAULT_AUTO_STANDBY;

        //CAN_Message.Payload_u8[2]			=(BYTE);	TRACKING BYTE
        AddCANMessageToBuffer( ( Raw_UI_To_MMS_Data_struct_type )CAN_Message, ( UINT )TBPOS5, ( const bool )FALSE );

    }

    void CCommsTransport::SetETCO2_ServiceMode()
    {
        Raw_UI_To_MMS_Data_struct_type CAN_Message;
        //Assemble Message constants
        CAN_Message.CAN_DLC_u8				= ( BYTE )DLENGTH2;
        CAN_Message.CAN_ID_u8				= ( BYTE )EXTERNAL_SENSOR_COMMAND;
        CAN_Message.Dest_Subsystem_ID_u8	= ( BYTE )SPI_SUBS_ID_MONITOR;
        CAN_Message.Payload_u8[0]			= ( BYTE )ESMODefinitions::ESC_SET_ETCO2_OP_MODE;
        //Fill in message variables
        CAN_Message.Payload_u8[1]			= ( BYTE )SERVICE_MODE;

        //CAN_Message.Payload_u8[2]			=(BYTE);	TRACKING BYTE
        AddCANMessageToBuffer( ( Raw_UI_To_MMS_Data_struct_type )CAN_Message, ( UINT )NOTB, ( const bool )FALSE );
    }

	void CCommsTransport::SetETCO2_NormalMode()
    {
        Raw_UI_To_MMS_Data_struct_type CAN_Message;
        //Assemble Message constants
        CAN_Message.CAN_DLC_u8				= ( BYTE )DLENGTH2;
        CAN_Message.CAN_ID_u8				= ( BYTE )EXTERNAL_SENSOR_COMMAND;
        CAN_Message.Dest_Subsystem_ID_u8	= ( BYTE )SPI_SUBS_ID_MONITOR;
        CAN_Message.Payload_u8[0]			= ( BYTE )ESMODefinitions::ESC_SET_ETCO2_OP_MODE;
        //Fill in message variables
        CAN_Message.Payload_u8[1]			= ( BYTE )NORMAL_OPERATION;

        //CAN_Message.Payload_u8[2]			=(BYTE);	TRACKING BYTE
        AddCANMessageToBuffer( ( Raw_UI_To_MMS_Data_struct_type )CAN_Message, ( UINT )NOTB, ( const bool )FALSE );
    }

    void CCommsTransport::SendESMO_ETCO2StartCal()
    {
        Raw_UI_To_MMS_Data_struct_type CAN_Message;
        //Get the system time for use in calibration
        GetSystemTime( &TempTime );
        //Assemble Message constants
        CAN_Message.CAN_DLC_u8				= ( BYTE )DLENGTH7;
        CAN_Message.CAN_ID_u8				= ( BYTE )EXTERNAL_SENSOR_COMMAND;
        CAN_Message.Dest_Subsystem_ID_u8	= ( BYTE )SPI_SUBS_ID_MONITOR;
        CAN_Message.Payload_u8[0]			= ( BYTE )ESMODefinitions::ESC_START_ETCO2_CALIB;
        //Fill in message variables
        CAN_Message.Payload_u8[1]			= ( BYTE )TempTime.wDay;
        CAN_Message.Payload_u8[2]			= ( BYTE )TempTime.wMonth;
        CAN_Message.Payload_u8[3]			= ( BYTE )TempTime.wYear - ( BYTE )ETCO2_CAL_YEAR_OFFSET;
        CAN_Message.Payload_u8[4]			= ( BYTE )TempTime.wHour;
        CAN_Message.Payload_u8[5]			= ( BYTE )TempTime.wMinute;
        
        AddCANMessageToBuffer( ( Raw_UI_To_MMS_Data_struct_type )CAN_Message, ( UINT )TBPOS7, ( const bool )FALSE );
    }


    /*!***************************************************************************************************
    * \brief Function which encapsulates the polling of the monitor for its current value for a particular
    		 calibration constant
    * \param SysID Defined in globals.h, Used to determine which calibration constant is being requested
    ******************************************************************************************************/
    void CCommsTransport::RequestUpdateMonitorCalibrationConstant( const int SysID )
    {
        Raw_UI_To_MMS_Data_struct_type CAN_Message;
        memset( &CAN_Message, 0, sizeof( Raw_UI_To_MMS_Data_struct_type ) );

        CAN_Message.CAN_DLC_u8				= ( BYTE )DLENGTH2;
        CAN_Message.CAN_ID_u8				= ( BYTE )MONITOR_COMMAND;
        CAN_Message.Dest_Subsystem_ID_u8	= ( BYTE )SPI_SUBS_ID_MONITOR;
        CAN_Message.Payload_u8[0]			= ( BYTE )MC_GET_CONFIG;
        CAN_Message.Payload_u8[1]			= ( BYTE )MapMonitorSysIDToCANID( SysID );


        //Send the message
        AddCANMessageToBuffer( ( Raw_UI_To_MMS_Data_struct_type )CAN_Message, ( UINT )NOTB, ( const bool )TRUE );
    }
    /*!***************************************************************************************************
    * \brief Sets a MONITOR calibration constant to the data passed to it
    * \param CANID The canid (defined in msg.h) that should be sent. (the calibration constant to change)
    * \param Data The new value that the calibration constant should be set to
    ******************************************************************************************************/
    void CCommsTransport::SendMonitorCalibrationMessage( const int SysID, const int_32_t Data )
    {
        BYTE CANID	=	MapMonitorSysIDToCANID( ( int )SysID );

        Raw_UI_To_MMS_Data_struct_type CAN_Message;
        memset( &CAN_Message, 0, sizeof( Raw_UI_To_MMS_Data_struct_type ) );

        CAN_Message.CAN_DLC_u8							= ( BYTE )DLENGTH3;
        CAN_Message.CAN_ID_u8							= ( BYTE )MONITOR_COMMAND;
        CAN_Message.Dest_Subsystem_ID_u8				= ( BYTE )SPI_SUBS_ID_MONITOR;

        if ( ( BYTE )CANID == ( BYTE ) MD_1_PSI_GAIN )
        {
            CAN_Message.Payload_u8[0]	= ( BYTE )MC_ADJUST_PRESSURE_1_PSI;
        }

        else
        {
            CAN_Message.Payload_u8[0]	= ( BYTE )MC_ADJUST_PRESSURE_2_5_PSI;
        }

        CAN_Message.Payload_u8[1]	= ( BYTE )Data;

        //Send the message
        AddCANMessageToBuffer( ( Raw_UI_To_MMS_Data_struct_type )CAN_Message, ( UINT )TBPOS3, ( const bool )FALSE );

    }

    /*!***************************************************************************************************
    * \brief Maps system id's as defined in globals.h to can ids as defined in msg.h
    * \param SysID The system id value that should be mapped from
    ******************************************************************************************************/
    BYTE CCommsTransport::MapMonitorSysIDToCANID( const int SysID ) const
    {
        BYTE TempData = ( BYTE )NULL;

        switch ( SysID )
        {
            //PTR4 Sensor
            case e_MON_PTR4_GAIN:
            {
                TempData = ( BYTE ) MD_1_PSI_GAIN;
                break;
            }

            //PTR5 Sensor
            case e_MON_PTR5_GAIN:
            {
                TempData = ( BYTE ) MD_2_5_PSI_GAIN;
                break;
            }

            //PTR4 Sensor
            case e_MON_PTR4_OFFSET:
            {
                TempData = ( BYTE ) MD_1_PSI_OFFSET ;
                break;
            }

            //PTR5 Sensor
            case e_MON_PTR5_OFFSET:
            {
                TempData = ( BYTE ) MD_2_5_PSI_OFFSET ;
                break;
            }			

            default:
                break;
        }

        return TempData;
    }

    void CCommsTransport::RequestInitialiseVentContModel( const LPVOID lpdwThreadParam ) const
    {
        CCommsTransport* pCCommsTransport = ( CCommsTransport* )lpdwThreadParam;
        Raw_UI_To_MMS_Data_struct_type CAN_Message;

        //Start on the one byte messages
        CAN_Message.CAN_DLC_u8				= ( BYTE )DLENGTH3;
        CAN_Message.CAN_ID_u8				= ( BYTE )CONTROLLER_COMMAND;
        CAN_Message.Dest_Subsystem_ID_u8	= ( BYTE )SPI_SUBS_ID_CONTROLLER;

        //Init one byte vent cont messages
        for ( int i = NULL; i < TOTAL_ONE_BYTE_VENTCONT_INITIALISATION_COMMANDS ; i++ )
        {
            CAN_Message.Payload_u8[0]			= ( BYTE )One_Byte_VentCont_Init_Commands[i];
            CAN_Message.Payload_u8[1]			= ( BYTE )One_Byte_VentCont_Init_Data[i];
            //CAN_Message.Payload_u8[2]			= TRACKING BYTE
            pCCommsTransport->AddCANMessageToBuffer( ( Raw_UI_To_MMS_Data_struct_type )CAN_Message, ( UINT )TBPOS3, ( BOOL )TRUE );
        }

        //Init two byte vent cont messages
        CAN_Message.CAN_DLC_u8				= ( BYTE )DLENGTH4;

        for ( int i = NULL; i < TOTAL_TWO_BYTE_VENTCONT_INITIALISATION_COMMANDS ; i++ )
        {
            CAN_Message.Payload_u8[0]			= ( BYTE )Two_Byte_VentCont_Init_Commands[i];
            CAN_Message.Payload_u8[1]			= ( BYTE )Two_Byte_VentCont_Init_Data[i][LO_BYTE];
            CAN_Message.Payload_u8[2]			= ( BYTE )Two_Byte_VentCont_Init_Data[i][HI_BYTE];
            //CAN_Message.Payload_u8[3]			= TRACKING BYTE
            pCCommsTransport->AddCANMessageToBuffer( ( Raw_UI_To_MMS_Data_struct_type )CAN_Message, ( UINT )TBPOS4, ( BOOL )TRUE );
        }
    }


    /*!********************************************************************
    * \brief Used by the request update thread to initialise all of the values
    	 this is important as it keeps the use of the CAN ifnastructure down.
    	 (Once values such as gain and offset for a sensor are initialised they
    	  will not change unless the UI changes them and so dont need to be
    	 continually requested
    * \param lpdwThreadParam Used so that the function can access member functions
    	  of the CommsTransport class
    ***********************************************************************/
    void CCommsTransport::RequestInitialiseMonitorModel( const LPVOID lpdwThreadParam ) const
    {
        CCommsTransport* pCCommsTransport = ( CCommsTransport* )lpdwThreadParam;
        Raw_UI_To_MMS_Data_struct_type CAN_Message;

        CAN_Message.CAN_DLC_u8				= ( BYTE )DLENGTH2;
        CAN_Message.CAN_ID_u8				= ( BYTE )MONITOR_COMMAND;
        CAN_Message.Dest_Subsystem_ID_u8	= ( BYTE )SPI_SUBS_ID_MONITOR;
        CAN_Message.Payload_u8[0]			= ( BYTE )MC_GET_CONFIG;

        //Go through each of the calibration messages and request an update
        for ( BYTE i = ( BYTE )START_MON_CAL_MESSAGES; i <= ( BYTE )END_MON_CAL_MESSAGES ; i++ )
        {
            CAN_Message.Payload_u8[1]			= ( BYTE )i;
            pCCommsTransport->AddCANMessageToBuffer( ( Raw_UI_To_MMS_Data_struct_type )CAN_Message, ( UINT )NOTB, ( BOOL )TRUE );
        }
    }
    void CCommsTransport::MuteAlarm()
    {
        Raw_UI_To_MMS_Data_struct_type CAN_Message;

        CAN_Message.CAN_DLC_u8				= ( BYTE )DLENGTH2;
        CAN_Message.CAN_ID_u8				= ( BYTE )MONITOR_COMMAND;
        CAN_Message.Dest_Subsystem_ID_u8	= ( BYTE )SPI_SUBS_ID_MONITOR;
        CAN_Message.Payload_u8[0]			= ( BYTE )MC_MUTE_ALARM ;
        // CAN_Message.Payload_u8[1]			= ( BYTE )TRACKING BYTE

        //Send the message
        AddCANMessageToBuffer( ( Raw_UI_To_MMS_Data_struct_type )CAN_Message, ( UINT )TBPOS2, ( const bool )TRUE );


    }
    /*!***************************************************************************************************
    * \brief Function which encapsulates the polling of the controller for its current value for a particular
    		 calibration constant
    * \param SysID Defined in globals.h, Used to determine which calibration constant is being requested
    ******************************************************************************************************/
    void CCommsTransport::RequestUpdateCalibrationConstant( const int SysID )
    {
        Raw_UI_To_MMS_Data_struct_type CAN_Message;

        CAN_Message.CAN_DLC_u8				= ( BYTE )DLENGTH2;
        CAN_Message.CAN_ID_u8				= ( BYTE )CONTROLLER_COMMAND;
        CAN_Message.Dest_Subsystem_ID_u8	= ( BYTE )SPI_SUBS_ID_CONTROLLER;
        CAN_Message.Payload_u8[0]			= ( BYTE )CC_GET_CONFIG;
        CAN_Message.Payload_u8[1]			= ( BYTE )MapControllerSysIDToCANID( SysID );

        SendMessagesImmediately = TRUE;
        //Send the message
        AddCANMessageToBuffer( ( Raw_UI_To_MMS_Data_struct_type )CAN_Message, ( UINT )NOTB, ( const bool )TRUE );
    }



    /*!***************************************************************************************************
    * \brief Encapsulates all parts of sending a controller calibration message. This includes mapping
    		 the sysid to a canid, sending an enable calibration message and sending a message to actually
    		 change the constant
    * \param SysID Defined in globals.h, Used to determine which calibration constant is being updated
    * \param Data The new value that the calibration cosnthould be set to
    ******************************************************************************************************/
    void CCommsTransport::SendControllerCalibrationConstant( const int SysID, const int_32_t Data )
    {
        //Map SysID to CAN ID
        BYTE CANID	=	MapControllerSysIDToCANID( ( int )SysID );
        //Send controller calibration enable message for the required constant
        SendEnableCalibration( ( BYTE )CANID );
        //Update the actual value
        SendCalibrationData( ( BYTE )CANID, ( int ) Data );
    }


    /*!***************************************************************************************************
    * \brief Sets a controller calibration constant to the data passed to it
    * \param CANID The canid (defined in msg.h) that should be sent. (the calibration constant to change)
    * \param Data The new value that the calibration constant should be set to
    ******************************************************************************************************/
    void CCommsTransport::SendCalibrationData( const BYTE CANID, const int_32_t Data )
    {
        //lint -esym(1960,5-0-6)
        //lint -esym(1960,5-0-3)
        //lint -esym(1960,5-0-4)
        //lint -esym(1960,5-0-12)
        Raw_UI_To_MMS_Data_struct_type CAN_Message;
        const int BitShiftHighToLow	=	8U;
        //Set up the simple part of the structure
        CAN_Message.CAN_DLC_u8							= ( BYTE )C_SET_CALIBRATION_LENGTH;
        CAN_Message.CAN_ID_u8							= ( BYTE )CONTROLLER_COMMAND;
        CAN_Message.Dest_Subsystem_ID_u8				= ( BYTE )SPI_SUBS_ID_CONTROLLER;
        CAN_Message.Payload_u8[C_SET_CALIBRATION_POS]	= ( BYTE )CC_SET_CALIBRATION;
        //  N.B The Tracking byte position.
        CAN_Message.Payload_u8[C_SET_CALIBRATION_TBPOS]	= ( BYTE )NULL;
        CAN_Message.Payload_u8[C_SET_CALIBRATION_ID]	= ( BYTE )CANID;
        //Split the Data in to two bytes as this is how the controller expects the data to arrive
        CAN_Message.Payload_u8[C_SET_CALIBRATION_HIGH_BYTE]	= ( WORD )Data >> ( int )BitShiftHighToLow;
        CAN_Message.Payload_u8[C_SET_CALIBRATION_LOW_BYTE]	= ( char )Data;

        //These messages are time critical, so set the flag to make sure they go out immediately
        SendMessagesImmediately = TRUE;
        //Send the message
        AddCANMessageToBuffer( ( Raw_UI_To_MMS_Data_struct_type )CAN_Message, ( UINT )C_SET_CALIBRATION_TBPOS, ( const bool )TRUE );

        SendMessagesImmediately = TRUE;
        //Send the message
        AddCANMessageToBuffer( ( Raw_UI_To_MMS_Data_struct_type )CAN_Message, ( UINT )C_SET_CALIBRATION_TBPOS, ( const bool )TRUE );
        //lint +esym(1960,5-0-6)
        //lint +esym(1960,5-0-3)
        //lint +esym(1960,5-0-4)
        //lint +esym(1960,5-0-12)
    }



    /*!***************************************************************************************************
    * \brief Sends an enable calibration message for the CANID requested.
    * \param CANID The canid (defined in msg.h) that should be sent. (the calibration constant to change)
    ******************************************************************************************************/
    void CCommsTransport::SendEnableCalibration( const BYTE CANID )
    {
        Raw_UI_To_MMS_Data_struct_type CAN_Message;

        //Set up the structure
        CAN_Message.CAN_DLC_u8				= ( BYTE )DLENGTH2;
        CAN_Message.CAN_ID_u8				= ( BYTE )CONTROLLER_COMMAND;
        CAN_Message.Dest_Subsystem_ID_u8	= ( BYTE )SPI_SUBS_ID_CONTROLLER;
        CAN_Message.Payload_u8[0]			= ( BYTE )CC_CALIBRATION_ENABLE;
        CAN_Message.Payload_u8[1]			= ( BYTE )CANID;
        //Send the message
        AddCANMessageToBuffer( ( Raw_UI_To_MMS_Data_struct_type )CAN_Message, ( UINT )NOTB, ( const bool )FALSE );
    }

    /*!***************************************************************************************************
    * \brief Maps system id's as defined in globals.h to can ids as defined in msg.h
    * \param SysID The system id value that should be mapped from
    ******************************************************************************************************/
    BYTE CCommsTransport::MapControllerSysIDToCANID( const int SysID ) const
    {
        BYTE TempData = ( BYTE )NULL;

        switch ( SysID )
        {
            //Sensors
            case e_PTR1_GAIN:
                TempData = ( BYTE )CD_CAL_AIR_INPUT_PRESSURE_SCALE;
                break;

            case e_PTR1_OFFSET:
                TempData = ( BYTE )CD_CAL_AIR_INPUT_PRESSURE_OFFSET;
                break;

            case e_PTR2_GAIN:
                TempData = ( BYTE )CD_CAL_O2_INPUT_PRESSURE_SCALE;
                break;

            case e_PTR2_OFFSET:
                TempData = ( BYTE )CD_CAL_O2_INPUT_PRESSURE_OFFSET;
                break;

            case e_PTR3_GAIN:
                TempData = ( BYTE )CD_CAL_FRESH_GAS_PRESSURE_SCALE;
                break;

            case e_PTR3_OFFSET:
                TempData = ( BYTE )CD_CAL_FRESH_GAS_PRESSURE_OFFSET;
                break;

            case e_PTR6_GAIN:
                TempData = ( BYTE )CD_CAL_PROXIMAL_PRESSURE_SCALE;
                break;

            case e_PTR6_OFFSET:
                TempData = ( BYTE )CD_CAL_PROXIMAL_PRESSURE_OFFSET;
                break;

            case e_FS1_GAIN:
                TempData = ( BYTE )CD_CAL_AIR_FLOW_SENSOR_SCALE;
                break;

            case e_FS1_OFFSET:
                TempData = ( BYTE )CD_CAL_AIR_FLOW_SENSOR_OFFSET;
                break;

            case e_FS2_GAIN:
                TempData = ( BYTE )CD_CAL_O2_FLOW_SENSOR_SCALE;
                break;

            case e_FS2_OFFSET:
                TempData = ( BYTE )CD_CAL_O2_FLOW_SENSOR_OFFSET;
                break;

            case e_FS3_GAIN:
                TempData = ( BYTE )CD_CAL_FRESH_GAS_FLOW_SCALE;
                break;

            case e_FS3_OFFSET:
                TempData = ( BYTE )CD_CAL_FRESH_GAS_FLOW_OFFSET;
                break;

            //jets + waveshaping
            case e_MEANJET_GAIN:
                TempData = ( BYTE )CD_CAL_MEAN_JET_SCALE;
                break;

            case e_MEANJET_CMV:
                TempData = ( BYTE )CD_CAL_MEAN_JET_CMV_OFFSET;
                break;

            case e_FORWARDJET_GAIN:
                TempData = ( BYTE )CD_CAL_FORWARD_JET_SCALE;
                break;

            case e_FORWARDJET_HFO:
                TempData = ( BYTE )CD_CAL_FORWARD_JET_HFO_OFFSET;
                break;

            case e_FORWARDJET_CMV:
                TempData = ( BYTE )CD_CAL_FORWARD_JET_OFFSET;
                break;

            case e_REVERSEJET_GAIN:
                TempData = ( BYTE )CD_CAL_REVERSE_JET_SCALE;
                break;

            case e_REVERSEJET_HFO:
                TempData = ( BYTE )CD_CAL_REVERSE_JET_HFO_OFFSET;
                break;

            case e_REVERSEJET_CMV:
                TempData = ( BYTE )CD_CAL_REVERSE_JET_OFFSET;
                break;

            case e_RISEJET_FAST:
                TempData = ( BYTE )CD_CAL_SHORT_RISE_TIME;
                break;

            case e_RISEJET_SLOW:
                TempData = ( BYTE )CD_CAL_LONG_RISE_TIME;
                break;

            case e_RISEJET_BALANCE:
                TempData = ( BYTE )CD_CAL_SHORT_TO_LONG_RISE_FILTER_BALANCE;
                break;

            case e_FALLJET_FAST:
                TempData = ( BYTE )CD_CAL_SHORT_FALL_TIME;
                break;

            case e_FALLJET_SLOW:
                TempData = ( BYTE )CD_CAL_LONG_FALL_TIME;
                break;

            case e_FALLJET_BALANCE:
                TempData = ( BYTE )CD_CAL_SHORT_TO_LONG_FALL_FILTER_BALANCE;
                break;

            case e_VTV_COARSE:
                TempData = ( BYTE )CD_TTV_FAST_ADJUST_FACTOR;
                break;

            case e_VTV_FINE:
                TempData = ( BYTE )CD_TTV_FINE_ADJUST_WINDOW;
                break;

            case e_FG_P:
                TempData = ( BYTE )CD_CAL_FRESH_GAS_P_FACTOR;
                break;

            case e_FG_I:
                TempData = ( BYTE )CD_CAL_FRESH_GAS_I_FACTOR;
                break;

            case e_FG_D:
                TempData = ( BYTE )CD_CAL_FRESH_GAS_D_FACTOR;
                break;

			case e_HFO_P:
				TempData = ( BYTE )CD_CAL_HFO_VTV_P_FACTOR;
                break;

			case e_HFO_I:
				TempData = ( BYTE )CD_CAL_HFO_VTV_I_FACTOR;
                break;

			case e_HFO_D:
				TempData = ( BYTE )CD_CAL_HFO_VTV_D_FACTOR;
                break;

            default:
                break;
        }

        return TempData;
    }



    /*!***************************************************************************************************
    * \brief initiates the comms transport layer
    ******************************************************************************************************/
    void CCommsTransport::InitCommsTransportLayer()
    {
        //Init the SPI
        m_SpiToCanbusManager.InitialiseSPI_CAN_Comms( this );
        m_SpiToCanbusManager.SetCANToDataInterface( m_ICANToData );
    }

    /*!********************************************************************
    * \brief Reset CAN message buffer
    * \details Sets the CAN Message buffer index back to zero
    ***********************************************************************/
    void CCommsTransport::ResetCANMessageBuffer()
    {
        m_uiCANBufferIndex = 0U;
    }


    /*!*********************************************************************************
    * \brief Schedules new CAN messages to the command and response class
    * \details Copies over the buffer for TX by the SPI Thread in command and response.
    ***********************************************************************************/
    void CCommsTransport::SendCANMessageBufferToCmdAndResp()
    {
        m_SpiToCanbusManager.ScheduleNewMessageFromUIThread( m_aCANBinDataBuffer, m_uiCANBufferIndex );
    }

    /*!********************************************************************
    * \brief Set the USB power to On or Off
    * \details This function contains the CAN construct for the USB Power
    * control.
    * \param bON TRUE = ON, FALSE = OFF.
    ***********************************************************************/
    void CCommsTransport::SetExternalUSBPower( const BOOL bON )
    {
        ResetCANMessageBuffer();

        SystemCommunications::Raw_UI_To_MMS_Data_struct_type aCANBinBuffer;
        memset( &aCANBinBuffer, 0, sizeof( SystemCommunications::Raw_UI_To_MMS_Data_struct_type ) );

        //Construct High Priority Alarm Request
        aCANBinBuffer.Dest_Subsystem_ID_u8 = SPI_SUBS_ID_MMS_LOCAL;
        aCANBinBuffer.CAN_ID_u8 = MMS_CAN_ID;
        aCANBinBuffer.CAN_DLC_u8 = 3U;
        aCANBinBuffer.Payload_u8[0] = MMS_LOCAL_CMD_SET_USB_POWER;

        if ( bON == TRUE )
        {
            aCANBinBuffer.Payload_u8[1] = 0x01U;
        }

        else
        {
            aCANBinBuffer.Payload_u8[1] = 0x00U;
        }

        AddCANMessageToBuffer( aCANBinBuffer, 2U, TRUE );
    }

    /*!***************************************************************************************************
    * \brief Created as a wrapper for AddCANMessageToBufferCS. This function continually polls the
    		 CS flag to see if it is safe to send a message. If it is not safe, it will loop until the other
    		thread is finished.
    * \param newCANMessage		The new message
    * \param uiTrackingBytePositionWithinPayload		The position within payload for the tracking byte.
    * \param bRetryEnabled		TRUE to add the CAN Message to the retry list if required.
    * \details Packages a new CAN message into the CAN message buffer and increments the tracking byte.
    ******************************************************************************************************/
    void CCommsTransport::AddCANMessageToBuffer( SystemCommunications::Raw_UI_To_MMS_Data_struct_type & newCANMessage, UINT const uiTrackingBytePositionWithinPayload, const BOOL bRetryEnabled )
    {
        /// If the critical section is successfully entered or the current thread already owns the critical section, the return value is nonzero.
        /// If another thread already owns the critical section, the return value is zero.
        while ( ( bool )TRUE )
        {
            BOOL bOurThreadOwnsCS = TryEnterCriticalSection( &m_CS_Write_To_SPI );

            if ( TRUE == bOurThreadOwnsCS )
            {
                CSAddCANMessageToBuffer( newCANMessage,  uiTrackingBytePositionWithinPayload,  bRetryEnabled );
                LeaveCriticalSection( &m_CS_Write_To_SPI );
                break;
            }

            else
            {
                //retry
            }
        }
    }

    /*!***************************************************************************************************
    * \brief Add a CAN message to CAN Message buffer. This contains a critical section used by mulitiple threads
    * \param newCANMessage		The new message
    * \param uiTrackingBytePositionWithinPayload		The position within payload for the tracking byte.
    * \param bRetryEnabled		TRUE to add the CAN Message to the retry list if required.
    * \details Packages a new CAN message into the CAN message buffer and increments the tracking byte.
    ******************************************************************************************************/
    void CCommsTransport::CSAddCANMessageToBuffer( SystemCommunications::Raw_UI_To_MMS_Data_struct_type & newCANMessage, UINT uiTrackingBytePositionWithinPayload, const BOOL bRetryEnabled )
    {
        if ( uiTrackingBytePositionWithinPayload > 7U )
        {
            uiTrackingBytePositionWithinPayload = 7U;
        }

        if ( MAX_NUM_OF_CAN_PACKETS > m_uiCANBufferIndex )
        {
            //Update Tracking Byte
            if ( uiTrackingBytePositionWithinPayload != ( UINT )NOTB )
            {
                if ( newCANMessage.Dest_Subsystem_ID_u8 == SPI_SUBS_ID_CONTROLLER )
                {
                    m_byTrackingByteNumberController++;
                    newCANMessage.Payload_u8[uiTrackingBytePositionWithinPayload] = ( BYTE )m_byTrackingByteNumberController;
                }

                else if ( newCANMessage.Dest_Subsystem_ID_u8 == SPI_SUBS_ID_MONITOR )
                {
                    m_byTrackingByteNumberMonitor++;
                    newCANMessage.Payload_u8[uiTrackingBytePositionWithinPayload] = ( BYTE )m_byTrackingByteNumberMonitor;
                }

                else if ( newCANMessage.Dest_Subsystem_ID_u8 == SPI_SUBS_ID_MMS_LOCAL )
                {
                    m_byTrackingByteNumberMMS++;
                    newCANMessage.Payload_u8[uiTrackingBytePositionWithinPayload] = ( BYTE )m_byTrackingByteNumberMMS;
                }

                else if ( newCANMessage.Dest_Subsystem_ID_u8 == SPI_SUBS_ID_EST_SENSOR )
                {
                    m_byTrackingByteNumberESMO++;
                    newCANMessage.Payload_u8[uiTrackingBytePositionWithinPayload] = ( BYTE )m_byTrackingByteNumberESMO;
                }

                else
                {
                    //We are not encoding any other subsystems here as of yet.
                    //Future Releases may do this here if required.
                }
            }

            //Set the CAN DLC if it isnt set already
            if ( newCANMessage.CAN_DLC_u8 == ( uint_8_t )0 )
            {
                newCANMessage.CAN_DLC_u8 = ( BYTE )( uiTrackingBytePositionWithinPayload + 1U );
            }


            //Add the Command to the TX Command Buffer
            m_aCANBinDataBuffer[m_uiCANBufferIndex] = newCANMessage;
            m_uiCANBufferIndex++;

            if ( bRetryEnabled == TRUE )
            {
                //Add the Command to the Retry List
                ( void )m_SpiToCanbusManager.GetRetryBuffer().AddMessageToRetryBuffer( newCANMessage, uiTrackingBytePositionWithinPayload );
            }
        }

        else//(m_uiCANBufferIndex > MAX_NUM_OF_CAN_PACKETS) //reset - JD 29/03/2017 added send and reset to ensure messages are realiably sent and system can continue if buffers overflow.
        {

            SendCANMessageBufferToCmdAndResp();
            ResetCANMessageBuffer();
            SendMessagesImmediately = FALSE;
        }

        if ( SendMessagesImmediately == TRUE )
        {
            SendCANMessageBufferToCmdAndResp();
            ResetCANMessageBuffer();
            SendMessagesImmediately = FALSE;
        }

    }

    static	unsigned int guiCallbackCount = 0U;
    /*!********************************************************************
    * \brief Private method to reset data in memory for new collection
    * \details In the SPI thread.
    ***********************************************************************/
    void CCommsTransport::RecvSpiResponse( const SystemCommunications::eSpiResponseType eResp, const BYTE* const ptrData, const unsigned int uiDataLen )
    {
#if ( COMMS_TRANSPORT_CALLBACK_DEBUG == TRUE )
        DEBUGMSG( COMMS_TRANSPORT_CALLBACK_DEBUG, ( L"RecvSpiResponse: ID %02X (%d) : ", ( int )eResp, ( int )eResp ) );

        for ( unsigned int uiIndex = 0U; uiIndex < uiDataLen; uiIndex++ )
        {
            DEBUGMSG( COMMS_TRANSPORT_CALLBACK_DEBUG, ( L"%02X:", ptrData[ uiIndex ] ) );
        }

        DEBUGMSG( COMMS_TRANSPORT_CALLBACK_DEBUG, ( TEXT( "\r\n" ) ) );
#endif
        guiCallbackCount++;

        if ( guiCallbackCount > 5U )
        {
            guiCallbackCount = 0U;
        }
    }

    void CCommsTransport::RequestUpdateController()
    {
        Raw_UI_To_MMS_Data_struct_type CAN_Message;

        CAN_Message.CAN_DLC_u8				= ( BYTE )DLENGTH2;
        CAN_Message.CAN_ID_u8				= ( BYTE )CONTROLLER_COMMAND;
        CAN_Message.Dest_Subsystem_ID_u8	= ( BYTE )SPI_SUBS_ID_CONTROLLER;
        CAN_Message.Payload_u8[0]			= ( BYTE )CC_GET_CONFIG;

        //Go through each of the calibration messages and request an update
        for ( int i = NULL; i < TOTAL_CONTROLLER_UPDATE_REQUEST_COMMANDS ; i++ )
        {
            CAN_Message.Payload_u8[1]			= Controller_Update_Request_Commands[i];
            AddCANMessageToBuffer( ( Raw_UI_To_MMS_Data_struct_type )CAN_Message, ( UINT )TBPOS3, ( BOOL )TRUE );
        }
    }

    void CCommsTransport::RequestUpdateMonitor()
    {
        Raw_UI_To_MMS_Data_struct_type CAN_Message;

        CAN_Message.CAN_DLC_u8				= ( BYTE )DLENGTH2;
        CAN_Message.CAN_ID_u8				= ( BYTE )MONITOR_COMMAND;
        CAN_Message.Dest_Subsystem_ID_u8	= ( BYTE )SPI_SUBS_ID_MONITOR;
        CAN_Message.Payload_u8[0]			= ( BYTE )MC_GET_CONFIG;

        //Go through each of the calibration messages and request an update
        for ( int i = NULL; i < TOTAL_MONITOR_UPDATE_REQUEST_COMMANDS ; i++ )
        {
            CAN_Message.Payload_u8[1]			= ( BYTE )Monitor_Update_Request_Commands[i];
            AddCANMessageToBuffer( ( Raw_UI_To_MMS_Data_struct_type )CAN_Message, ( UINT )NOTB, ( BOOL )TRUE );
        }
    }
    void CCommsTransport::RequestUpdateESMO()
    {
        Raw_UI_To_MMS_Data_struct_type CAN_Message;

        CAN_Message.CAN_DLC_u8				= ( BYTE )DLENGTH2;
        CAN_Message.CAN_ID_u8				= ( BYTE )EXTERNAL_SENSOR_COMMAND;
        CAN_Message.Dest_Subsystem_ID_u8	= ( BYTE )SPI_SUBS_ID_MONITOR;
        CAN_Message.Payload_u8[0]			= ( BYTE )ESMODefinitions::ESC_GET_CONFIG;

        //Go through each of the calibration messages and request an update
        for ( int i = NULL; i < TOTAL_ETCO2_ESMO_UPDATE_REQUEST_COMMANDS ; i++ )
        {
            CAN_Message.Payload_u8[1]			= ( BYTE )ESMO_ETCO2_Update_Request_Commands[i];
            AddCANMessageToBuffer( ( Raw_UI_To_MMS_Data_struct_type )CAN_Message, ( UINT )NOTB, ( BOOL )TRUE );
        }
    }

    /*!********************************************************************
     * \brief Used by the request update thread to initialise all of the values
    		 this is important as it keeps the use of the CAN infastructure down.
    		 (Once values such as gain and offset for a sensor are initialised they
    		  will not change unless the UI changes them and so dont need to be
    		 continually requested
     * \param lpdwThreadParam Used so that the function can access member functions
    		  of the CommsTransport class
    ***********************************************************************/
    void CCommsTransport::RequestInitialiseControllerModel( const LPVOID lpdwThreadParam ) const
    {
        CCommsTransport* pCCommsTransport = ( CCommsTransport* )lpdwThreadParam;
        Raw_UI_To_MMS_Data_struct_type CAN_Message;

        CAN_Message.CAN_DLC_u8				= ( BYTE )DLENGTH2;
        CAN_Message.CAN_ID_u8				= ( BYTE )CONTROLLER_COMMAND;
        CAN_Message.Dest_Subsystem_ID_u8	= ( BYTE )SPI_SUBS_ID_CONTROLLER;
        CAN_Message.Payload_u8[0]			= ( BYTE )CC_GET_CONFIG;

        //Go through each of the calibration messages and request an update
        for ( BYTE i = ( BYTE )START_CAL_MESSAGES; i < ( BYTE )END_CAL_MESSAGES ; i++ )
        {
            CAN_Message.Payload_u8[1]			= ( BYTE )i;
            pCCommsTransport->AddCANMessageToBuffer( ( Raw_UI_To_MMS_Data_struct_type )CAN_Message, ( UINT )TBPOS3, ( BOOL )TRUE );
        }

    }

	
	void CCommsTransport::RequestPneumaticType(void) 
	{
		Raw_UI_To_MMS_Data_struct_type CAN_Message;  	
	
        CAN_Message.CAN_DLC_u8				= ( BYTE )DLENGTH2;
        CAN_Message.CAN_ID_u8				= ( BYTE )CONTROLLER_COMMAND;
        CAN_Message.Dest_Subsystem_ID_u8	= ( BYTE )SPI_SUBS_ID_CONTROLLER;

        CAN_Message.Payload_u8[0]			= ( BYTE )CC_GET_CONFIG;
        CAN_Message.Payload_u8[1]			= ( BYTE )CD_CAL_PNEUMATIC_MODULE_ID;

        AddCANMessageToBuffer( ( Raw_UI_To_MMS_Data_struct_type )CAN_Message, ( UINT )TBPOS3, ( const bool )TRUE );
	}

	void CCommsTransport::SetPneumaticType (const int PneumaticsType)
	{	
		Raw_UI_To_MMS_Data_struct_type CAN_Message;

        //Set up the structure
        CAN_Message.CAN_DLC_u8				= ( BYTE )DLENGTH2;
        CAN_Message.CAN_ID_u8				= ( BYTE )CONTROLLER_COMMAND;
        CAN_Message.Dest_Subsystem_ID_u8	= ( BYTE )SPI_SUBS_ID_CONTROLLER;
        CAN_Message.Payload_u8[0]			= ( BYTE )CC_CALIBRATION_ENABLE;
        CAN_Message.Payload_u8[1]			= ( BYTE )CD_CAL_PNEUMATIC_MODULE_ID;	

		SendMessagesImmediately = TRUE;
        //Send the message
        AddCANMessageToBuffer( ( Raw_UI_To_MMS_Data_struct_type )CAN_Message, ( UINT )NOTB, ( const bool )FALSE );
		
        //Set up the structure
        CAN_Message.CAN_DLC_u8				= ( BYTE )C_SET_CALIBRATION_LENGTH;
        CAN_Message.CAN_ID_u8				= ( BYTE )CONTROLLER_COMMAND;
        CAN_Message.Dest_Subsystem_ID_u8	= ( BYTE )SPI_SUBS_ID_CONTROLLER;
        CAN_Message.Payload_u8[0]			= ( BYTE )CC_SET_CALIBRATION;
        CAN_Message.Payload_u8[1]			= ( BYTE )0x7B;
		CAN_Message.Payload_u8[2]			= ( BYTE )CD_CAL_PNEUMATIC_MODULE_ID;
		CAN_Message.Payload_u8[3]			= ( BYTE )PneumaticsType;
		CAN_Message.Payload_u8[4]			= ( BYTE )NULL;

		SendMessagesImmediately = TRUE;
        //Send the message
        AddCANMessageToBuffer( ( Raw_UI_To_MMS_Data_struct_type )CAN_Message, ( UINT )TBPOS2, ( const bool )TRUE );
	}

    void CCommsTransport::RequestInitialiseESMOModel( const LPVOID lpdwThreadParam ) const
    {
        CCommsTransport* pCCommsTransport = ( CCommsTransport* )lpdwThreadParam;
        Raw_UI_To_MMS_Data_struct_type CAN_Message;

        CAN_Message.CAN_DLC_u8				= ( BYTE )DLENGTH2;
        CAN_Message.CAN_ID_u8				= ( BYTE )EXTERNAL_SENSOR_COMMAND;
        CAN_Message.Dest_Subsystem_ID_u8	= ( BYTE )SPI_SUBS_ID_MONITOR;
        CAN_Message.Payload_u8[0]			= ( BYTE )ESMODefinitions::ESC_GET_CONFIG;

        //Go through each of the calibration messages and request an update
        for ( int i = NULL; i < TOTAL_ETCO2_ESMO_UPDATE_REQUEST_COMMANDS ; i++ )
        {
            CAN_Message.Payload_u8[1]			= ( BYTE )ESMO_ETCO2_Update_Request_Commands[i];
            pCCommsTransport->AddCANMessageToBuffer( ( Raw_UI_To_MMS_Data_struct_type )CAN_Message, ( UINT )NOTB, ( BOOL )TRUE );
        }

    }


    /*!***************************************************************************************************
    * \brief Sets a Controller VTV Coarse Adjustment constant to the data passed to it
    * \param SysID VTV Coarse Adjustment control ID
    * \param Data The new value that the calibration constant should be set to
    ******************************************************************************************************/
    void CCommsTransport::SendVTV_CoarseMessage( const int_32_t Data )
    {
        Raw_UI_To_MMS_Data_struct_type CAN_Message;

        CAN_Message.CAN_DLC_u8				= ( BYTE )DLENGTH3;
        CAN_Message.CAN_ID_u8				= ( BYTE )CONTROLLER_COMMAND;
        CAN_Message.Dest_Subsystem_ID_u8	= ( BYTE )SPI_SUBS_ID_CONTROLLER;
        CAN_Message.Payload_u8[0]			= ( BYTE )CC_SET_TTV_FAST_ADJUST_FACTOR;
        CAN_Message.Payload_u8[1]			= ( BYTE )Data;

        AddCANMessageToBuffer( ( Raw_UI_To_MMS_Data_struct_type )CAN_Message, ( UINT )TBPOS3, ( const bool )TRUE );
    }


    /*!***************************************************************************************************
    * \brief Sets a Controller VTV Fine Adjustment constant to the data passed to it
    * \param SysID VTV Fine Adjustment control ID
    * \param Data The new value that the calibration constant should be set to
    ******************************************************************************************************/
    void CCommsTransport::SendVTV_FineMessage( const int_32_t Data )
    {
        Raw_UI_To_MMS_Data_struct_type CAN_Message;

        CAN_Message.CAN_DLC_u8				= ( BYTE )DLENGTH3;
        CAN_Message.CAN_ID_u8				= ( BYTE )CONTROLLER_COMMAND;
        CAN_Message.Dest_Subsystem_ID_u8	= ( BYTE )SPI_SUBS_ID_CONTROLLER;
        CAN_Message.Payload_u8[0]			= ( BYTE )CC_SET_TTV_FINE_ADJUST_WINDOW;
        CAN_Message.Payload_u8[1]			= ( BYTE )Data;

        AddCANMessageToBuffer( ( Raw_UI_To_MMS_Data_struct_type )CAN_Message, ( UINT )TBPOS3, ( const bool )TRUE );
    }


    /*!***************************************************************************************************
    * \brief Sets a Controller Max Leak Compensation constant to the data passed to it
    * \param SysID Max Leak Compensation control ID
    * \param Data The new value that the calibration constant should be set to
    ******************************************************************************************************/
    void CCommsTransport::SendMax_LeakCompensationMessage( const int_32_t Data )
    {
        Raw_UI_To_MMS_Data_struct_type CAN_Message;

        CAN_Message.CAN_DLC_u8				= ( BYTE )DLENGTH3;
        CAN_Message.CAN_ID_u8				= ( BYTE )CONTROLLER_COMMAND;
        CAN_Message.Dest_Subsystem_ID_u8	= ( BYTE )SPI_SUBS_ID_CONTROLLER;
        CAN_Message.Payload_u8[0]			= ( BYTE )CC_SET_VOLUME_CONTROL_LEAK_COMPENSATION;
        CAN_Message.Payload_u8[1]			= ( BYTE )Data;

        AddCANMessageToBuffer( ( Raw_UI_To_MMS_Data_struct_type )CAN_Message, ( UINT )TBPOS3, ( const bool )TRUE );
    }

}//namespace
