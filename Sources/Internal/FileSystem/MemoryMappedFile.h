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
#include "Base/BaseObject.h"

namespace DAVA
{
    
class MemoryMappedFile : public BaseObject
{
protected:
    virtual ~MemoryMappedFile();
    
public:
    MemoryMappedFile(const FilePath &path, uint32 size);
    MemoryMappedFile(const FilePath &path);
    MemoryMappedFile(uint32 size);
    
    uint8 * GetPointer(uint32 offset = 0);
    
    uint32 Read(void * pointerToData, uint32 dataSize);
    bool Seek(int32 position, uint32 seekType);
    bool IsEof();
    
    //! File seek enumeration
	enum eFileSeek
	{
		SEEK_FROM_START		= 1, //! Seek from start of file
		SEEK_FROM_END		= 2, //! Seek from end of file
		SEEK_FROM_CURRENT	= 3, //! Seek from current file position relatively
	};
    
private:
    uint8 *pointer;
    uint32 size;
    int fd;
    int32 currPos;
    
};
    
}

#endif /* defined(__Framework__MemoryMappedFile__) */
