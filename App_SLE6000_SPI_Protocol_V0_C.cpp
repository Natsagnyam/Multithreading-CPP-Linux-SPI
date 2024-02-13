/*!********************************************************************
 * \file
 * \brief This file contains the functionality for determining the SPI Protocol
 * helper functions.
 * \details
 * (C)opyright 2015 SLE Ltd
 *
 * SVN Revision: $Revision: 1438 $
 *
 * Modification Record 
 * -------------------
 * 
 *  Date        | By             | Description
 *  ----------: | :------------: | :-------- 
 *  21-Sep-2014 | R.Greenslade   | Initial build.
  ***********************************************************************/

/* ************************************************************************* */
/* #Includes */
/* ************************************************************************* */


#include "inc/App_SLE6000_SPI_Protocol_V0_C.h"

namespace CommandAndResponse
{



// *************************************************************************
// Public Variables
BYTE  SPI_UI_Status_Flag_Byte      = (BYTE) (0U);

/* ************************************************************************* */
/* #Defines */
/* ************************************************************************* */
/* Private define ------------------------------------------------------------*/
#define SP_INVALID_ASCII_CHAR_RETURN_CODE_u8 (BYTE) (0xFFU)
#define SP_CONVERSION_SUCCESS__RETURN_CODE_u8 (BYTE) (0x00U)

/* Private macro -------------------------------------------------------------*/

/* Private types -------------------------------------------------------------*/
	typedef enum { 
		SC_PREAMBLE = 0U,
		SC_OUTER_START, 
		SC_OUTER_SEQ,
		SC_STATUS_START,
		SC_STATUS_PKT_CNT,
		SC_STATUS_UI_FLAGS,
		SC_STATUS_MMS_ACK_FLAGS,
		SC_STATUS_PADDING,
		SC_STATUS_END,
		SC_INNER_PACKETS,
		SC_OUTER_CRC,
		SC_OUTER_PADDING,
		SC_OUTER_END,
	  SC_OUTER_COMPLETE
	} SCU_Outer_Progress_Enum_type;
	
