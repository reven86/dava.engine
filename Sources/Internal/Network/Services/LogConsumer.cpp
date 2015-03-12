#include <Debug/DVAssert.h>
#include <FileSystem/File.h>
#include <Utils/StringFormat.h>

#include <Network/Base/Endpoint.h>

#include "LogConsumer.h"

namespace DAVA
{
namespace Net
{

LogConsumer::LogConsumer(const char8* filename, bool toConsoleFlag)
    : file(NULL)
    , toConsole(toConsoleFlag)
{
    DVASSERT(filename != NULL);
    file = File::Create(filename, File::WRITE | File::APPEND);
    DVASSERT(file != NULL);
}

LogConsumer::~LogConsumer()
{
    SafeRelease(file);
}

void LogConsumer::OnPacketReceived(IChannel* channel, const void* buffer, size_t length)
{
    if (file)
    {
        String endp (channel->RemoteEndpoint().ToString());
        String message(static_cast<const char8*>(buffer), length);
        String output = Format("[%s] %s", endp.c_str(), message.c_str());
        file->WriteLine(output);
        if (toConsole)
            printf("%s\n", output.c_str());
    }
}

}   // namespace Net
}   // namespace DAVA
