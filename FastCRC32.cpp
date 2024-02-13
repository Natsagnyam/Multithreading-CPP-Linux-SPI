/*!********************************************************************
 * \file
 * \brief This file provides the CRC32 firmware functions for the STM32 MMS processor
 * \details 
 * (C)opyright 2015 SLE Ltd
 *
 * SVN Revision: $Revision: 488 $
 *
 * Modification Record 
 * -------------------
 * 
 *  Date        | By             | Description
 *  ----------: | :------------: | :-------- 
 *  16-Jul-2014 | A. Willis      | Initial build.
 ***********************************************************************/
// // #include "stdafx.h"
#include "inc/FastCRC32.h"

namespace CommandAndResponse
{
	
	/*!******************************************************************** 
	* \brief The class constructor (empty)
	***********************************************************************/
	CFastCRC32::CFastCRC32(void)
	{
	}
	/*!******************************************************************** 
	* \brief The class destructor (empty)
	***********************************************************************/
	CFastCRC32::~CFastCRC32(void)
	{
	}

	/*!******************************************************************** 
	* \brief Performs a single CRC32 operation on a 32 bit value
	*
	* \param CFa_Crc_u32	Initial CRC value
	* \param CFa_Data_u32	Value to be operated upon DWORD
	* \return CFB_Result_Crc_u32: 32 bit CRC result
	***********************************************************************/
	DWORD CFastCRC32::Crc32Fast_fn(const DWORD CFa_Crc_u32, const DWORD CFa_Data_u32) const
	{
		static const DWORD CrcTable[16U] = { // Nibble lookup table for 0x04C11DB7 polynomial
		0x00000000UL,0x04C11DB7UL,0x09823B6EUL,0x0D4326D9UL,0x130476DCUL,0x17C56B6BUL,0x1A864DB2UL,0x1E475005UL,
		0x2608EDB8UL,0x22C9F00FUL,0x2F8AD6D6UL,0x2B4BCB61UL,0x350C9B64UL,0x31CD86D3UL,0x3C8EA00AUL,0x384FBDBDUL };

		DWORD CF_Crc_Working_u32       = CFa_Crc_u32;
		DWORD CF_Data_u32              = CFa_Data_u32;
		
		DWORD CF_Crc_Result_u32        = CF_Crc_Working_u32 ^ CF_Data_u32; // Apply all 32-bits
		
		// Process 32-bits, 4 at a time, or 8 rounds
		DWORD CF_Crc_Table_Index_u32;        
		DWORD CF_Crc_Table_Value_u32;

		CF_Crc_Working_u32     =   (CF_Crc_Result_u32 << 0x00000004UL) ;
		CF_Crc_Table_Index_u32 =   CF_Crc_Result_u32;
		CF_Crc_Table_Index_u32 >>= 28UL;
		CF_Crc_Table_Index_u32  &= 0x0000000FUL;
		CF_Crc_Table_Value_u32 =   CrcTable[CF_Crc_Table_Index_u32];  
		CF_Crc_Result_u32      =   CF_Crc_Working_u32 ^ CF_Crc_Table_Value_u32;

		CF_Crc_Working_u32     =   (CF_Crc_Result_u32 << 0x00000004UL) ;
		CF_Crc_Table_Index_u32 =   CF_Crc_Result_u32;
		CF_Crc_Table_Index_u32 >>= 28UL;
		CF_Crc_Table_Index_u32  &= 0x0000000FUL;
		CF_Crc_Table_Value_u32 =   CrcTable[CF_Crc_Table_Index_u32];  
		CF_Crc_Result_u32      =   CF_Crc_Working_u32 ^ CF_Crc_Table_Value_u32;

		CF_Crc_Working_u32     =   (CF_Crc_Result_u32 << 0x00000004UL) ;
		CF_Crc_Table_Index_u32 =   CF_Crc_Result_u32;
		CF_Crc_Table_Index_u32 >>= 28UL;
		CF_Crc_Table_Index_u32  &= 0x0000000FUL;
		CF_Crc_Table_Value_u32 =   CrcTable[CF_Crc_Table_Index_u32];  
		CF_Crc_Result_u32      =   CF_Crc_Working_u32 ^ CF_Crc_Table_Value_u32;

		CF_Crc_Working_u32     =   (CF_Crc_Result_u32 << 0x00000004UL) ;
		CF_Crc_Table_Index_u32 =   CF_Crc_Result_u32;
		CF_Crc_Table_Index_u32 >>= 28UL;
		CF_Crc_Table_Index_u32  &= 0x0000000FUL;
		CF_Crc_Table_Value_u32 =   CrcTable[CF_Crc_Table_Index_u32];  
		CF_Crc_Result_u32      =   CF_Crc_Working_u32 ^ CF_Crc_Table_Value_u32;

		CF_Crc_Working_u32     =   (CF_Crc_Result_u32 << 0x00000004UL) ;
		CF_Crc_Table_Index_u32 =   CF_Crc_Result_u32;
		CF_Crc_Table_Index_u32 >>= 28UL;
		CF_Crc_Table_Index_u32  &= 0x0000000FUL;
		CF_Crc_Table_Value_u32 =   CrcTable[CF_Crc_Table_Index_u32];  
		CF_Crc_Result_u32      =   CF_Crc_Working_u32 ^ CF_Crc_Table_Value_u32;

		CF_Crc_Working_u32     =   (CF_Crc_Result_u32 << 0x00000004UL) ;
		CF_Crc_Table_Index_u32 =   CF_Crc_Result_u32;
		CF_Crc_Table_Index_u32 >>= 28UL;
		CF_Crc_Table_Index_u32  &= 0x0000000FUL;
		CF_Crc_Table_Value_u32 =   CrcTable[CF_Crc_Table_Index_u32];  
		CF_Crc_Result_u32      =   CF_Crc_Working_u32 ^ CF_Crc_Table_Value_u32;

		CF_Crc_Working_u32     =   (CF_Crc_Result_u32 << 0x00000004UL) ;
		CF_Crc_Table_Index_u32 =   CF_Crc_Result_u32;
		CF_Crc_Table_Index_u32 >>= 28UL;
		CF_Crc_Table_Index_u32  &= 0x0000000FUL;
		CF_Crc_Table_Value_u32 =   CrcTable[CF_Crc_Table_Index_u32];  
		CF_Crc_Result_u32      =   CF_Crc_Working_u32 ^ CF_Crc_Table_Value_u32;

		CF_Crc_Working_u32     =   (CF_Crc_Result_u32 << 0x00000004UL) ;
		CF_Crc_Table_Index_u32 =   CF_Crc_Result_u32;
		CF_Crc_Table_Index_u32 >>= 28UL;
		CF_Crc_Table_Index_u32  &= 0x0000000FUL;
		CF_Crc_Table_Value_u32 =   CrcTable[CF_Crc_Table_Index_u32];  
		CF_Crc_Result_u32      =   CF_Crc_Working_u32 ^ CF_Crc_Table_Value_u32;    

		return(CF_Crc_Result_u32);
	}

