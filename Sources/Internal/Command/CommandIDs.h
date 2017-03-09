#pragma once

namespace DAVA
{
using CommandID = DAVA::uint32;

enum eDAVACommandIDs : CommandID
{
    INVALID_COMMAND = 0,
    BATCH_COMMAND,
    PROPERTY_VALUE_COMMAND,
    COLLECTION_ITEM_VALUE_COMMAND,
    USER_COMMAND
};
} //namespace DAVA
