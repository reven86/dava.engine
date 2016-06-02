#include "PackManager/Private/VirtualFileSystemSqliteWraper.h"

#include <sqlite3.h>

#include <FileSystem/File.h>
#include <FileSystem/FileSystem.h>
#include <Concurrency/Thread.h>
#include <ctime>

/*
** When using this VFS, the sqlite3_file* handles that SQLite uses are
** actually pointers to instances of type DavaFile.
*/
struct WrapFile
{
    sqlite3_file base; /* Base class. Must be first. */
    DAVA::File* file; /* File descriptor */
};

static int Write(sqlite3_file* pFile, /* File handle */
                 const void* zBuf, /* Buffer containing data to write */
                 int iAmt, /* Size of data to write in bytes */
                 sqlite_int64 iOfst /* File offset to write to */)
{
    WrapFile* f = reinterpret_cast<WrapFile*>(pFile);
    if (!f->file->Seek(static_cast<DAVA::int32>(iOfst), DAVA::File::SEEK_FROM_START))
    {
        return SQLITE_IOERR_WRITE;
    }

    DAVA::uint32 sizeToWrite = static_cast<DAVA::uint32>(iAmt);
    DAVA::uint32 result = f->file->Write(zBuf, sizeToWrite);
    if (result != sizeToWrite)
    {
        return SQLITE_IOERR_WRITE;
    }

    return SQLITE_OK;
}

static int Close(sqlite3_file* pFile)
{
    WrapFile* p = reinterpret_cast<WrapFile*>(pFile);
    DAVA::SafeRelease(p->file);
    return SQLITE_OK;
}

static int Read(sqlite3_file* pFile, void* zBuf, int iAmt, sqlite_int64 iOfst)
{
    WrapFile* f = reinterpret_cast<WrapFile*>(pFile);

    if (!f->file->Seek(static_cast<DAVA::int32>(iOfst), DAVA::File::SEEK_FROM_START))
    {
        return SQLITE_IOERR_WRITE;
    }
    DAVA::uint32 bufSize = static_cast<DAVA::uint32>(iAmt);
    DAVA::uint32 sizeRead = f->file->Read(zBuf, bufSize);

    if (bufSize == sizeRead)
    {
        return SQLITE_OK;
    }

    return SQLITE_IOERR_READ;
}

static int Truncate(sqlite3_file* pFile, sqlite_int64 size)
{
    WrapFile* p = reinterpret_cast<WrapFile*>(pFile);
    DAVA::uint32 newSize = static_cast<DAVA::uint32>(size);
    if (!p->file->Truncate(newSize))
    {
        return SQLITE_IOERR_TRUNCATE;
    }
    return SQLITE_OK;
}

static int Sync(sqlite3_file* pFile, int /*flags*/)
{
    WrapFile* p = reinterpret_cast<WrapFile*>(pFile);

    if (!p->file->Flush())
    {
        return SQLITE_IOERR_FSYNC;
    }
    return SQLITE_OK;
}

/*
** Write the size of the file in bytes to *pSize.
*/
static int FileSize(sqlite3_file* pFile, sqlite_int64* pSize)
{
    WrapFile* p = reinterpret_cast<WrapFile*>(pFile);

    DAVA::uint32 fileSize = p->file->GetSize();

    *pSize = static_cast<sqlite_int64>(fileSize);
    return SQLITE_OK;
}

/*
** Locking functions. The xLock() and xUnlock() methods are both no-ops.
** The xCheckReservedLock() always indicates that no other process holds
** a reserved lock on the database file. This ensures that if a hot-journal
** file is found in the file-system it is rolled back.
*/
static int Lock(sqlite3_file* /*pFile*/, int /*eLock*/)
{
    return SQLITE_OK;
}
static int Unlock(sqlite3_file* pFile, int eLock)
{
    return SQLITE_OK;
}
static int CheckReservedLock(sqlite3_file* /*pFile*/, int* pResOut)
{
    *pResOut = 0;
    return SQLITE_OK;
}

/*
** No xFileControl() verbs are implemented by this VFS.
*/
static int FileControl(sqlite3_file* /*pFile*/, int /*op*/, void* /*pArg*/)
{
    return SQLITE_OK;
}

