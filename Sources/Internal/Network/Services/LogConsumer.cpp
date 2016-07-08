#include "Debug/DVAssert.h"
#include "FileSystem/File.h"
#include "Utils/StringFormat.h"

#include "Network/Base/Endpoint.h"

#include "LogConsumer.h"

namespace DAVA
{
namespace Net
{
void LogConsumer::OnPacketReceived(IChannel* channel, const void* buffer, size_t length)
{
    String data(static_cast<const char8*>(buffer), length);
    String endp(channel->RemoteEndpoint().ToString());

    String output = Format("[%s] %s", endp.c_str(), data.c_str());
    newDataNotifier.Emit(output);
}

} // namespace Net
} // namespace DAVA