#include "FileSystem/StaticMemoryFile.h"
#include "Utils/StringFormat.h"
#include "Logger/Logger.h"

namespace DAVA
{
StaticMemoryFile* StaticMemoryFile::Create(uint8* data, uint32 dataSize, uint32 attributes)
{
    if (attributes & File::APPEND)
    {
        Logger::Warning("[StaticMemoryFile::Create] Cannot append static memory file");
        return nullptr;
    }

    StaticMemoryFile* fl = new StaticMemoryFile(data, dataSize, attributes);
    fl->filename = Format("memoryfile_%p", static_cast<void*>(fl));

    return fl;
}

StaticMemoryFile::StaticMemoryFile(uint8* data, uint32 dataSize, uint32 attributes)
    : File()
    , memoryBuffer(data)
    , memoryBufferSize(dataSize)
    , fileAttributes(attributes)
{
}

StaticMemoryFile::~StaticMemoryFile()
{
    memoryBuffer = nullptr;
    memoryBufferSize = 0;
    currentPos = 0;
}

uint32 StaticMemoryFile::Write(const void* pointerToData, uint32 dataSize)
{
    DVASSERT(nullptr != pointerToData);
    if ((fileAttributes & File::WRITE) == 0)
    {
        return 0;
    }

    uint32 written = GetRWOperationSize(dataSize);
    if (written > 0)
    {
        Memcpy(memoryBuffer + currentPos, pointerToData, written);
        currentPos += written;
    }

    isEof = (dataSize != written);

    return written;
}

uint32 StaticMemoryFile::Read(void* pointerToData, uint32 dataSize)
{
    DVASSERT(nullptr != pointerToData);
    if ((fileAttributes & File::READ) == 0)
    {
        return 0;
    }

    uint32 read = GetRWOperationSize(dataSize);
    if (read > 0)
    {
        Memcpy(pointerToData, memoryBuffer + currentPos, read);
        currentPos += read;
    }
    isEof = (dataSize != read);

    return read;
}

uint32 StaticMemoryFile::GetRWOperationSize(uint32 dataSize) const
{
    if (0 == dataSize || 0 == memoryBufferSize)
    {
        return 0;
    }

    if (currentPos < memoryBufferSize)
    {
        uint32 tailSpace = memoryBufferSize - currentPos;
        return Min(tailSpace, dataSize);
    }

    return 0;
}

bool StaticMemoryFile::Seek(int64 position, eFileSeek seekType)
{
    int64 pos = 0;
    switch (seekType)
    {
    case SEEK_FROM_START:
        pos = position;
        break;
    case SEEK_FROM_CURRENT:
        pos = GetPos() + position;
        break;
    case SEEK_FROM_END:
        pos = GetSize() - 1 + position;
        break;
    default:
        return false;
    };

    if (pos < 0)
    {
        return false;
    }

    // behavior taken from std::FILE - don't move pointer to less than 0 value
    currentPos = static_cast<uint32>(pos);

    // like in std::FILE
    // The end-of-file internal indicator of the stream is cleared after a successful call to this function
    isEof = false;

    return true;
}

} // end of namespace DAVA
