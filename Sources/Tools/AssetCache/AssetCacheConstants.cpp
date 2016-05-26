#include "AssetCache/AssetCacheConstants.h"
#include "Base/GlobalEnum.h"
#include "Debug/DVAssert.h"

namespace DAVA
{
namespace AssetCache
{
const String LOCALHOST = "127.0.0.1";

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
    { Error::CANNOT_SEND_REQUEST_ADD, "CANNOT_SEND_REQUEST_ADD" },
    { Error::CANNOT_SEND_REQUEST_GET, "CANNOT_SEND_REQUEST_GET" },
    { Error::CORRUPTED_DATA, "CORRUPTED_DATA" }
    } };

    DVASSERT(static_cast<uint32>(Error::ERRORS_COUNT) == errorStrings.size());

    const auto& errorDetails = errorStrings[static_cast<uint32>(error)];

    DVASSERT(errorDetails.first == error);
    return errorDetails.second;
}

} // end of namespace AssetCache
} // end of namespace DAVA