/*
** The xSectorSize() and xDeviceCharacteristics() methods. These two
** may return special values allowing SQLite to optimize file-system
** access to some extent. But it is also safe to simply return 0.
*/
static int SectorSize(sqlite3_file* pFile)
{
    return 0;
}

static int DeviceCharacteristics(sqlite3_file* pFile)
{
    return 0;
}

/*
** Open a file handle.
*/
static int Open(sqlite3_vfs* pVfs, /* VFS */
                const char* zName, /* File to open, or 0 for a temp file */
                sqlite3_file* pFile, /* Pointer to DemoFile struct to populate */
                int flags, /* Input SQLITE_OPEN_XXX flags */
                int* pOutFlags /* Output SQLITE_OPEN_XXX flags (or NULL) */)
{
    static const sqlite3_io_methods davaio = {
        1, /* iVersion */
        Close, /* xClose */
        Read, /* xRead */
        Write, /* xWrite */
        Truncate, /* xTruncate */
        Sync, /* xSync */
        FileSize, /* xFileSize */
        Lock, /* xLock */
        Unlock, /* xUnlock */
        CheckReservedLock, /* xCheckReservedLock */
        FileControl, /* xFileControl */
        SectorSize, /* xSectorSize */
        DeviceCharacteristics /* xDeviceCharacteristics */
    };

    WrapFile* p = reinterpret_cast<WrapFile*>(pFile); /* Populate this structure */
    int oflags = 0; /* eFileAttributes */

    if (zName == nullptr)
    {
        return SQLITE_IOERR;
    }

    if (flags & SQLITE_OPEN_MAIN_JOURNAL)
    {
        // do we need in memory jornal file?
        throw std::runtime_error("not implemented");
    }

    if (flags & SQLITE_OPEN_EXCLUSIVE)
    {
        // no supported by DAVA
        // oflags |= O_EXCL;
    }
    if (flags & SQLITE_OPEN_CREATE)
    {
        oflags |= (DAVA::File::CREATE | DAVA::File::OPEN);
    }
    if (flags & SQLITE_OPEN_READONLY)
    {
        oflags |= (DAVA::File::READ | DAVA::File::OPEN);
    }
    if (flags & SQLITE_OPEN_READWRITE)
    {
        // currently open DB only in READONLY mode
        // becouse on iOS and Android we can't modify file in APK or Bundle
        oflags |= (DAVA::File::READ | /*DAVA::File::WRITE |*/ DAVA::File::OPEN);
    }

    memset(p, 0, sizeof(WrapFile));
    p->file = DAVA::File::Create(zName, oflags);
    if (p->file == nullptr)
    {
        return SQLITE_CANTOPEN;
    }

    if (pOutFlags)
    {
        *pOutFlags = flags;
    }
    p->base.pMethods = &davaio;
    return SQLITE_OK;
}

/*
** Delete the file identified by argument zPath. If the dirSync parameter
** is non-zero, then ensure the file-system modification to delete the
** file has been synced to disk before returning.
*/
static int Delete(sqlite3_vfs* pVfs, const char* zPath, int dirSync)
{
    if (!DAVA::FileSystem::Instance()->DeleteFile(zPath))
    {
        return SQLITE_OK;
    }

    if (dirSync)
    {
        // DAVA no such api to sync file was deleted
    }
    return SQLITE_OK;
}

/*
** Query the file-system to see if the named file exists, is readable or
** is both readable and writable.
*/
static int Access(sqlite3_vfs* /*pVfs*/, const char* zPath, int flags, int* pResOut)
{
    if (!DAVA::FileSystem::Instance()->IsFile(zPath))
    {
        *pResOut = 0;
    }
    else
    {
        *pResOut = 1;
    }

    return SQLITE_OK;
}

