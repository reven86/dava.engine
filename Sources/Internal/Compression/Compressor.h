#ifndef DAVAENGINE_FILE_SYSTEM_COMPRESSOR_H
#define DAVAENGINE_FILE_SYSTEM_COMPRESSOR_H

#include "Base/BaseTypes.h"

namespace DAVA
{
class Compressor
{
public:
    enum class Type : uint32
    {
        None,
        Lz4,
        Lz4HC,
        RFC1951, // deflate, inflate
    };

    virtual ~Compressor();

    virtual bool Compress(const Vector<uint8>& in, Vector<uint8>& out) const = 0;
    // you should resize output to correct size before call this method
    virtual bool Decompress(const Vector<uint8>& in, Vector<uint8>& out) const = 0;
};

} // end namespace DAVA

#endif // DAVAENGINE_FILE_SYSTEM_COMPRESSOR_H