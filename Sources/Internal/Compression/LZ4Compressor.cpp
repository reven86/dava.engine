/*==================================================================================
Copyright (c) 2008, binaryzebra
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
* Neither the name of the binaryzebra nor the
names of its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "Compression/LZ4Compressor.h"
#include "Logger/Logger.h"

#include <lz4/lz4.h>
#include <lz4/lz4hc.h>

namespace DAVA
{
bool LZ4Compressor::Compress(const Vector<uint8>& in, Vector<uint8>& out) const
{
    if (in.size() > LZ4_MAX_INPUT_SIZE)
    {
        Logger::Error("LZ4 compress failed too big input buffer");
        return false;
    }
    uint32 maxSize = static_cast<uint32>(LZ4_compressBound(in.size()));
    if (out.size() < maxSize)
    {
        out.resize(maxSize);
    }
    int compressedSize = LZ4_compress(reinterpret_cast<const char*>(in.data()), reinterpret_cast<char*>(out.data()), in.size());
    if (compressedSize == 0)
    {
        return false;
    }
    out.resize(static_cast<uint32>(compressedSize));
    return true;
}

bool LZ4Compressor::Uncompress(const Vector<uint8>& in, Vector<uint8>& out) const
{
    bool result = true;
    int decompressResult = LZ4_decompress_fast(reinterpret_cast<const char*>(in.data()), reinterpret_cast<char*>(out.data()), out.size());
    if (decompressResult < 0)
    {
        Logger::Error("LZ4 decompress failed");
        result = false;
    }
    return result;
}

bool LZ4HCCompressor::Compress(const Vector<uint8>& in, Vector<uint8>& out) const
{
    if (in.size() > LZ4_MAX_INPUT_SIZE)
    {
        Logger::Error("LZ4 compress failed too big input buffer");
        return false;
    }
    uint32 maxSize = static_cast<uint32>(LZ4_compressBound(in.size()));
    if (out.size() < maxSize)
    {
        out.resize(maxSize);
    }
    int compressedSize = LZ4_compressHC(reinterpret_cast<const char*>(in.data()), reinterpret_cast<char*>(out.data()), in.size());
    if (compressedSize == 0)
    {
        return false;
    }
    out.resize(static_cast<uint32>(compressedSize));
    return true;
}

} // end namespace DAVA
