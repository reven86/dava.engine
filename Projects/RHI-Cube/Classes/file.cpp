/*-------------------------------------------------------------------------------------------------
file:   file.cpp
author: Mosiychuck Dmitry
dsc:
-------------------------------------------------------------------------------------------------*/
#include "file.h"
#include "fileinterface.h"
#include "FileSystem/File.h"
#include "FileSystem/FileSystem.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
/*
CRC TFile::GetFileAttributes(const char* path, WIN32_FILE_ATTRIBUTE_DATA* data)
{
    VALIDATE(!path, ERR_BADVAL)
    VALIDATE(!data, ERR_BADVAL)

    //-------------------------------------------

    TRY_WIN32(GetFileAttributesEx(path, GetFileExInfoStandard, data))

    return ERR_OK;
}
*/

static std::vector<std::string> IncludeSearchPath;

void
TFile::SetCurFile(const char* filename)
{
    DAVA::FilePath fileDir = DAVA::FilePath(filename).GetDirectory();

    IncludeSearchPath.clear();
    //    IncludeSearchPath.push_back(fileDir.IsEmpty() ? std::string() : fileDir.GetFrameworkPath());
    IncludeSearchPath.push_back(DAVA::FilePath("~res:/Materials/Shaders/").MakeDirectoryPathname().GetFrameworkPath());
    IncludeSearchPath.push_back("../../Tools/ResourceEditor/Data/Materials/Shaders/");
}

///////////////////////////////////////////////////////////////////////////////////////////////////
TFile::TFile(void)
    :
    //    m_file(0),
    _file(nullptr)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
TFile::~TFile(void)
{
    Close();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
CRC TFile::Create(const char* path, uint32 dw_desired_access, uint32 dw_share_mode)
{
    return ERR_FILESYSTEMERROR;
    /// return Open(path, CREATE_ALWAYS, dw_desired_access, dw_share_mode);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
CRC TFile::OpenExisting(const char* filename, uint32 dw_desired_access, uint32 dw_share_mode)
{
    if (DAVA::FileSystem::Instance()->IsFile(filename))
    {
        _file = DAVA::File::Create(filename, DAVA::File::OPEN | DAVA::File::READ);
    }
    else
    {
        for (std::vector<std::string>::const_iterator p = IncludeSearchPath.begin(), p_end = IncludeSearchPath.end(); p != p_end; ++p)
        {
            std::string name = *p;

            if (filename[0] == '/')
                name += (filename + 1);
            else
                name += filename;

            if (DAVA::FileSystem::Instance()->IsFile(name.c_str()))
            {
                _file = DAVA::File::Create(name, DAVA::File::OPEN | DAVA::File::READ);
                break;
            }
        }
    }

    //    _file = DAVA::File::CreateFromSystemPath( path, DAVA::File::OPEN | DAVA::File::READ );
    return (_file) ? ERR_OK : ERR_FILESYSTEMERROR;
    /// return Open(path, OPEN_EXISTING, dw_desired_access, dw_share_mode);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
CRC TFile::Open(const char* path, uint32 dw_creation_disposition, uint32 dw_desired_access, uint32 dw_share_mode)
{
    /// VALIDATE(!path, ERR_BADVAL)
    /*
    //-------------------------------------------

    Close();

    m_file = CreateFile(path,
                        dw_desired_access,
                        dw_share_mode,
                        0,
                        dw_creation_disposition,
                        FILE_ATTRIBUTE_NORMAL,
                        0);
    if(INVALID_HANDLE_VALUE == m_file)
    {
        m_file = 0;

        if(dw_creation_disposition == OPEN_EXISTING)
        {
            return GENERATE_WARNINGCODE(ERR_FILENOTFOUND);
        }else
        {
            return ERR_FILESYSTEMERROR;
        }
    }
*/
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TFile::Close(void)
{
    if (_file)
    {
        _file->Release();
        _file = nullptr;
    }
    /*
    if(!m_file)
    {
        return;
    }

    BOOL result = CloseHandle(m_file);
    if(!result)
    {
        FATALERROR(ERR_FILESYSTEMERROR);
    }

    m_file = 0;
*/
}

///////////////////////////////////////////////////////////////////////////////////////////////////
CRC TFile::Read(void* dst, uint32 number_of_bytes_to_read, uint32* number_of_bytes_read)
{
    if (_file)
    {
        uint32 read = _file->Read(dst, number_of_bytes_to_read);

        if (number_of_bytes_read)
            *number_of_bytes_read = read;

        return ERR_OK;
    }
    else
    {
        return ERR_FILESYSTEMERROR;
    }
    /*
    VALIDATE(!m_file, ERR_BADOP)

    //-------------------------------------------

    uint32 numberOfBytesRead;
    TRY_WIN32(ReadFile(m_file, dst, number_of_bytes_to_read, &numberOfBytesRead, 0))

    if(number_of_bytes_read)
    {
        *number_of_bytes_read = numberOfBytesRead;
    }
*/
}

///////////////////////////////////////////////////////////////////////////////////////////////////
CRC TFile::Write(void* src, uint32 number_of_bytes_to_write)
{
    /*
    VALIDATE(!m_file, ERR_BADOP)

    //-------------------------------------------

    uint32 number_of_bytes_written;
    TRY_WIN32(WriteFile(m_file, src, number_of_bytes_to_write, &number_of_bytes_written, 0))

    if(number_of_bytes_written != number_of_bytes_to_write)
    {
        return ERROR(ERR_FILESYSTEMERROR);
    }
*/
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
CRC TFile::WriteString(const _string& s)
{
    return Write((void*)s.c_str(), (uint32)s.size());
}

CRC CFileInterface::WriteString(const _string& s)
{
    return m_file->Write((void*)s.c_str(), (uint32)s.size());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
CRC TFile::Size(uint32* size_low, uint32* size_high)
{
    if (_file)
    {
        *size_low = _file->GetSize();

        if (size_high)
            *size_high = 0;
    }
    /*
    VALIDATE(!m_file, ERR_BADOP)
    VALIDATE(!size_low, ERR_BADVAL)

    //-------------------------------------------

    uint32 size_low_ = GetFileSize(m_file, size_high);
    if(INVALID_FILE_SIZE == size_low_)
    {
        return ERROR(ERR_FILESYSTEMERROR);
    }

    *size_low = size_low_;
*/
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
CRC TFile::RemoveFile(const char* path)
{
    return ERR_OK;
}
