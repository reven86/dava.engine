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


#include "BSDiff.h"
#include "ZLibStream.h"
#include "FileSystem/File.h"

namespace DAVA
{

bool BSDiff::Diff(char8 *origData, uint32 origSize, char8 *newData, uint32 newSize, File *patchFile, BSType type)
{
    bool ret = false;
    ZLibOStream outStream(patchFile);

    if(NULL != patchFile)
    {
        // write BS type
        uint32 typeToWrite = type;
        patchFile->Write(&typeToWrite);

        bsdiff_stream diffStream;
        diffStream.type = type;
        diffStream.free = &BSDiff::BSFree;
        diffStream.malloc = &BSDiff::BSMalloc;
        diffStream.write = &BSDiff::BSWrite;

        switch(type)
        {
            case BS_ZLIB:
                diffStream.opaque = &outStream;
                break;
            case BS_PLAIN:
                diffStream.opaque = patchFile;
                break;
            default:
                DVASSERT(0 && "Unknow BS-type");
                break;
        }

        // make bsdiff 
        if(0 == bsdiff((uint8_t *) origData, origSize, (uint8_t *) newData, newSize, &diffStream))
        {
            ret = true;
        }
    }

    return ret;
}

// This function should be as safe as possible. 
// So we should continue to work event after DVASSERT
bool BSDiff::Patch(char8 *origData, uint32 origSize, char8 *newData, uint32 newSize, File *patchFile)
{
	bool ret = false;
	ZLibIStream inStream(patchFile);

	// read BS type
	uint32 typeToRead = -1;
	if(sizeof(typeToRead) == patchFile->Read(&typeToRead))
	{
		bool type_is_ok = true;
		bspatch_stream patchStream;
		patchStream.read = &BSDiff::BSRead;
		patchStream.type = (BSType) typeToRead;

		switch(typeToRead)
		{
			case BS_ZLIB:
				patchStream.opaque = &inStream;
				break;
			case BS_PLAIN:
				patchStream.opaque = patchFile;
				break;
			default:
				DVASSERT(0 && "Unknow BS-type");
				type_is_ok = false;
				break;
		}

		if(type_is_ok)
		{
			// apply bsdiff
			if(0 == bspatch((uint8_t *)origData, origSize, (uint8_t *)newData, newSize, &patchStream))
			{
				ret = true;
			}
		}
	}

    return ret;
}

void* BSDiff::BSMalloc(int64_t size)
{
    return new uint8_t[(size_t) size];
}

void BSDiff::BSFree(void* ptr)
{
    if(NULL != ptr)
    {
        delete[] (uint8_t *) ptr;
    }
}

int BSDiff::BSWrite(struct bsdiff_stream* stream, const void* buffer, int64_t size)
{
    int ret = 0;

    if(stream->type == BS_PLAIN)
    {
        File *file = (File *) stream->opaque;
        if(size != file->Write((char8 *) buffer, (uint32) size))
        {
            ret = -1;
        }
    }
    else if(stream->type == BS_ZLIB)
    {
        ZLibOStream *outStream = (ZLibOStream *) stream->opaque;
        if(size != outStream->Write((char8 *) buffer, (uint32) size))
        {
            ret = -1;
        }
    }
    else
    {
        DVASSERT(0 && "Unknow BS-type");
        ret = -1;
    }

    return ret;
}

// This function should be as safe as possible. 
// So we should continue to work event after DVASSERT
int BSDiff::BSRead(const struct bspatch_stream* stream, void* buffer, int64_t size)
{
    int ret = 0;

    if(stream->type == BS_PLAIN)
    {
        File *file = (File *) stream->opaque;
        if(size != file->Read((char8 *) buffer, (uint32) size))
        {
            ret = -1;
        }
    }
    else if(stream->type == BS_ZLIB)
    {
        ZLibIStream *inStream = (ZLibIStream *) stream->opaque;
        if(size != inStream->Read((char8 *) buffer, (uint32) size))
        {
            ret = -1;
        }
    }
    else
    {
        DVASSERT(0 && "Unknow BS-type");
        ret = -1;
    }

    return ret;
}

}