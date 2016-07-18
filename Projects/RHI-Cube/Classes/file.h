/*-------------------------------------------------------------------------------------------------
file:   file.h
author: Mosiychuck Dmitry
dsc:
-------------------------------------------------------------------------------------------------*/
#pragma once
#include "filebase.h"

namespace DAVA
{
class File;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
class SYSTEM_API TFile : public CFileBase
{
public:
    TFile(void);
    virtual ~TFile(void);

    //  static CRC GetFileAttributes(const char* path, WIN32_FILE_ATTRIBUTE_DATA* data);

    virtual CRC Create(const char* path,
                       uint32 dw_desired_access = GENERIC_READ | GENERIC_WRITE,
                       uint32 dw_share_mode = FILE_SHARE_READ | FILE_SHARE_WRITE);
    virtual CRC OpenExisting(const char* path,
                             uint32 dw_desired_access = GENERIC_READ,
                             uint32 dw_share_mode = FILE_SHARE_READ);
    virtual CRC Open(const char* path,
                     uint32 dw_creation_disposition = OPEN_ALWAYS,
                     uint32 dw_desired_access = GENERIC_READ | GENERIC_WRITE,
                     uint32 dw_share_mode = FILE_SHARE_READ | FILE_SHARE_WRITE);
    virtual void Close(void);
    virtual CRC Read(void* dst, uint32 number_of_bytes_to_read, uint32* number_of_bytes_read = 0);
    virtual CRC Write(void* src, uint32 number_of_bytes_to_write);
    virtual CRC WriteString(const _string& s);
    virtual CRC Size(uint32* size_low, uint32* size_high = 0);

    virtual CRC RemoveFile(const char* path);

    static void SetCurFile(const char* file);

protected:
    //    HANDLE m_file;
    DAVA::File* _file;
};
