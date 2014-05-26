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

#ifndef __DAVAENGINE_MEMORY_MAPPED_FILE_H__
#define __DAVAENGINE_MEMORY_MAPPED_FILE_H__

#include "Base/BaseTypes.h"
#include "FileSystem/File.h"

namespace DAVA
{
    
class MemoryMappedFile : public File
{
protected:
    virtual ~MemoryMappedFile();
    
public:
    MemoryMappedFile(const FilePath &path, uint32 size);
    MemoryMappedFile(const FilePath &path);
    MemoryMappedFile(uint32 size);
    static MemoryMappedFile* Create(const FilePath &path);
    
    uint8 * GetPointer(uint32 offset = 0);
    
    uint32 Read(void * pointerToData, uint32 dataSize);
    uint8 *Read(uint32 dataSizeNeeded, uint32 &dataSizeRead);
    bool Seek(int32 position, uint32 seekType);
    bool IsEof();
    virtual	uint32 GetSize() { return size; }
    
    //! File seek enumeration
	enum eFileSeek
	{
		SEEK_FROM_START		= 1, //! Seek from start of file
		SEEK_FROM_END		= 2, //! Seek from end of file
		SEEK_FROM_CURRENT	= 3, //! Seek from current file position relatively
	};
    
private:
    uint8 *pointer;
    int fd;
    uint32 size;
    int32 currPos;    
};
    
}

#endif // __DAVAENGINE_MEMORY_MAPPED_FILE_H__
