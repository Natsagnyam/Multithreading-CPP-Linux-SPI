/* sle7000 type def 

/home/natska/Documents/Cmd2CAN/cmds2CAN/EngUtility/SPIComms/linux_type_sle.h
/home/natska/Documents/Cmd2CAN/cmds2CAN/linux_type_sle.h
*/    
    
#pragma once


    typedef struct _SYSTEMTIME
{
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
} SYSTEMTIME, *LPSYSTEMTIME;

typedef void* LPVOID;

    typedef unsigned char        BOOL;
    typedef unsigned char        BYTE;
    typedef char                 CHAR;
    typedef unsigned short int   WORD;      // 16 bit
    typedef signed short         __int16;
    typedef unsigned int          DWORD;       // 32 bit
    typedef unsigned int	        UINT;
    typedef unsigned long int     ULONG;       // 64 bit
    
    typedef void* HANDLE;
    typedef void* HGDIOBJ;