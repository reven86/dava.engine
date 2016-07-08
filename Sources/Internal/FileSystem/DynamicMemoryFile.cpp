#include "FileSystem/DynamicMemoryFile.h"
#include "Utils/StringFormat.h"

namespace DAVA
{
DynamicMemoryFile* DynamicMemoryFile::Create(Vector<uint8>&& data, uint32 attributes, const FilePath& name)
{
    DynamicMemoryFile* f = new DynamicMemoryFile();
    f->data = std::move(data);
    f->currentPtr = 0;
    f->fileAttributes = attributes;
    f->filename = name;
    return f;
}

DynamicMemoryFile* DynamicMemoryFile::Create(const uint8* data, int32 dataSize, uint32 attributes)
{
    DynamicMemoryFile* fl = new DynamicMemoryFile();
    fl->filename = Format("memoryfile_%p", static_cast<void*>(fl));
    fl->Write(data, dataSize);
    fl->fileAttributes = attributes;
    fl->currentPtr = 0;

    return fl;
}

DynamicMemoryFile* DynamicMemoryFile::Create(uint32 attributes)
{
    DynamicMemoryFile* fl = new DynamicMemoryFile();
    fl->fileAttributes = attributes;
    fl->filename = Format("memoryfile_%p", static_cast<void*>(fl));

    return fl;
}

DynamicMemoryFile::DynamicMemoryFile()
    : File()
    , isEof(false)
{
    currentPtr = 0;
    fileAttributes = File::WRITE;
}

DynamicMemoryFile::~DynamicMemoryFile()
{
}

const uint8* DynamicMemoryFile::GetData() const
{
    if (!data.empty())
    {
        return data.data();
    }
    else
    {
        return nullptr;
    }
}

uint32 DynamicMemoryFile::Write(const void* pointerToData, uint32 dataSize)
{
    if (!(fileAttributes & File::WRITE) && !(fileAttributes & File::APPEND))
    {
        return 0;
    }

    if (data.size() < currentPtr + dataSize)
    {
        data.resize(currentPtr + dataSize);
    }
    if (dataSize)
    {
        DVASSERT(nullptr != pointerToData);
        Memcpy(&(data[currentPtr]), pointerToData, dataSize);
        currentPtr += dataSize;
    }

    return dataSize;
}

uint32 DynamicMemoryFile::Read(void* pointerToData, uint32 dataSize)
{
    DVASSERT(NULL != pointerToData);

    if (!(fileAttributes & File::READ))
    {
        return 0;
    }

    int32 realReadSize = dataSize;
    uint32 size = static_cast<uint32>(data.size());
    if (currentPtr + realReadSize > size)
    {
        isEof = true;
        realReadSize = size - currentPtr;
    }
    if (0 < realReadSize)
    {
        Memcpy(pointerToData, &(data[currentPtr]), realReadSize);
        currentPtr += realReadSize;

        return realReadSize;
    }

    return 0;
}

uint32 DynamicMemoryFile::GetPos() const
{
    return currentPtr;
}

uint32 DynamicMemoryFile::GetSize() const
{
    return static_cast<uint32>(data.size());
}

bool DynamicMemoryFile::Seek(int32 position, eFileSeek seekType)
{
    int32 pos = 0;
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
    currentPtr = pos;

    // like in std::FILE
    // The end-of-file internal indicator of the stream is cleared after a successful call to this function
    isEof = false;

    return true;
}

bool DynamicMemoryFile::IsEof() const
{
    return isEof;
}
};
