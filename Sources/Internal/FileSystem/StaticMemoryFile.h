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

#ifndef __DAVAENGINE_STATIC_MEMORY_FILE_H__
#define __DAVAENGINE_STATIC_MEMORY_FILE_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "FileSystem/File.h"

namespace DAVA
{
/**
	\ingroup filesystem
	\brief class to work with file read-write interface on memory buffer
 */

class StaticMemoryFile : public File
{
protected:
    StaticMemoryFile(uint8* data, uint32 dataSize, uint32 attributes);
    ~StaticMemoryFile() override;

public:
    /**
     \brief function to create a file instance with give attributes
     \param[in] data pointer to data to create file from
     \param[in] dataSize size of data to create file from
     \param[in] attributes combinations of eFileAttributes
     \returns file instance
     */
    static StaticMemoryFile* Create(uint8* data, uint32 dataSize, uint32 attributes);

    /**
     \brief returns pointer to the data contained in the memory file
     \returns pointer to the first byte of the file data if file is empty returns NULL
     */
    const uint8* GetData() const;

    /**
     \brief Write [dataSize] bytes to this file from [pointerToData]
     \param[in] pointerToData function get data from this pointer
     \param[in] dataSize size of data we want to write
     \returns number of bytes actually written
     */
    uint32 Write(const void* pointerToData, uint32 dataSize) override;

    /**
     \brief Read [dataSize] bytes from this file to [pointerToData]
     \param[in, out] pointerToData function write data to this pointer
     \param[in] dataSize size of data we want to read
     \return number of bytes actually read
     */
    uint32 Read(void* pointerToData, uint32 dataSize) override;

    /**
     \brief Get current file position
     */
    uint32 GetPos() const override;

    /**
     \brief Get current file size if writing
     \brief and get real file size if file for reading
     */
    uint32 GetSize() const override;

    /**
     \brief Set current file position
     \param position position to set
     \param seekType \ref IO::eFileSeek flag to set type of positioning
     \return true if successful otherwise false.
     */
    bool Seek(int32 position, uint32 seekType) override;

    //! return true if end of file reached and false in another case
    bool IsEof() const override;

private:
    uint32 GetRWOperationSize(uint32 dataSize) const;

protected:
    uint8* memoryBuffer = nullptr;
    uint32 memoryBufferSize = 0;
    uint32 currentPos = 0;

    uint32 fileAttributes = File::READ | File::OPEN;
    bool isEof = false;
};

inline const uint8* StaticMemoryFile::GetData() const
{
    return memoryBuffer;
}

inline uint32 StaticMemoryFile::GetPos() const
{
    return currentPos;
}

inline uint32 StaticMemoryFile::GetSize() const
{
    return memoryBufferSize;
}

inline bool StaticMemoryFile::IsEof() const
{
    return isEof;
}
};

#endif //__DAVAENGINE_STATIC_MEMORY_FILE_H__
