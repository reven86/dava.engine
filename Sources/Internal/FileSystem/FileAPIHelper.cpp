#include "FileSystem/FileAPIHelper.h"
#include "Utils/UTF8Utils.h"
#include "Debug/DVAssert.h"
#include "Logger/Logger.h"

#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>

namespace DAVA
{
namespace FileAPI
{
#ifdef __DAVAENGINE_WINDOWS__
struct Stat : _stat
{
};
const auto FileStat = _wstat;
#else
struct Stat : stat
{
};
const auto FileStat = stat;
#endif

FILE* OpenFile(const char* fileName, const char* mode)
{
    DVASSERT(nullptr != fileName);
    DVASSERT(nullptr != mode);
    return std::fopen(fileName, mode);
}

int32 RemoveFile(const char* fileName)
{
    DVASSERT(nullptr != fileName);
    return std::remove(fileName);
}

int32 RenameFile(const char* oldFileName, const char* newFileName)
{
    DVASSERT(nullptr != oldFileName);
    DVASSERT(nullptr != newFileName);
    return std::rename(oldFileName, newFileName);
}

bool IsRegularFile(const char* fileName)
{
    DVASSERT(nullptr != fileName);

    Stat fileStat;

#ifdef __DAVAENGINE_WINDOWS__
    WideString p = UTF8Utils::EncodeToWideString(String(fileName));
    int32 result = FileStat(p.c_str(), &fileStat);
#else
    int32 result = FileStat(fileName, &fileStat);
#endif
    if (result == 0)
    {
        return (0 != (fileStat.st_mode & S_IFREG));
    }

    switch (errno)
    {
    case ENOENT:
        // file not found
        break;
    case EINVAL:
        Logger::Error("Invalid parameter to stat.");
        break;
    default:
        /* Should never be reached. */
        Logger::Error("Unexpected error in %s: errno = (%d)", __FUNCTION__, static_cast<int32>(errno));
    }
    return false;
}
} // end namespace FileAPI
} // end namespace DAVA
