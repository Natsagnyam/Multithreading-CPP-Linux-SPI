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
        