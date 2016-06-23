#include "ClientApplication.h"

#include "AddRequest.h"
#include "GetRequest.h"

ClientApplication::ClientApplication()
    : cacheClient(true)
{
    requests.emplace_back(std::unique_ptr<CacheRequest>(new AddRequest()));
    requests.emplace_back(std::unique_ptr<CacheRequest>(new GetRequest()));
}

ClientApplication::~ClientApplication()
{
    activeRequest = nullptr;
}

bool ClientApplication::ParseCommandLine(int argc, char* argv[])
{
    if (argc > 1)
    {
        for (auto& r : requests)
        {
            auto commandLineIsOk = r->options.Parse(argc, argv);
            if (commandLineIsOk)
            {
                activeRequest = r.get();
                exitCode = activeRequest->CheckOptions();
                break;
            }
        }

        if (exitCode != DAVA::AssetCache::Error::NO_ERRORS)
        {
            PrintUsage();
            return false;
        }
        return true;
    }
    else
    {
        PrintUsage();
        exitCode = DAVA::AssetCache::Error::WRONG_COMMAND_LINE;
    }
    return false;
}

void ClientApplication::PrintUsage() const
{
    printf("\nUsage: AssetCacheClient <command>\n");
    printf("\n Commands: ");

    auto count = requests.size();
    for (auto& r : requests)
    {
        printf("%s", r->options.GetCommand().c_str());
        if (count != 1)
        {
            printf(", ");
        }
        --count;
    }

    printf("\n\n");
    for (auto& r : requests)
    {
        printf("%s\n", r->options.GetUsageString().c_str());
        printf("\n");
    }
}

void ClientApplication::Process()
{
    DVASSERT(activeRequest != nullptr);

    exitCode = activeRequest->Process(cacheClient);
}