	typedef enum { 
		SC_INNER_START = 0U, 
		SC_INNER_SEQ,
		SC_INNER_SRC,
		SC_INNER_DEST,
		SC_INNER_ID,
		SC_INNER_DLC,
		SC_INNER_PAYLOAD,
		SC_INNER_PADDING,
		SC_INNER_END,
		SC_INNER_COMPLETE
		} SCU_Inner_Progress_Enum_type;


/* Private variables ---------------------------------------------------------*/

WORD SPI_O_Sequence_Counter_u16 = (WORD) (0U);

/*!******************************************************************** 
* \brief Converts supplied Ascii value to binary 
* \details Legal Input range is ('0' - '9' & 'A' - 'F')
* \param SP_ACB8_Ascii_Byte_u8 the ASCII value to be converted.
* \return BYTE binary representation of supplied ASCII value. 
*                   Range 0x00-0x0F or 0xFF if invalid
***********************************************************************/
BYTE SPI_ASCII_Char_To_Bin_u8_fn(const BYTE SP_ACB8_Ascii_Byte_u8)
{
    BYTE SPI_ACB8_RetVal_u8 = (BYTE)  (0U);
    
    if(((BYTE)'0' <= SP_ACB8_Ascii_Byte_u8 ) && ((BYTE)'9' >= SP_ACB8_Ascii_Byte_u8 ))
    {
       SPI_ACB8_RetVal_u8 =  (SP_ACB8_Ascii_Byte_u8 - (BYTE)'0');
    }
    else if((((BYTE)'a') <= SP_ACB8_Ascii_Byte_u8 ) && (((BYTE)'f') >= SP_ACB8_Ascii_Byte_u8 ))
    {
       SPI_ACB8_RetVal_u8 =  (SP_ACB8_Ascii_Byte_u8 - (BYTE)'a');
       SPI_ACB8_RetVal_u8 += 10U;
    }
    else if(((BYTE)'A' <= SP_ACB8_Ascii_Byte_u8 ) && ((BYTE)'F' >= SP_ACB8_Ascii_Byte_u8 ))
    {
       SPI_ACB8_RetVal_u8 =  (SP_ACB8_Ascii_Byte_u8 - (BYTE)'A');
       SPI_ACB8_RetVal_u8 += 10U;
    }
    else
    {
       SPI_ACB8_RetVal_u8 = SP_INVALID_ASCII_CHAR_RETURN_CODE_u8;
    }
    return(SPI_ACB8_RetVal_u8);
}

/*!******************************************************************** 
* \brief Converts supplied Binary value to ASCII HEX  
* \details Legal Input range is 0 - 15 (0x00 - 0x0F)
* \param SP_Bin_u8_To_Convert the binary value to be converted.
* \return BYTE ('0' - '9' & 'A' - 'F') 
*                   or SP_INVALID_ASCII_CHAR_RETURN_CODE_u8 (0xFF) if invalid
***********************************************************************/
BYTE SPI_Bin_u8_To_ASCII_Char_fn(const BYTE SP_Bin_u8_To_Convert)
{
	BYTE SP_Retval_u8 = (BYTE) (0U);
	
	if(9U >= SP_Bin_u8_To_Convert)
	{
		SP_Retval_u8 = SP_Bin_u8_To_Convert + (BYTE) ('0');
	}
	else if(15U >= SP_Bin_u8_To_Convert)
	{
		SP_Retval_u8 = SP_Bin_u8_To_Convert -10U;
		SP_Retval_u8 += (BYTE) ('A');
	}
	else
	{
		SP_Retval_u8 = SP_INVALID_ASCII_CHAR_RETURN_CODE_u8;
	}
	return(SP_Retval_u8);
}

/*!******************************************************************** 
* \brief Converts supplied Binary value to ASCII HEX pair in u16
* \details Legal Input range is 0 - 255 (0x00 - 0xFF)
* \param SPP_Bin_u8_To_Convert the binary value to be converted.
* \param SPP_Result_u8_ptr Pointer to where to place result
* \return BYTE  SP_CONVERSION_SUCCESS__RETURN_CODE_u8
*                       or  SP_INVALID_ASCII_CHAR_RETURN_CODE_u8
***********************************************************************/
BYTE SPI_Bin_u8_To_ASCII_Pair_fn(const BYTE SPP_Bin_u8_To_Convert, BYTE* const SPP_Result_u8_ptr)
{
	BYTE SPP_x_u8              = ((BYTE) (0U));
	BYTE SPP_r_u8              = ((BYTE) (0U));
	BYTE SPP_loop_u8           = ((BYTE) (0U));
	BYTE SPP_Result_Success_u8 = SP_CONVERSION_SUCCESS__RETURN_CODE_u8; // Until proven otherwise
	const BYTE SPP_Shift_Table[2] = { 4U, 0U };               
	
	while( ((SPP_loop_u8 < ((BYTE)(2U))) && (SP_INVALID_ASCII_CHAR_RETURN_CODE_u8 != SPP_Result_Success_u8)) )
	{
		SPP_x_u8 =   SPP_Bin_u8_To_Convert;
		SPP_x_u8 >>= SPP_Shift_Table[SPP_loop_u8];
		SPP_x_u8 &=  ((BYTE) (0x0FU));
	    SPP_r_u8 = SPI_Bin_u8_To_ASCII_Char_fn(SPP_x_u8); 
	
	    if( SP_INVALID_ASCII_CHAR_RETURN_CODE_u8 != SPP_r_u8)
		{
			SPP_Result_u8_ptr[SPP_loop_u8] = SPP_r_u8;  
		}
		else
		{
			SPP_Result_u8_ptr[SPP_loop_u8] = 0U;
			SPP_Result_Success_u8 =	SP_INVALID_ASCII_CHAR_RETURN_CODE_u8;
		}
        
		SPP_loop_u8++;
	}  // end for
	
	return(SPP_Result_Success_u8);
}

/*****************************************************************************
** Function name:		SPI_Bin_u8_To_ASCII_Pair_fn
**
** Description:  		Converts supplied Binary value to ASCII HEX pair in u16
**                   
** Inputs:			    Legal Input range is 0 - 255 (0x00 - 0xFF)
**                  Pointer to where to place result
** Returned value:	BYTE  SP_CONVERSION_SUCCESS__RETURN_CODE_u8
**                       or  SP_INVALID_ASCII_CHAR_RETURN_CODE_u8
**                   
** 
*****************************************************************************/

/*!******************************************************************** 
* \brief Converts supplied Binary value to ASCII HEX pair in u16
* \details Legal Input range is 0 - 65535 (0x00 - 0xFFFF)
* \param SPQ_Bin_u16_To_Convert the binary value to be converted.
* \param SPQ_Result_u8_ptr Pointer to where to place result
* \return BYTE  SP_CONVERSION_SUCCESS__RETURN_CODE_u8
*                       or  SP_INVALID_ASCII_CHAR_RETURN_CODE_u8
***********************************************************************/
BYTE SPI_Bin_u16_To_ASCII_Quad_fn(const WORD SPQ_Bin_u16_To_Convert, BYTE* const SPQ_Result_u8_ptr)
{
	WORD SPQ_x_u16              = ((WORD) (0U));
	BYTE  SPQ_x_u8 				= ((BYTE)  (0U));
	BYTE  SPQ_r_u8               = ((BYTE)  (0U));
	BYTE  SPQ_loop_u8            = ((BYTE)  (0U));
	BYTE  SPQ_Result_Success_u8  = SP_CONVERSION_SUCCESS__RETURN_CODE_u8; // Until proven otherwise
	const BYTE SPQ_Shift_Table[4U] = { 12U, 8U, 4U, 0U };               
	
	
	
	while(((SPQ_loop_u8 < ((BYTE) (4U))) && (SP_INVALID_ASCII_CHAR_RETURN_CODE_u8 != SPQ_Result_Success_u8)))
	{
		SPQ_x_u16 =   SPQ_Bin_u16_To_Convert;
		SPQ_x_u16 >>= SPQ_Shift_Table[SPQ_loop_u8];
		SPQ_x_u16 &=  ((WORD) (0x0FU));
		
		SPQ_x_u8  = (BYTE) (SPQ_x_u16);
	    SPQ_r_u8   = SPI_Bin_u8_To_ASCII_Char_fn(SPQ_x_u8); 
	
	    if( SP_INVALID_ASCII_CHAR_RETURN_CODE_u8 != SPQ_r_u8)
		{
			SPQ_Result_u8_ptr[SPQ_loop_u8] = SPQ_r_u8;  
		}
		else
		{
			SPQ_Result_u8_ptr[SPQ_loop_u8] = 0U;
			SPQ_Result_Success_u8 =	SP_INVALID_ASCII_CHAR_RETURN_CODE_u8;
		}

		SPQ_loop_u8++;
	}  // end for
	
	return(SPQ_Result_Success_u8);
}

/*!******************************************************************** 
* \brief Constructs an entire SPI packet from UI to MMS  
* \details Contains the state machine for forming an MMS packet to be TXed over the SPI.
* \param Raw_UI_To_MMS_Data_struct_ptr Pointer to Data to be encoded
* \param Num_Valid_Data_Items_u8 Number of populated entrys
* \param SPI_UI_To_MMS_Packet The resultant MMS packet.
* \return BOOL  TRUE = Success, FALSE = Failure.
***********************************************************************/
BOOL SPI_Construct_UI_To_MMS_Packet(const Raw_UI_To_MMS_Data_struct_type* const Raw_UI_To_MMS_Data_struct_ptr, const BYTE Num_Valid_Data_Items_u8, SPI_A_Outer_struct_Ascii_type& SPI_UI_To_MMS_Packet)
{
	BOOL     SCU_Return_Success_b;
	BYTE  SCU_Loop_u8        = ((BYTE)  (0U));
	BYTE  SCU_I_Pkt_Count_u8 = ((BYTE)  (0U));
	BYTE  SCU_x_u8           = ((BYTE)  (0U));
	BYTE  SCU_Conversion_Success_u8 = SP_CONVERSION_SUCCESS__RETURN_CODE_u8;

	SCU_Outer_Progress_Enum_type SCU_Outer_Construction_Progress = SC_PREAMBLE;
	
	SCU_Inner_Progress_Enum_type SCU_Inner_Progress = SC_INNER_START;
	
		
	while((SC_OUTER_COMPLETE > SCU_Outer_Construction_Progress) && (SP_CONVERSION_SUCCESS__RETURN_CODE_u8 == SCU_Conversion_Success_u8))
	{
		switch(SCU_Outer_Construction_Progress)
		{
			case SC_PREAMBLE:
				for( SCU_Loop_u8 = (BYTE) (0U); SCU_Loop_u8 < SPI_A_O_PREAMBLE_SIZE; SCU_Loop_u8++)
				{
					SPI_UI_To_MMS_Packet.O_Preamble[SCU_Loop_u8]  = SPI_OUTER_PREAMBLE_CHAR;
				}
				SCU_Outer_Construction_Progress = SC_OUTER_START;
			break;

			case SC_OUTER_START:
					SPI_UI_To_MMS_Packet.O_Start_Delimiter = SPI_OUTER_START_DELIMITER_CHAR;
					SCU_Outer_Construction_Progress = SC_OUTER_SEQ;
			break;
		
			case SC_OUTER_SEQ:
				 SCU_Conversion_Success_u8 = SPI_Bin_u16_To_ASCII_Quad_fn( 
			                 SPI_O_Sequence_Counter_u16, &(SPI_UI_To_MMS_Packet.O_Sequence_Count[0]));
					 SCU_Outer_Construction_Progress = SC_STATUS_START;
			break;
			
			case 	SC_STATUS_START:
			  	SPI_UI_To_MMS_Packet.O_Status_Packet.S_Start_Delimiter = SPI_STATUS_START_DELIMITER_CHAR;
		  		SCU_Outer_Construction_Progress = SC_STATUS_PKT_CNT;
			break;   
			
			case SC_STATUS_PKT_CNT:
					SCU_Conversion_Success_u8 = SPI_Bin_u8_To_ASCII_Pair_fn( 
			                Num_Valid_Data_Items_u8 ,&(SPI_UI_To_MMS_Packet.O_Status_Packet.S_Packet_Count[0]) );
	  			SCU_Outer_Construction_Progress = SC_STATUS_UI_FLAGS; 
			break;   
	
			case SC_STATUS_UI_FLAGS:
				SCU_Conversion_Success_u8 = SPI_Bin_u8_To_ASCII_Pair_fn(
			                SPI_UI_Status_Flag_Byte , &(SPI_UI_To_MMS_Packet.O_Status_Packet.S_Status_Flags[0]));  
			
				SCU_Outer_Construction_Progress = SC_STATUS_MMS_ACK_FLAGS;
			break;   
			
			case SC_STATUS_MMS_ACK_FLAGS:
			  SCU_Conversion_Success_u8 = SPI_Bin_u8_To_ASCII_Pair_fn(
			                SPI_UI_Status_Flag_Byte ,&(SPI_UI_To_MMS_Packet.O_Status_Packet.S_Status_Ack_Flags[0]));
			  SCU_Outer_Construction_Progress = SC_STATUS_PADDING;
			break;   
			
			case SC_STATUS_PADDING:
				for(SCU_Loop_u8 = 0U; SPI_A_S__PADDING_SIZE > SCU_Loop_u8; SCU_Loop_u8++)	
				{
					SPI_UI_To_MMS_Packet.O_Status_Packet.S_Padding[SCU_Loop_u8] = SPI_STATUS_PADDING_CHAR;
				}
				SCU_Outer_Construction_Progress = SC_STATUS_END;
			break;   
			
			case SC_STATUS_END:
				 SPI_UI_To_MMS_Packet.O_Status_Packet.S_End_Delimiter = SPI_OUTER_END_DELIMITER_CHAR;
				 SCU_Outer_Construction_Progress = SC_INNER_PACKETS;
			break;   
			
			case SC_INNER_PACKETS:
				// ** Construct the inner packets
			  SCU_I_Pkt_Count_u8 = ((BYTE) (0U));
			
			
			  while(SCU_I_Pkt_Count_u8 < SPI_A_O_NUM_INNER_DATA_PACKETS)
				{
					 if(SCU_I_Pkt_Count_u8 < Num_Valid_Data_Items_u8)
					 { 
						 SCU_Inner_Progress = SC_INNER_START;
						
						 while((SC_INNER_COMPLETE > SCU_Inner_Progress) 
							 &&  (SP_CONVERSION_SUCCESS__RETURN_CODE_u8 == SCU_Conversion_Success_u8)) 
						 {
							 switch(SCU_Inner_Progress)
							 {
									case  SC_INNER_START: 
										SPI_UI_To_MMS_Packet.O_Data_Packet[SCU_I_Pkt_Count_u8].D_Start_Delimiter = SPI_DATA_START_DELIMITER_CHAR;
										SCU_Inner_Progress = SC_INNER_SEQ; 
									break;  
									
									case  SC_INNER_SEQ:
										SCU_Conversion_Success_u8 = SPI_Bin_u8_To_ASCII_Pair_fn(
											 SCU_I_Pkt_Count_u8, &(SPI_UI_To_MMS_Packet.O_Data_Packet[SCU_I_Pkt_Count_u8].D_Sequence_Count[0]));
										SCU_Inner_Progress = SC_INNER_SRC;
									break;  
									case  SC_INNER_SRC:
										
										SPI_UI_To_MMS_Packet.O_Data_Packet[SCU_I_Pkt_Count_u8].D_Source_ID[0] = SPI_SUBS_ID_UI;
									
										SCU_Inner_Progress = SC_INNER_DEST;
									break;  
									case  SC_INNER_DEST:
										SPI_UI_To_MMS_Packet.O_Data_Packet[SCU_I_Pkt_Count_u8].D_Dest_ID[0]  
													= Raw_UI_To_MMS_Data_struct_ptr[SCU_I_Pkt_Count_u8].Dest_Subsystem_ID_u8;
										SCU_Inner_Progress = SC_INNER_ID;
									break;  
									
									case  SC_INNER_ID:
											SCU_Conversion_Success_u8 = SPI_Bin_u8_To_ASCII_Pair_fn(
												 Raw_UI_To_MMS_Data_struct_ptr[SCU_I_Pkt_Count_u8].CAN_ID_u8, &(SPI_UI_To_MMS_Packet.O_Data_Packet[SCU_I_Pkt_Count_u8].D_CAN_ID[0]));
								
										SCU_Inner_Progress = SC_INNER_DLC;
									break;
									
									case  SC_INNER_DLC:                   
                                    
                                      SCU_x_u8 =  Raw_UI_To_MMS_Data_struct_ptr[SCU_I_Pkt_Count_u8].CAN_DLC_u8;   // Get the DLC binary value
                                      SCU_Conversion_Success_u8 = SPI_Bin_u8_To_ASCII_Char_fn (SCU_x_u8);         // Convert to ASCII Char
                                      if(SP_INVALID_ASCII_CHAR_RETURN_CODE_u8 !=  SCU_Conversion_Success_u8)
                                        {
                                            SPI_UI_To_MMS_Packet.O_Data_Packet[SCU_I_Pkt_Count_u8].D_CAN_DLC[0] = SCU_Conversion_Success_u8 ;
                                            SCU_Conversion_Success_u8 = SP_CONVERSION_SUCCESS__RETURN_CODE_u8;
                                      }
                                        else
                                        {
                                             // Do nothing. Leave SCU_Conversion_Success_u8 set to FAIL
                                        }
                                        SCU_Inner_Progress = SC_INNER_PAYLOAD;
                                    break;  

                                   	
									case  SC_INNER_PAYLOAD:
										SCU_Loop_u8 = 0U;
									
										while(( 8U > SCU_Loop_u8 ) &&  (SP_CONVERSION_SUCCESS__RETURN_CODE_u8 == SCU_Conversion_Success_u8)) 
										{
											SCU_x_u8 = (SCU_Loop_u8 * 2U);   // Destination buffer index
											SCU_Conversion_Success_u8 = SPI_Bin_u8_To_ASCII_Pair_fn(
											 Raw_UI_To_MMS_Data_struct_ptr[SCU_I_Pkt_Count_u8].Payload_u8[SCU_Loop_u8], 
												&(SPI_UI_To_MMS_Packet.O_Data_Packet[SCU_I_Pkt_Count_u8].D_Payload[SCU_x_u8]));  // Natska? payload
											SCU_Loop_u8++;
										}
										SCU_Inner_Progress = SC_INNER_PADDING;
									
									break;  
									
									case  SC_INNER_PADDING:
										SPI_UI_To_MMS_Packet.O_Data_Packet[SCU_I_Pkt_Count_u8].D_Padding[0U] = SPI_DATA_PADDING_CHAR;
										SCU_Inner_Progress = SC_INNER_END;
									break;  
									
									case  SC_INNER_END:
										SPI_UI_To_MMS_Packet.O_Data_Packet[SCU_I_Pkt_Count_u8].D_End_Delimiter= SPI_DATA_END_DELIMITER_CHAR;
						
										SCU_Inner_Progress =SC_INNER_COMPLETE;
									break;


									case  SC_INNER_COMPLETE:
										
									
									break;
									
									default:
										break;
								} // end switch
								
						 }	// End Inner Packet while 1 packet construction 
					 } // end if < Num_Valid_Data_Items_u8
					 else
					 {    // This inner packet is Null
						 		
						 SPI_UI_To_MMS_Packet.O_Data_Packet[SCU_I_Pkt_Count_u8].D_Start_Delimiter = SPI_DATA_START_DELIMITER_CHAR;
						    
						    
						 SCU_Conversion_Success_u8 = SPI_Bin_u8_To_ASCII_Pair_fn(
											 SCU_I_Pkt_Count_u8, &(SPI_UI_To_MMS_Packet.O_Data_Packet[SCU_I_Pkt_Count_u8].D_Sequence_Count[0]));
						 		
						 SPI_UI_To_MMS_Packet.O_Data_Packet[SCU_I_Pkt_Count_u8].D_Source_ID[0U] = SPI_SUBS_ID_UI;
						 
						 SPI_UI_To_MMS_Packet.O_Data_Packet[SCU_I_Pkt_Count_u8].D_Dest_ID[0U] = SPI_SUBS_ID_NULL;
						 SPI_UI_To_MMS_Packet.O_Data_Packet[SCU_I_Pkt_Count_u8].D_CAN_ID[0U] = SPI_DATA_NULL_CHAR;
						 SPI_UI_To_MMS_Packet.O_Data_Packet[SCU_I_Pkt_Count_u8].D_CAN_ID[1U] = SPI_DATA_NULL_CHAR;
						 SPI_UI_To_MMS_Packet.O_Data_Packet[SCU_I_Pkt_Count_u8].D_CAN_DLC[0U] = SPI_DATA_NULL_CHAR;
						 for(SCU_x_u8 = 0U; 16U > SCU_x_u8; SCU_x_u8++)
						 {
							 SPI_UI_To_MMS_Packet.O_Data_Packet[SCU_I_Pkt_Count_u8].D_Payload[SCU_x_u8] = SPI_DATA_NULL_CHAR;
						 }
						 SPI_UI_To_MMS_Packet.O_Data_Packet[SCU_I_Pkt_Count_u8].D_Padding[0U] = SPI_DATA_PADDING_CHAR;
						 SPI_UI_To_MMS_Packet.O_Data_Packet[SCU_I_Pkt_Count_u8].D_End_Delimiter= SPI_DATA_END_DELIMITER_CHAR;
						
					 }
				 
					 SCU_I_Pkt_Count_u8++;
				 } // end while 
		
				SCU_Outer_Construction_Progress = SC_OUTER_PADDING;
			break;   
				
			case SC_OUTER_PADDING:
				for(SCU_Loop_u8 = 0U; SPI_A_O_PADDING2_SIZE > SCU_Loop_u8; SCU_Loop_u8++)	
				{
					SPI_UI_To_MMS_Packet.O_Paddings2[SCU_Loop_u8] = SPI_OUTER_PADDING_CHAR;
				}
			
				SCU_Outer_Construction_Progress = SC_OUTER_END;
			break;   

			case SC_OUTER_END:
				SPI_UI_To_MMS_Packet.O_End_Delimiter   = SPI_OUTER_END_DELIMITER_CHAR; 
				SCU_Outer_Construction_Progress = SC_OUTER_COMPLETE;
			break;   
			case SC_OUTER_COMPLETE:
				
			break;
	  
			default: 
			break;							
			

		} // end switch
	
	} // end while
	
	if (SP_CONVERSION_SUCCESS__RETURN_CODE_u8 == SCU_Conversion_Success_u8) 
	{
	   SPI_O_Sequence_Counter_u16++;
		SCU_Return_Success_b = TRUE;
		 // Ready to send
	}
	else
	{
		SCU_Return_Success_b = FALSE;
	}
	return(SCU_Return_Success_b);

	
}

/*!******************************************************************** 
* \brief Converts supplied ASCII HEX characters to Binary value  
* \details  Legal Input range is ('0' - '9' & 'A' - 'F')
* \param CAB_ASCII_To_Convert_u8_ptr Pointer to Data to be converted.
* \param CAB_Result_Address_u32_ptr Pointer to the result data.
* \param CAB_ASCII_Num_Str_Length_u8 TThe number of values to convert.
* \return BOOL  TRUE = Success, FALSE = Failure.
***********************************************************************/
BOOL Convert_ASCII_Num_To_Bin_u32_fn(const BYTE *CAB_ASCII_To_Convert_u8_ptr, //
  	DWORD * const CAB_Result_Address_u32_ptr, const BYTE CAB_ASCII_Num_Str_Length_u8 )
{
	BOOL CAB_Conversion_Success_b     = TRUE;
	DWORD CAB_Result_u32      	        = (DWORD) ( 0U);
	BYTE  CAB_Loop_u8                  = (BYTE) (0U);
	BYTE  CAB_Sub_Bin_Value_u8         = (BYTE) (0U);
	
	if((BYTE) (8U) < CAB_ASCII_Num_Str_Length_u8)
	{
			//Required for future release.
	}		
	else
	{
		CAB_Loop_u8 = (BYTE) (0U);
		CAB_Sub_Bin_Value_u8 = (BYTE) (0U);
		
		while((CAB_Loop_u8 < CAB_ASCII_Num_Str_Length_u8) 
			&& (TRUE == CAB_Conversion_Success_b))
		{
			  CAB_Result_u32 <<= (DWORD) (4U);
				CAB_Sub_Bin_Value_u8 = SPI_ASCII_Char_To_Bin_u8_fn( *CAB_ASCII_To_Convert_u8_ptr);
			
        if(SP_INVALID_ASCII_CHAR_RETURN_CODE_u8 != CAB_Sub_Bin_Value_u8)
				{
					CAB_Result_u32 |= (DWORD) (CAB_Sub_Bin_Value_u8);
					CAB_ASCII_To_Convert_u8_ptr++;
					CAB_Loop_u8++;
				}
				else  // It was an invalid character (i..e non ASCII-HEX)
				{
						CAB_Result_u32 = 0U;
						CAB_Conversion_Success_b = FALSE;
				}
		}
	}
	*CAB_Result_Address_u32_ptr = CAB_Result_u32;
	return(CAB_Conversion_Success_b);
}

	
/*!******************************************************************** 
* \brief Decodes an entire SPI packet from MMS to UI  
* \details Contains the state machine for Decoding the MMS packet RXed over the SPI.
* \param SPI_Decode_MMS_To_UI_Packet  Pointer to Data to be Decoded..   SPI_Decode_MMS_To_UI_Packet_fn
* \param Decoded_UI_Message_str_Array Decoded data structure
* \param NumOfPacketsRx The number of packet RXed.
* \return BOOL  TRUE = Success, FALSE = Failure.
***********************************************************************/
BOOL SPI_Decode_MMS_To_UI_Packet_fn(SPI_A_Outer_struct_Ascii_type& SPI_MMS_To_UI_Packet, 
Raw_UI_To_MMS_Data_struct_type* const Decoded_UI_Message_str_Array, BYTE& NumOfPacketsRx) 
{
    BOOL     SDP_DecodeSuccess_b = TRUE;    // Until proven otherwise
    BOOL     SDP_Appropriate_Data_b = TRUE;    // Until proven otherwise
    DWORD SDP_Binary_Result_u32             = (DWORD)(0U);
    BYTE  *SDP_Field_To_Convert             = (BYTE * ) (0U);
   
    BYTE  SDP_Inner_Packet_Idx_u8           = (BYTE) (0U); 
    BYTE  SDP_Loop_u8                       = (BYTE) (0U); 
    BYTE  SDP_x_Shift_u8                    = (BYTE) (0U);
    BYTE  SDP_x_Offset_Idx_u8               = (BYTE) (0U);
    BYTE  SDP_x_u8                          = (BYTE) (0U);
    
    struct {
    BYTE S_Num_Valid_Packets_u8;
    BYTE S_UI_Status_Flags_u8;
    BYTE S_UI_MMS_Status_Ack_Flags_u8;
    } SDU_Decoded_Status_struct;
        
    typedef enum 
    {
        S_DECODE_MMS_EXTRACT_SENT_CRC = (0U),
        S_DECODE_MMS_CRC_CHECK,
        S_DECODE_MMS_EXTRACT_OUTER_PACKET_SEQUENCE_NO,
        S_DECODE_MMS_EXTRACT_STATUS_VALID_PACKET_COUNT,
        S_DECODE_MMS_EXTRACT_STATUS_UI_FLAGS, 
        S_DECODE_MMS_EXTRACT_STATUS_MMS_ACK_FLAGS,
        S_DECODE_MMS_EXTRACT_INNER_DATA_PACKETS,
        S_DECODE_MMS_EXTRACT_COMPLETED,
        S_DECODE_MMS_EXTRACT_FAILED
    } Decode_UI_MMS_Packet_Sequence_enum;

    typedef enum 
    {
        S_MMS_DATA_DEC_SEQUENCE_COUNT = 0U,        
        S_MMS_DATA_DEC_DEST_ID,    // Dest extracted first so later can filter out messages not destined for UI
        S_MMS_DATA_DEC_SOURCE_ID,   
        S_MMS_DATA_DEC_CAN_ID,
        S_MMS_DATA_DEC_CAN_DLC,
        S_MMS_DATA_DEC_CAN_PAYLOAD,
        S_MMS_DATA_DEC_COMPLETED,
        
    }Decode_UI_MMS_Data_Packet_Sequence_enum;
    
    
    
    Decode_UI_MMS_Packet_Sequence_enum      Decode_UI_MMS_Packet_Sequence      = S_DECODE_MMS_EXTRACT_SENT_CRC;
    Decode_UI_MMS_Data_Packet_Sequence_enum Decode_UI_MMS_Data_Packet_Sequence = S_MMS_DATA_DEC_SEQUENCE_COUNT;
    
    while((S_DECODE_MMS_EXTRACT_COMPLETED > Decode_UI_MMS_Packet_Sequence) 
      && (TRUE == SDP_DecodeSuccess_b) && (TRUE == SDP_Appropriate_Data_b))
    {
            switch(Decode_UI_MMS_Packet_Sequence)
            {                
                case    S_DECODE_MMS_EXTRACT_SENT_CRC:
            
                  Decode_UI_MMS_Packet_Sequence = S_DECODE_MMS_EXTRACT_OUTER_PACKET_SEQUENCE_NO;
                break;
                
                case  S_DECODE_MMS_CRC_CHECK:   
                break;
                
                case    S_DECODE_MMS_EXTRACT_OUTER_PACKET_SEQUENCE_NO:  
                    SDP_Field_To_Convert = &(SPI_MMS_To_UI_Packet.O_Sequence_Count[0]  );
                    SDP_DecodeSuccess_b = Convert_ASCII_Num_To_Bin_u32_fn( SDP_Field_To_Convert, &SDP_Binary_Result_u32, SPI_A_O_SEQUENCE_COUNT_SIZE);
                
                    Decode_UI_MMS_Packet_Sequence = S_DECODE_MMS_EXTRACT_STATUS_VALID_PACKET_COUNT;
                break;
                
                case  S_DECODE_MMS_EXTRACT_STATUS_VALID_PACKET_COUNT:  
					SDP_Field_To_Convert = &(SPI_MMS_To_UI_Packet.O_Status_Packet.S_Packet_Count[0]);                    
                    SDP_DecodeSuccess_b = Convert_ASCII_Num_To_Bin_u32_fn( SDP_Field_To_Convert, &SDP_Binary_Result_u32, SPI_A_S__PACKET_COUNT_SIZE);
                
					if( TRUE == SDP_DecodeSuccess_b)
                    {
                        if( SPI_A_O_NUM_INNER_DATA_PACKETS  < (BYTE) (SDP_Binary_Result_u32))
                        {
                            SDP_Appropriate_Data_b = FALSE;
                        }
                        else
                        {
                           SDU_Decoded_Status_struct.S_Num_Valid_Packets_u8 = (BYTE) (SDP_Binary_Result_u32);
						   NumOfPacketsRx = SDU_Decoded_Status_struct.S_Num_Valid_Packets_u8;

                        }
                    }    
					Decode_UI_MMS_Packet_Sequence = S_DECODE_MMS_EXTRACT_STATUS_UI_FLAGS;
                break; 
                
                case  S_DECODE_MMS_EXTRACT_STATUS_UI_FLAGS:  
					SDP_Field_To_Convert = &(SPI_MMS_To_UI_Packet.O_Status_Packet.S_Status_Flags[0]  );                    
                    SDP_DecodeSuccess_b = Convert_ASCII_Num_To_Bin_u32_fn( SDP_Field_To_Convert, &SDP_Binary_Result_u32, SPI_A_S__STATUS_FLAGS_SIZE);
                
					if( TRUE == SDP_DecodeSuccess_b)
                    {
                         SDU_Decoded_Status_struct.S_UI_Status_Flags_u8 = (BYTE) (SDP_Binary_Result_u32);
                    }    
                
					Decode_UI_MMS_Packet_Sequence = S_DECODE_MMS_EXTRACT_STATUS_MMS_ACK_FLAGS;
                break; //   
                
                case  S_DECODE_MMS_EXTRACT_STATUS_MMS_ACK_FLAGS: 
					SDP_Field_To_Convert = &(SPI_MMS_To_UI_Packet.O_Status_Packet.S_Status_Ack_Flags[0]);                    
                    SDP_DecodeSuccess_b = Convert_ASCII_Num_To_Bin_u32_fn( SDP_Field_To_Convert, &SDP_Binary_Result_u32, SPI_A_S__ACK_STATUS_FLAGS_SIZE);
                
					if( TRUE == SDP_DecodeSuccess_b)
                    {
                        SDU_Decoded_Status_struct.S_UI_MMS_Status_Ack_Flags_u8 = (BYTE) (SDP_Binary_Result_u32);
                    }                        
					Decode_UI_MMS_Packet_Sequence = S_DECODE_MMS_EXTRACT_INNER_DATA_PACKETS;
                break; //   
                
                case  S_DECODE_MMS_EXTRACT_INNER_DATA_PACKETS:   
					SDP_Inner_Packet_Idx_u8 = (BYTE) (0U);
                
					while((    SPI_A_O_NUM_INNER_DATA_PACKETS > SDP_Inner_Packet_Idx_u8) 
                            && (TRUE == SDP_DecodeSuccess_b) && (TRUE == SDP_Appropriate_Data_b))
                    {
                         Decode_UI_MMS_Data_Packet_Sequence = S_MMS_DATA_DEC_SEQUENCE_COUNT;
                         
                         while(( S_MMS_DATA_DEC_COMPLETED > Decode_UI_MMS_Data_Packet_Sequence )
                              && (TRUE == SDP_DecodeSuccess_b) && (TRUE == SDP_Appropriate_Data_b))
                         {
                             switch(Decode_UI_MMS_Data_Packet_Sequence) 
                             {
                                 case S_MMS_DATA_DEC_SEQUENCE_COUNT:
                                        SDP_Field_To_Convert = &(SPI_MMS_To_UI_Packet.O_Data_Packet[SDP_Inner_Packet_Idx_u8].D_Sequence_Count[0]);
                                        SDP_DecodeSuccess_b = Convert_ASCII_Num_To_Bin_u32_fn( SDP_Field_To_Convert, &SDP_Binary_Result_u32, SPI_A_D__SEQUENCE_COUNT_SIZE);
                                        Decode_UI_MMS_Data_Packet_Sequence = S_MMS_DATA_DEC_DEST_ID ; 
                                        
                                 break;
                                        
                                  case  S_MMS_DATA_DEC_DEST_ID:
						                SDP_x_u8 = (BYTE) ( SPI_MMS_To_UI_Packet.O_Data_Packet[SDP_Inner_Packet_Idx_u8].D_Dest_ID[0U]  );
							            Decoded_UI_Message_str_Array[SDP_Inner_Packet_Idx_u8].Dest_Subsystem_ID_u8 = SDP_x_u8;
                                        Decode_UI_MMS_Data_Packet_Sequence = S_MMS_DATA_DEC_SOURCE_ID ;
                                 break;
                                 
                                 case  S_MMS_DATA_DEC_SOURCE_ID:
								       SDP_x_u8 = (BYTE) ( SPI_MMS_To_UI_Packet.O_Data_Packet[SDP_Inner_Packet_Idx_u8].D_Source_ID[0U]  );
                                       Decode_UI_MMS_Data_Packet_Sequence = S_MMS_DATA_DEC_CAN_ID;
                                 break;
                                 
                                 case  S_MMS_DATA_DEC_CAN_ID:
                                       SDP_Field_To_Convert = &(SPI_MMS_To_UI_Packet.O_Data_Packet[SDP_Inner_Packet_Idx_u8].D_CAN_ID[0]);
                                       SDP_DecodeSuccess_b = Convert_ASCII_Num_To_Bin_u32_fn( SDP_Field_To_Convert, &SDP_Binary_Result_u32, SPI_A_D__CAN_ID_SIZE);
                                       if( TRUE == SDP_DecodeSuccess_b)
                                       {
                                            Decoded_UI_Message_str_Array[SDP_Inner_Packet_Idx_u8].CAN_ID_u8 =  (BYTE) (SDP_Binary_Result_u32);
                                       }
                                       Decode_UI_MMS_Data_Packet_Sequence = S_MMS_DATA_DEC_CAN_DLC;
                                 break;
                                 
                                 case  S_MMS_DATA_DEC_CAN_DLC:
                                       SDP_Field_To_Convert = &(SPI_MMS_To_UI_Packet.O_Data_Packet[SDP_Inner_Packet_Idx_u8].D_CAN_DLC[0]);
                                       SDP_DecodeSuccess_b = Convert_ASCII_Num_To_Bin_u32_fn( SDP_Field_To_Convert, &SDP_Binary_Result_u32, SPI_A_D__CAN_DLC_SIZE);
                                       if( TRUE == SDP_DecodeSuccess_b)
                                       {
                                            Decoded_UI_Message_str_Array[SDP_Inner_Packet_Idx_u8].CAN_DLC_u8 = (BYTE) (SDP_Binary_Result_u32);
                                       }
                                       Decode_UI_MMS_Data_Packet_Sequence =S_MMS_DATA_DEC_CAN_PAYLOAD;
                                 break;
                                 
                                 case  S_MMS_DATA_DEC_CAN_PAYLOAD:   // Natska 
                                       SDP_Field_To_Convert = &(SPI_MMS_To_UI_Packet.O_Data_Packet[SDP_Inner_Packet_Idx_u8].D_Payload[0U]);
                                       SDP_DecodeSuccess_b = Convert_ASCII_Num_To_Bin_u32_fn( SDP_Field_To_Convert, &SDP_Binary_Result_u32, 8U);
                                       if( TRUE == SDP_DecodeSuccess_b)
                                       { 
                                           SDP_x_Shift_u8 = 0U;
                                           for(SDP_Loop_u8 = 0U; SDP_Loop_u8 < 4U; SDP_Loop_u8++ )
                                           {
                                                Decoded_UI_Message_str_Array[SDP_Inner_Packet_Idx_u8].Payload_u8[SDP_Loop_u8] = (BYTE) ((SDP_Binary_Result_u32) >> SDP_x_Shift_u8);
                                                SDP_x_Shift_u8 += (BYTE) (8U);
                                           }
                                       }
                                       SDP_Field_To_Convert = &(SPI_MMS_To_UI_Packet.O_Data_Packet[SDP_Inner_Packet_Idx_u8].D_Payload[8U]);
                                       SDP_DecodeSuccess_b = Convert_ASCII_Num_To_Bin_u32_fn( SDP_Field_To_Convert, &SDP_Binary_Result_u32, 8U);
                                       if( TRUE == SDP_DecodeSuccess_b)
                                       { 
                                            SDP_x_Shift_u8 = 0U;
                                            for(SDP_Loop_u8 = 0U; SDP_Loop_u8 < 4U; SDP_Loop_u8++ )
                                            {
                                                SDP_x_Offset_Idx_u8 = SDP_Loop_u8 + 4U;
                                                Decoded_UI_Message_str_Array[SDP_Inner_Packet_Idx_u8].Payload_u8[SDP_x_Offset_Idx_u8] = (BYTE) ((SDP_Binary_Result_u32) >> SDP_x_Shift_u8);
                                                SDP_x_Shift_u8 += (BYTE) (8U);
                                            }
                                       }
                                       Decode_UI_MMS_Data_Packet_Sequence = S_MMS_DATA_DEC_COMPLETED;
                                 break;
                                 
                                 case   S_MMS_DATA_DEC_COMPLETED:
										//For future Release	
                                 break;
                                 
                                 default:
                                     // Error trap
                                 break;
                             } // End sequenceswitch
                        } 
                        
                        SDP_Inner_Packet_Idx_u8++;
                     } // end pkt idx while
                     if(SPI_A_O_NUM_INNER_DATA_PACKETS != SDP_Inner_Packet_Idx_u8)
                     {
						//For future Release
					 }
                     
                   Decode_UI_MMS_Packet_Sequence = S_DECODE_MMS_EXTRACT_COMPLETED;
                break; //   
                
              case	S_DECODE_MMS_EXTRACT_COMPLETED:   
                  //For future Release
              break;
                
              case  S_DECODE_MMS_EXTRACT_FAILED:   
					//For future Release
              break; 
                
              default:
					//For future Release
              break;
                
        
                
        } // end switch            
                
    } // end while - complete sequence
    
    
    return(SDP_DecodeSuccess_b);
    
    
}


}