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


#include "FileSystem/StaticMemoryFile.h"
#include "Utils/StringFormat.h"

namespace DAVA 
{

StaticMemoryFile * StaticMemoryFile::Create(uint8 *data, uint32 dataSize, uint32 attributes)
{
    StaticMemoryFile *fl = new StaticMemoryFile(data, dataSize, attributes);
	fl->filename = Format("memoryfile_%p", static_cast<void*>(fl));
	
	return fl;
}


StaticMemoryFile::StaticMemoryFile(uint8 *data, uint32 dataSize, uint32 attributes)
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
    currentPtr = 0;
}
	
uint32 StaticMemoryFile::Write(const void * pointerToData, uint32 dataSize)
{
    DVASSERT(nullptr != pointerToData);

	if (!(fileAttributes & File::WRITE) && !(fileAttributes & File::APPEND))
	{
		return 0;
	}
	
    uint32 written = 0;
	if(dataSize > 0)
	{
        uint32 freeSpace = memoryBufferSize - currentPtr;
        written = (freeSpace >= dataSize) ? dataSize : freeSpace;

 		Memcpy(memoryBuffer + currentPtr, pointerToData, written);
        
        currentPtr += written;
	}
	
    return written;
}

uint32 StaticMemoryFile::Read(void * pointerToData, uint32 dataSize)
{
    DVASSERT(nullptr != pointerToData);

	if (!(fileAttributes & File::READ))
	{
		return 0;
	}

    uint32 read = 0;
    if (dataSize > 0)
    {
        uint32 space = memoryBufferSize - currentPtr;
        read = (space >= dataSize) ? dataSize : space;

        Memcpy(pointerToData, memoryBuffer + currentPtr, read);

        currentPtr += read;
    }

    return read;
}


bool StaticMemoryFile::Seek(int32 position, uint32 seekType)
{
	int32 pos = 0;
	switch(seekType)
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


};
