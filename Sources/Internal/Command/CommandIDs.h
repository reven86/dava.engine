#pragma once

namespace DAVA
{
using CommandID = DAVA::uint32;

enum eDAVACommandIDs : CommandID
{
    INVALID_COMMAND_ID = 0,
    COMMAND_BATCH,
    PROPERTY_VALUE_COMMAND,
    COLLECTION_ITEM_VALUE_COMMAND,
    COMMAND_IDS_COUNT
};
} //namespace DAVA
