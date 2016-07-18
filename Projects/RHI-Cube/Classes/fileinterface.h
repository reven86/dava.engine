/*-------------------------------------------------------------------------------------------------
file:   fileinterface.h
author: Vadzim Ziankovich
dsc:
-------------------------------------------------------------------------------------------------*/
#pragma once
#include "rc.h"

class CFileBase;

///////////////////////////////////////////////////////////////////////////////////////////////////
class SYSTEM_API CFileInterface : public CNonCopyble
{
public:
    typedef CFileBase* (*CreatorFn)();

    CFileInterface(void);
    CFileInterface(CFileBase* impl);
    virtual ~CFileInterface(void);

    static void RegisterCreateFile(CreatorFn pfn_create_file);

    CRC Create(const char* path,
               uint32 dw_desired_access = GENERIC_READ | GENERIC_WRITE,
               uint32 dw_share_mode = FILE_SHARE_READ | FILE_SHARE_WRITE);
    CRC OpenExisting(const char* path,
                     uint32 dw_desired_access = GENERIC_READ,
                     uint32 dw_share_mode = FILE_SHARE_READ);
    CRC Open(const char* path,
             uint32 dw_creation_disposition = OPEN_ALWAYS,
             uint32 dw_desired_access = GENERIC_READ | GENERIC_WRITE,
             uint32 dw_share_mode = FILE_SHARE_READ | FILE_SHARE_WRITE);
    void Close(void);
    CRC Read(void* dst, uint32 number_of_bytes_to_read, uint32* number_of_bytes_read = 0);
    CRC Write(void* src, uint32 number_of_bytes_to_write);
    CRC WriteString(const _string& s);
    CRC Size(uint32* size_low, uint32* size_high = 0);

    CRC RemoveFile(const char* path);

private:
    CFileBase* m_file;

    static CreatorFn s_pfn_create_file;
};
