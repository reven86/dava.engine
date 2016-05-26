#include "DownloaderCommon.h"
#include "FileSystem/File.h"
#include "FileSystem/FileSystem.h"

namespace DAVA
{
DownloadTaskDescription::DownloadTaskDescription(const String& srcUrl, const FilePath& storeToFilePath, DownloadType downloadMode, int32 _timeout, int32 _retriesCount, uint8 _partsCount)
    : id(0)
    , url(srcUrl)
    , storePath(storeToFilePath)
    , fileErrno(0)
    , timeout(_timeout)
    , retriesCount(_retriesCount)
    , retriesLeft(retriesCount)
    , type(downloadMode)
    , status(DL_UNKNOWN)
    , error(DLE_NO_ERROR)
    , downloadTotal(0)
    , downloadProgress(0)
    , partsCount(_partsCount)
{
}

DownloadPart::DownloadPart(Downloader* currentDownloader)
    : downloader(currentDownloader)
    , dataBuffer(0)
    , downloadSize(0)
    , seekPos(0)
    , progress(0)

{
}

bool DownloadPart::SaveToBuffer(char8* srcBuf, uint32 size)
{
    DVASSERT(downloadSize - progress >= size);
    if (downloadSize - progress > 0)
    {
        Memcpy(dataBuffer + progress, srcBuf, size);
        progress += size;
        return true;
    }
    else
    {
        Logger::Error("[DownloadPart::SaveToBuffer] Cannot save data.");
    }
    return false;
}

DataChunkInfo::DataChunkInfo(uint32 size)
    : buffer(NULL)
    , bufferSize(size)
    , progress(0)
{
    buffer = new char8[bufferSize];
}

DataChunkInfo::~DataChunkInfo()
{
    SafeDeleteArray(buffer);
}
}