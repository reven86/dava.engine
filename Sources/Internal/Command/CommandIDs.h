#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
using CommandID_t = DAVA::uint32;

enum CommandID : CommandID_t
{
    CMDID_BATCH,

    CMDID_SET_PROPERTY_VALUE,
    CMDID_SET_COLLECTION_ITEM_VALUE,

    CMDID_USER = 0xFFFF
};
}
