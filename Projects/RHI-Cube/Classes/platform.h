/*-------------------------------------------------------------------------------------------------
file:   platform.h
author: Vadzim Ziankovich
dsc:
-------------------------------------------------------------------------------------------------*/
#pragma once
#include <stdarg.h>

#include "Base/BaseTypes.h"
using DAVA::uint32;
/*
    typedef unsigned int DWORD;
    typedef int LONG;
    typedef unsigned char BYTE;
    typedef unsigned short WORD;
    typedef int BOOL;
    typedef float FLOAT;
    typedef int INT;
    typedef unsigned int UINT;

    typedef unsigned long UINT_PTR;
    typedef unsigned long SIZE_T;
    typedef unsigned long LONGLONG;
    typedef unsigned long long UINT64;
*/
//#ifdef OS_CENTOS
/*
    #undef __STRICT_ANSI__
    #include <wchar.h>
    #include <stdio.h>
    #include <stdarg.h>
    #include <string.h>
    #include <stddef.h>
*/
//  #define WINAPI

//  #define CONST const
//  #define nullptr NULL
//  #define errno_t int

//  #define _strupr_s Platform_strupr
/*
    typedef unsigned int DWORD;
    typedef int LONG;
    typedef unsigned char BYTE;
    typedef unsigned short WORD;
    typedef int BOOL;
    typedef float FLOAT;
    typedef int INT;
    typedef unsigned int UINT;

    typedef unsigned long UINT_PTR;
    typedef unsigned long SIZE_T;
    typedef unsigned long LONGLONG;
    typedef unsigned long long UINT64;

    typedef BYTE* LPBYTE;
    typedef void* HANDLE;
    typedef HANDLE HWND;
    typedef HANDLE HMODULE;
    typedef UINT_PTR WPARAM;
    typedef UINT64 LPARAM;
*/
/*
    typedef struct tagRECT
    {
        LONG left;
        LONG top;
        LONG right;
        LONG bottom;
    } RECT, *PRECT;

    typedef struct tagPOINT
    {
        LONG x;
        LONG y;
    } POINT, *PPOINT;
*/
template <typename T1, typename T2>
auto max(const T1& a, const T2& b) -> decltype(a > b ? a : b)
{
    return a > b ? a : b;
}

template <typename T1, typename T2>
auto min(const T1& a, const T2& b) -> decltype(a < b ? a : b)
{
    return a < b ? a : b;
}
/*
    inline int _vscprintf(const char* format, va_list pargs)
    {
        int retval;
        va_list argcopy;
        va_copy(argcopy,pargs);
        retval = vsnprintf(NULL,0,format,argcopy);
        va_end(argcopy);
        return retval;
    }

    inline int _vscwprintf(const wchar_t* format, va_list argptr)
    {
        return vswprintf(0,0,format,argptr);
    }
*/
//  #define _vsnprintf vsnprintf
//  #define _vsnwprintf vswprintf

//  #define LINE_ENDING "\n"

    #define CONST const
//  #define sscanf_s sscanf
//  #define nullptr NULL

    #define ModuleName() "server"

#ifndef GENERIC_READ
    #define GENERIC_READ 0x01
    #define GENERIC_WRITE 0x02
    #define FILE_SHARE_READ 0x04
    #define FILE_SHARE_WRITE 0x08

    #define OPEN_ALWAYS 0x10
    #define CREATE_ALWAYS 0x20
    #define OPEN_EXISTING 0x40
    #define TRUNCATE_EXISTING 0x80
    
    #define MB_OK 0x01
    #define MB_SETFOREGROUND 0x02
    #define MB_TOPMOST 0x03
    #define MB_ICONERROR 0x04
    #define MB_ICONINFORMATION 0x05
    #define MB_ICONWARNING 0x06
#endif
//#else // win32x64

    #define LINE_ENDING "\r\n"

//#endif // OS_CENTOS

#define SYSTEM_API

SYSTEM_API void* PlatformLoadLibrary(const char* name);
SYSTEM_API bool PlatformFreeLibrary(void* handle);
SYSTEM_API void* PlatformGetProcAddress(void* module, const char* proc_name);

SYSTEM_API void PlatformSetUnhandledExceptionFilter();
SYSTEM_API void PlatformMessageBox(const char* message, const char* caption, uint32 msg_type);

SYSTEM_API void Platform_vsprintf(char* buffer, size_t buf_size, const char* format, va_list pargs);
SYSTEM_API int Platform_vscprintf(const char* format, va_list ap);
SYSTEM_API int Platform_strupr(char* string, size_t size);

SYSTEM_API float PlatformTimeNextQuant(LONGLONG& qpc_time_previous);
SYSTEM_API LONGLONG PlatformTimeFreq(void);
SYSTEM_API LONGLONG PlatformTimeTicks(void);
SYSTEM_API uint32 PlatformTimeGetTime(void);
