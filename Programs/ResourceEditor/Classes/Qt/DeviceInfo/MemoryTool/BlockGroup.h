#ifndef __MEMORYTOOL_BLOCKGROUP_H__
#define __MEMORYTOOL_BLOCKGROUP_H__

#include "Base/BaseTypes.h"

#include "Qt/DeviceInfo/MemoryTool/BlockLink.h"

struct BlockGroup
{
    template <typename T>
    BlockGroup(const DAVA::String& title_, DAVA::uint32 key_, T&& blockLink_)
        : title(title_)
        , key(key_)
        , blockLink(std::move(blockLink_))
    {
    }
    BlockGroup(BlockGroup&& other)
        : title(std::move(other.title))
        , key(std::move(other.key))
        , blockLink(std::move(other.blockLink))
    {
    }
    BlockGroup& operator=(BlockGroup&& other)
    {
        if (this != &other)
        {
            title = std::move(other.title);
            key = std::move(other.key);
            blockLink = std::move(other.blockLink);
        }
        return *this;
    }

    DAVA::String title;
    DAVA::uint32 key;
    BlockLink blockLink;
};

#endif // __MEMORYTOOL_BLOCKGROUP_H__