/*
** Argument zPath points to a nul-terminated string containing a file path.
** If zPath is an absolute path, then it is copied as is into the output
** buffer. Otherwise, if it is a relative path, then the equivalent full
** path is written to the output buffer.
**
** This function assumes that paths are UNIX style. Specifically, that:
**
**   1. Path components are separated by a '/'. and
**   2. Full paths begin with a '/' character.
*/
static int FullPathname(sqlite3_vfs* pVfs, /* VFS */
                        const char* zPath, /* Input path (possibly a relative path) */
                        int nPathOut, /* Size of output buffer in bytes */
                        char* zPathOut /* Pointer to output buffer */)
{
    DAVA::FilePath path(zPath);
    DAVA::String absolute = path.GetAbsolutePathname();

    if (absolute.size() < static_cast<DAVA::uint32>(nPathOut))
    {
        std::strncpy(zPathOut, absolute.c_str(), absolute.size() + 1); // copy '\0' too
    }
    else
    {
        return SQLITE_IOERR;
    }

    return SQLITE_OK;
}

/*
** The following four VFS methods:
**
**   xDlOpen
**   xDlError
**   xDlSym
**   xDlClose
**
** are supposed to implement the functionality needed by SQLite to load
** extensions compiled as shared objects. This simple VFS does not support
** this functionality, so the following functions are no-ops.
*/
static void* DlOpen(sqlite3_vfs* /*pVfs*/, const char* /*zPath*/)
{
    return nullptr;
}

static void DlError(sqlite3_vfs* /*pVfs*/, int nByte, char* zErrMsg)
{
    sqlite3_snprintf(nByte, zErrMsg, "Loadable extensions are not supported");
    zErrMsg[nByte - 1] = '\0';
}

static void (*DlSym(sqlite3_vfs* /*pVfs*/, void* /*pH*/, const char* /*z*/))(void)
{
    return nullptr;
}

static void DlClose(sqlite3_vfs* pVfs, void* pHandle)
{
}

/*
** Parameter zByte points to a buffer nByte bytes in size. Populate this
** buffer with pseudo-random data.
*/
static int Randomness(sqlite3_vfs* pVfs, int nByte, char* zByte)
{
    return SQLITE_OK;
}

/*
** Sleep for at least nMicro microseconds. Return the (approximate) number
** of microseconds slept for.
*/
static int Sleep(sqlite3_vfs* pVfs, int nMicro)
{
    DAVA::Thread::Sleep(nMicro / 1000);
    return nMicro;
}

/*
** Set *pTime to the current UTC time expressed as a Julian day. Return
** SQLITE_OK if successful, or an error code otherwise.
**
**   http://en.wikipedia.org/wiki/Julian_day
**
** This implementation is not very good. The current time is rounded to
** an integer number of seconds. Also, assuming time_t is a signed 32-bit
** value, it will stop working some time in the year 2038 AD (the so-called
** "year 2038" problem that afflicts systems that store time this way).
*/
static int CurrentTime(sqlite3_vfs* /*pVfs*/, double* pTime)
{
    time_t t = std::time(nullptr);
    *pTime = t / 86400.0 + 2440587.5;
    return SQLITE_OK;
}

/*
** This function returns a pointer to the VFS implemented in this file.
** To make the VFS available to SQLite:
**
**   sqlite3_vfs_register(sqlite3_demovfs(), 0);
*/
static sqlite3_vfs* sqlite3DavaVFS()
{
    static sqlite3_vfs demovfs = {
        1, /* iVersion */
        sizeof(WrapFile), /* szOsFile */
        512, /* mxPathname */
        nullptr, /* pNext */
        "dava_vfs", /* zName */
        nullptr, /* pAppData */
        Open, /* xOpen */
        Delete, /* xDelete */
        Access, /* xAccess */
        FullPathname, /* xFullPathname */
        DlOpen, /* xDlOpen */
        DlError, /* xDlError */
        DlSym, /* xDlSym */
        DlClose, /* xDlClose */
        Randomness, /* xRandomness */
        Sleep, /* xSleep */
        CurrentTime, /* xCurrentTime */
    };
    return &demovfs;
}

namespace DAVA
{
void RegisterDavaVFSForSqlite3()
{
    DAVA::int32 result = sqlite3_vfs_register(sqlite3DavaVFS(), 1);
    DVASSERT(result == SQLITE_OK);
}

void UnregisterDavaVFSForSqlite3()
{
    DAVA::int32 result = sqlite3_vfs_unregister(sqlite3DavaVFS());
    DVASSERT(result == SQLITE_OK);
}
} // end namespace DAVA
