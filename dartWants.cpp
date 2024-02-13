// dart Tx
        //Loop Until Told to Quit by the Main Application
        while (WaitForSingleObject(pCallingClass->GethQuitEvent(), 0U) == (DWORD)WAIT_TIMEOUT)
        {
            //Check to see if the UI CAN Message array is marked for TX.
            dwWaitObjectResultFromSetVentMode = WaitForSingleObject(pCallingClass->GethSetVentModeEvent(), 0U);
            if (dwWaitObjectResultFromSetVentMode == (DWORD)WAIT_OBJECT_0)
            {
                //Clear the Event
                (void)ResetEvent(pCallingClass->GethSetVentModeEvent());
                
                //Swap the buffer to the Command Buffer
                //Get CAN Bin Message Array
                paCANBinMessage = pCallingClass->Get_aCANBinDataFrameBufferUI();
                //Populate ASCII Hex Buffer for Tx.
                byNumOfPopulatedEntrys = pCallingClass->GetuiNumOfPopulatedEntrysUI();

                //Construct outward packet
                (void)SPI_Construct_UI_To_MMS_Packet(paCANBinMessage, byNumOfPopulatedEntrys, g_SPI_UI_To_MMS_Packet);
                      

                //Compute CRC
                pCallingClass->GenerateCRCandAddToOutboundPacket(&g_SPI_UI_To_MMS_Packet);

                ioData.Buffer = &g_SPI_UI_To_MMS_Packet;
            }

// -----------------------------------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------------------

// dart Rx             
  
				sizeof(PCH_SPI_RW),
				g_SPI_MMS_To_UI_Packet_Byte_Array ,   // Rx, Tx buffer,  Outer frame
				sizeof(SPI_A_Outer_struct_Ascii_type), 
				&wRetBytes, 
				NULL);
            // GethSPIPort(); is above. get spi terminal 
			//lint +esym(1960, 5-2-12)

            if (dwWaitObjectResultFromSetVentMode == (DWORD)WAIT_OBJECT_0)
            {
                //Inform Command & Response Class that the buffer has been copied over
                (void)SetEvent(pCallingClass->GethTXBufferSentEvent());
            }
            
            if ((FALSE == bRetValue) || (wRetBytes == 0U))
            {
                //In debug mode, show the condition in the output console.
                DEBUGMSG(SPI_DEBUG, (L"SPI: IOCTL_SPI_READ has failed, check SPI driver. Bytes return from Read [%d]\r\n", wRetBytes));
            }
            else
            {
				// g_SPI_MMS_To_UI_Packet_Byte_Array[] is Rx of spi
                // Find the Start Delimiter 
                WORD wSDP_Start_Del_Idx_u16 = (WORD) (0U);                          
                BYTE bySDP_x_u8 = g_SPI_MMS_To_UI_Packet_Byte_Array[wSDP_Start_Del_Idx_u16];
        
                while((bySDP_x_u8 != SPI_OUTER_START_DELIMITER_CHAR) && (wSDP_Start_Del_Idx_u16 <= SPI_A_O_PREAMBLE_SIZE))
                {
                    wSDP_Start_Del_Idx_u16++;
                    DWORD* pdwRawPacket = (DWORD*)(&g_SPI_MMS_To_UI_Packet_Byte_Array[wSDP_Start_Del_Idx_u16]);
                    //lint +e927
                    
                    dwSCM_CRC_u32 = pCallingClass->m_CFastCRC.Crc32FastBlock_fn(0xFFFFFFFFUL, (SPI_RAW_CRC_POSITION / 4U), pdwRawPacket);
                    
                    //Extract CRC from Packet
                    DWORD dwRawCRC32;
                    dwRawCRC32  = (DWORD)(pCallingClass->ASCIItoBIN_BYTE(g_SPI_MMS_To_UI_Packet.O_CRC[0U])) << 28U;
                    dwRawCRC32 |= (DWORD)(pCallingClass->ASCIItoBIN_BYTE(g_SPI_MMS_To_UI_Packet.O_CRC[1U])) << 24U;
                    dwRawCRC32 |= (DWORD)(pCallingClass->ASCIItoBIN_BYTE(g_SPI_MMS_To_UI_Packet.O_CRC[2U])) << 20U;
                    dwRawCRC32 |= (DWORD)(pCallingClass->ASCIItoBIN_BYTE(g_SPI_MMS_To_UI_Packet.O_CRC[3U])) << 16U;
                    dwRawCRC32 |= (DWORD)(pCallingClass->ASCIItoBIN_BYTE(g_SPI_MMS_To_UI_Packet.O_CRC[4U])) << 12U;
                    dwRawCRC32 |= (DWORD)(pCallingClass->ASCIItoBIN_BYTE(g_SPI_MMS_To_UI_Packet.O_CRC[5U])) << 8U;
                    dwRawCRC32 |= (DWORD)(pCallingClass->ASCIItoBIN_BYTE(g_SPI_MMS_To_UI_Packet.O_CRC[6U])) << 4U;
                    dwRawCRC32 |= (DWORD)pCallingClass->ASCIItoBIN_BYTE(g_SPI_MMS_To_UI_Packet.O_CRC[7U]);
                    
                    //Compare CRCs
                    if (dwSCM_CRC_u32 != dwRawCRC32)
                    {
                        //CRC has failed
                        pCallingClass->IncrementSPIFailureCountAndRaiseAlarm();
                    }
                    else
                    {
                        //lint -esym(1960, 5-2-12) SPI_Decode_UI_To_MMS_Packet_fn = SPI_Decode_to_UI_from_MMS_Packet_fn
                        BOOL bSPIOK = SPI_Decode_UI_To_MMS_Packet_fn( g_SPI_MMS_To_UI_Packet, aRXCANBinMessage, NumOfRxPackets);
                        // decode from MMS to GUI 
                        // decode from outer frame into the inner frame
                        //lint +esym(1960, 5-2-12)

                        if (FALSE == bSPIOK)
                        {
#if SPI_DEBUG
                            iDecodeError++;
                            DEBUGMSG(SPI_DEBUG, (L"SPI: Packet Decode Failure, Decode Count %d\r\n", iDecodeError));
#endif
                            (void)MainApp::g_pC6000UIApp->GetDebugLog().AddEventUsingCompiler( cstrFName, __LINE__);
                        }
          





           //Get Current Timebase for SpO2 display
                                TstructWavData.TBasePres = pCallingClass->GetGraphControls()[UIControls::CGraphControl::eGraphPressure].GetCurrentTimeBase();
                                TstructWavData.TBaseFlow = pCallingClass->GetGraphControls()[UIControls::CGraphControl::eGraphFlow].GetCurrentTimeBase();
                                TstructWavData.TBaseVol = pCallingClass->GetGraphControls()[UIControls::CGraphControl::eGraphVolume].GetCurrentTimeBase();
							/*	TstructWavData.TBaseESMO = pCallingClass->GetGraphControls()[UIControls::CGraphControl::eGraphCO2].GetCurrentTimeBase();
								TstructWavData.TBaseESMO = pCallingClass->GetGraphControls()[UIControls::CGraphControl::eGraphCO2Trend].GetCurrentTimeBase();	// 3816c
								TstructWavData.TBaseESMO = pCallingClass->GetGraphControls()[UIControls::CGraphControl::eGraphSpO2].GetCurrentTimeBase();
								*/
                                TstructRealtimeData.iLengthPressure	= 0;
                                TstructRealtimeData.iLengthFlow		= 0;
                                TstructRealtimeData.iLengthVolume	= 0;
                                TstructRealtimeData.iLengthESMO		= 0; // CO2