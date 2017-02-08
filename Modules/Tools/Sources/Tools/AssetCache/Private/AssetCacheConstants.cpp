#include "Tools/AssetCache/AssetCacheConstants.h"

#include <Base/GlobalEnum.h>
#include <Debug/DVAssert.h>

namespace DAVA
{
namespace AssetCache
{
const String& GetLocalHost()
{
    static DAVA::String localHost("127.0.0.1");
    return localHost;
}

String PacketToString(ePacketID packet)
{
    static const Vector<std::pair<ePacketID, String>> packetStrings =
    { {
    { ePacketID::PACKET_UNKNOWN, "PACKET_UNKNOWN" },
    { ePacketID::PACKET_ADD_REQUEST, "PACKET_ADD_REQUEST" },
    { ePacketID::PACKET_ADD_RESPONSE, "PACKET_ADD_RESPONSE" },
    { ePacketID::PACKET_GET_REQUEST, "PACKET_GET_REQUEST" },
    { ePacketID::PACKET_GET_RESPONSE, "PACKET_GET_RESPONSE" },
    { ePacketID::PACKET_WARMING_UP_REQUEST, "PACKET_WARMING_UP_REQUEST" },
    { ePacketID::PACKET_STATUS_REQUEST, "PACKET_STATUS_REQUEST" },
    { ePacketID::PACKET_STATUS_RESPONSE, "PACKET_STATUS_RESPONSE" },
    { ePacketID::PACKET_REMOVE_REQUEST, "PACKET_REMOVE_REQUEST" },
    { ePacketID::PACKET_REMOVE_RESPONSE, "PACKET_REMOVE_RESPONSE" },
    { ePacketID::PACKET_CLEAR_REQUEST, "PACKET_CLEAR_REQUEST" },
    { ePacketID::PACKET_CLEAR_RESPONSE, "PACKET_CLEAR_RESPONSE" }
    } };

    DVASSERT(static_cast<uint32>(ePacketID::PACKET_COUNT) == packetStrings.size());

    const auto& entry = packetStrings[static_cast<uint32>(packet)];

    DVASSERT(entry.first == packet);
    return entry.second;
}

String ErrorToString(Error error)
{
    static const Vector<std::pair<Error, String>> errorStrings =
    { {
    { Error::NO_ERRORS, "OK" },
    { Error::CODE_NOT_INITIALIZED, "CODE_NOT_INITIALIZED" },
    { Error::WRONG_COMMAND_LINE, "WRONG_COMMAND_LINE" },
    { Error::WRONG_IP, "WRONG_IP" },
    { Error::OPERATION_TIMEOUT, "OPERATION_TIMEOUT" },
    { Error::CANNOT_CONNECT, "CANNOT_CONNECT" },
    { Error::SERVER_ERROR, "SERVER_ERROR" },
    { Error::NOT_FOUND_ON_SERVER, "NOT_FOUND_ON_SERVER" },
    { Error::READ_FILES, "READ_FILES" },
    { Error::ADDRESS_RESOLVER_FAILED, "ADDRESS_RESOLVER_FAILED" },
    { Error::CANNOT_SEND_REQUEST, "CANNOT_SEND_REQUEST" },
    { Error::CORRUPTED_DATA, "CORRUPTED_DATA" },
    { Error::UNSUPPORTED_VERSION, "UNSUPPORTED_VERSION" },
    { Error::UNEXPECTED_PACKET, "UNEXPECTED_PACKET" }
    } };

    DVASSERT(static_cast<uint32>(Error::ERRORS_COUNT) == errorStrings.size());

    const auto& errorDetails = errorStrings[static_cast<uint32>(error)];

    DVASSERT(errorDetails.first == error);
    return errorDetails.second;
}

} // end of namespace AssetCache
} // end of namespace DAVA
