#ifndef __DAVAENGINE__CRC32__
#define __DAVAENGINE__CRC32__

#include "Base/BaseTypes.h"

namespace DAVA
{
class FilePath;
class CRC32
{
public:
    // Calculate the CRC32 for the in-memory buffer.
    static uint32 ForBuffer(const char* data, uint32 size);

    // Calculate CRC32 for the whole file.
    static uint32 ForFile(const FilePath& pathName);

protected:
    // Process the data block.
    static uint32 ProcessData(const char* data, uint32 size, uint32 crc32);
};
}
#endif /* defined(__DAVAENGINE__CRC32__) */
