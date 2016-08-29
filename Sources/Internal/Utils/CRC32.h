#ifndef __DAVAENGINE__CRC32__
#define __DAVAENGINE__CRC32__

#include "Base/BaseTypes.h"

namespace DAVA
{
class FilePath;
class CRC32
{
public:
    CRC32();
    void AddData(const void* data, size_t size);
    uint32 Done();

    // Calculate the CRC32 for the in-memory buffer.
    static uint32 ForBuffer(const void* data, size_t size);

    // Calculate CRC32 for the whole file.
    static uint32 ForFile(const FilePath& pathName);

private:
    uint32 crc32;
};
}
#endif /* defined(__DAVAENGINE__CRC32__) */