	/*!******************************************************************** 
	* \brief Performs CRC32 check on a block of data at address CFBa_InBuffer_u32_ptr
	*
	* \param CFBa_Initial_Crc_u32	Initial CRC Seed value DWORD
	* \param CFBa_Size_u32			Size in 32 bit (4 byte) units type DWORD
	* \param CFBa_InBuffer_u32_ptr	Pointer to 32 bit (4 byte) units block to CRC check
	* \return CFB_Result_Crc_u32: 32 bit CRC result
	* \details Calls Crc32Fast_fn using Polynomial 0x04C11DB7
	***********************************************************************/
	DWORD CFastCRC32::Crc32FastBlock_fn(const DWORD CFBa_Initial_Crc_u32, DWORD CFBa_Size_u32, DWORD * const CFBa_InBuffer_u32_ptr) const// 
	{
		DWORD   CFB_Result_Crc_u32  = (DWORD) (0UL);
		DWORD   CFB_Submit_Crc_u32  = CFBa_Initial_Crc_u32;
		DWORD * CFB_Buffer_u32_ptr  = CFBa_InBuffer_u32_ptr;
	        
		while(CFBa_Size_u32 > 0U)
		{
			CFBa_Size_u32--;
			CFB_Result_Crc_u32 = Crc32Fast_fn(CFB_Submit_Crc_u32, *CFB_Buffer_u32_ptr);
			CFB_Submit_Crc_u32 = CFB_Result_Crc_u32;
			CFB_Buffer_u32_ptr++;
		}

		return(CFB_Result_Crc_u32);
	}
}