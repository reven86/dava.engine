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

#include <Compression/ZipCompressor.h>
#include <Compression/LZ4Compressor.h>

#include "UnitTests/UnitTests.h"

using namespace DAVA;

DAVA_TESTCLASS (CompressorTest)
{
    DAVA_TEST (TestFunction)
    {
        uint32 inSize = 4096;
        const Vector<uint8> in(inSize, 'a');

        LZ4Compressor lz4;
        LZ4HCCompressor lz4hc;
        ZipCompressor zip;

        Vector<uint8> compressedLz4;

        TEST_VERIFY(lz4.Compress(in, compressedLz4));

        TEST_VERIFY(in.size() > compressedLz4.size());

        Vector<uint8> compressedLz4hc;

        TEST_VERIFY(lz4hc.Compress(in, compressedLz4hc));

        TEST_VERIFY(in.size() > compressedLz4hc.size());

        Vector<uint8> compressedDeflate;

        TEST_VERIFY(zip.Compress(in, compressedDeflate));

        TEST_VERIFY(in.size() > compressedDeflate.size());

        Vector<uint8> uncompressedLz4(inSize, '\0');

        TEST_VERIFY(lz4.Uncompress(compressedLz4, uncompressedLz4));

        TEST_VERIFY(uncompressedLz4 == in);

        Vector<uint8> uncompressedLz4hc(inSize, '\0');

        TEST_VERIFY(lz4hc.Uncompress(compressedLz4hc, uncompressedLz4hc));

        TEST_VERIFY(uncompressedLz4hc == in);

        Vector<uint8> uncompressedZip(inSize, '\0');

        TEST_VERIFY(zip.Uncompress(compressedDeflate, uncompressedZip));

        TEST_VERIFY(uncompressedZip == in);
    }
};
