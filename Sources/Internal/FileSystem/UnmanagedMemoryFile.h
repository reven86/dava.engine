#pragma once

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "FileSystem/File.h"

namespace DAVA
{
class UnmanagedMemoryFile : public File
{
public:
    UnmanagedMemoryFile() = default;
    UnmanagedMemoryFile(const uint8* p, uint32 sz);
    UnmanagedMemoryFile(const UnmanagedMemoryFile& r);

    uint32 Read(void* destinationBuffer, uint32 dataSize) override;
    uint64 GetPos() const override;
    bool IsEof() const override;

private:
    bool Seek(int64 position, eFileSeek seekType) override;
    uint32 Write(const void* sourceBuffer, uint32 dataSize) override;

private:
    const uint8* pointer = nullptr;
    uint32 size = 0;
    uint32 offset = 0;
};

inline UnmanagedMemoryFile::UnmanagedMemoryFile(const uint8* p, uint32 sz)
    : pointer(p)
    , size(sz)
{
}

inline UnmanagedMemoryFile::UnmanagedMemoryFile(const UnmanagedMemoryFile& r)
    : pointer(r.pointer)
    , size(r.size)
{
}

inline uint32 UnmanagedMemoryFile::Read(void* destinationBuffer, uint32 dataSize)
{
    DVASSERT(pointer != nullptr);

    if (offset + dataSize > size)
    {
        dataSize = size - offset;
    }

    memcpy(destinationBuffer, pointer + offset, dataSize);
    offset += dataSize;

    return dataSize;
}

inline uint64 UnmanagedMemoryFile::GetPos() const
{
    return offset;
}

inline bool UnmanagedMemoryFile::IsEof() const
{
    return offset == size;
}

inline bool UnmanagedMemoryFile::Seek(int64 position, eFileSeek seekType)
{
    DVASSERT_MSG(0, "Seek is not supported");
    return false;
}

inline uint32 UnmanagedMemoryFile::Write(const void* sourceBuffer, uint32 dataSize)
{
    DVASSERT_MSG(0, "Write is not supported");
    return 0;
}
}
