//
//  MemoryMappedFile.h
//  Framework
//
//  Created by Vladimir Bondarenko on 2/19/14.
//
//

#ifndef __Framework__MemoryMappedFile__
#define __Framework__MemoryMappedFile__

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

#endif /* defined(__Framework__MemoryMappedFile__) */
