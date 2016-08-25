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
    uint64 GetPos() const override;

    /**
     \brief Get current file size if writing
     \brief and get real file size if file for reading
     */
    uint64 GetSize() const override;

    /**
     \brief Set current file position
     \param position position to set
     \param seekType \ref IO::eFileSeek flag to set type of positioning
     \return true if successful otherwise false.
     */
    bool Seek(int64 position, eFileSeek seekType) override;

    //! return true if end of file reached and false in another case
    bool IsEof() const override;

private:
    uint32 GetRWOperationSize(uint32 dataSize) const;

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

inline uint64 StaticMemoryFile::GetPos() const
{
    return currentPos;
}

inline uint64 StaticMemoryFile::GetSize() const
{
    return memoryBufferSize;
}

inline bool StaticMemoryFile::IsEof() const
{
    return isEof;
}
};

#endif //__DAVAENGINE_STATIC_MEMORY_FILE_H__
