/*-------------------------------------------------------------------------------------------------
file:   filebase.h
author: Vadzim Ziankovich
dsc:
-------------------------------------------------------------------------------------------------*/
#pragma once
#include "rc.h"
#include "common.h"
#include <string>

///////////////////////////////////////////////////////////////////////////////////////////////////
class SYSTEM_API CFileBase : public CNonCopyble
{
public:
    CFileBase(void)
    {
    }
    virtual ~CFileBase(void)
    {
    }

    virtual CRC Create(const char* path,
                       uint32 dw_desired_access = GENERIC_READ | GENERIC_WRITE,
                       uint32 dw_share_mode = FILE_SHARE_READ | FILE_SHARE_WRITE) = 0;
    virtual CRC OpenExisting(const char* path,
                             uint32 dw_desired_access = GENERIC_READ,
                             uint32 dw_share_mode = FILE_SHARE_READ) = 0;
    virtual CRC Open(const char* path,
                     uint32 dw_creation_disposition = OPEN_ALWAYS,
                     uint32 dw_desired_access = GENERIC_READ | GENERIC_WRITE,
                     uint32 dw_share_mode = FILE_SHARE_READ | FILE_SHARE_WRITE) = 0;

    virtual void Close(void) = 0;

    virtual CRC Read(void* dst, uint32 number_of_bytes_to_read, uint32* number_of_bytes_read = 0) = 0;
    virtual CRC Write(void* src, uint32 number_of_bytes_to_write) = 0;
    virtual CRC WriteString(const _string& s) = 0;
    virtual CRC Size(uint32* size_low, uint32* size_high = 0) = 0;

    virtual CRC RemoveFile(const char* path) = 0;
};
