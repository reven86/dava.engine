#ifndef __CLIENT_APPLICATION_H__
#define __CLIENT_APPLICATION_H__

#include "Base/BaseTypes.h"

#include "AssetCache/AssetCacheClient.h"

class CacheRequest;
class ClientApplication final
{
public:
    ClientApplication();
    ~ClientApplication();

    bool ParseCommandLine(int argc, char* argv[]);

    DAVA::AssetCache::Error GetExitCode() const
    {
        return exitCode;
    };

    void Process();

private:
    void PrintUsage() const;

private:
    DAVA::AssetCache::Error exitCode = DAVA::AssetCache::Error::WRONG_COMMAND_LINE;

    DAVA::List<std::unique_ptr<CacheRequest>> requests;
    CacheRequest* activeRequest = nullptr;

    DAVA::AssetCacheClient cacheClient;
};

#endif //__CLIENT_APPLICATION_H__