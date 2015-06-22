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


#include "ZLibStream.h"
#include "FileSystem/File.h"

namespace DAVA
{

ZLibIStream::ZLibIStream(File *_file)
: file(_file)
{
    SafeRetain(file);

    zstream.zalloc = Z_NULL;
    zstream.zfree = Z_NULL;
    zstream.opaque = Z_NULL;
    zstream.avail_in = 0;
    zstream.next_in = Z_NULL;
    
    int res = inflateInit(&zstream);
    DVASSERT(Z_OK == res);
}

ZLibIStream::~ZLibIStream()
{
    inflateEnd(&zstream);
    SafeRelease(file);
}

uint32 ZLibIStream::Read(char8 *data, uint32 size)
{
    zstream.avail_out = size;
    zstream.next_out = (unsigned char *) data;

    while(zstream.avail_out > 0)
    {
        if(0 == zstream.avail_in)
        {
            zstream.avail_in = file->Read(readBuffer, ZLIB_CHUNK_SIZE);
            zstream.next_in = (unsigned char *) readBuffer;
        }

        if(0 != zstream.avail_in)
        {
            if(Z_OK != inflate(&zstream, Z_NO_FLUSH))
            {
                break;
            }
        }
		else
		{
			// we didn't read anything
			break;
		}
    }

    return (size - zstream.avail_out);
}


ZLibOStream::ZLibOStream(File *_file, int compressionLevel)
: file(_file)
{
    SafeRetain(file);

    zstream.zalloc = Z_NULL;
    zstream.zfree = Z_NULL;
    zstream.opaque = Z_NULL;
    zstream.avail_in = 0;
    zstream.next_in = Z_NULL;
    deflateInit(&zstream, compressionLevel);
}

ZLibOStream::~ZLibOStream()
{
    int ret = Z_OK;
    while(Z_OK == ret)
    {
        zstream.avail_out = ZLIB_CHUNK_SIZE;
        zstream.next_out = (unsigned char *) writeBuffer;

        ret = deflate(&zstream, Z_FINISH);

        uint32 outSize = ZLIB_CHUNK_SIZE - zstream.avail_out;
        file->Write(writeBuffer, outSize);
    }

    deflateEnd(&zstream);
    SafeRelease(file);
}

uint32 ZLibOStream::Write(char8 *data, uint32 size)
{
    zstream.avail_in = size;
    zstream.next_in = (unsigned char *) data;

    while(zstream.avail_in > 0)
    {
        zstream.avail_out = ZLIB_CHUNK_SIZE;
        zstream.next_out = (unsigned char *) writeBuffer;

        if(Z_OK == deflate(&zstream, Z_NO_FLUSH))
        {
            uint32 outSize = ZLIB_CHUNK_SIZE - zstream.avail_out;
			if(outSize != file->Write(writeBuffer, outSize))
			{
				// we didn't write everything
				break;
			}
        }
        else
        {
            break;
        }
    }

    return (size - zstream.avail_in);
}

}
