//
//  MemoryMappedFile.cpp
//  Framework
//
//  Created by Vladimir Bondarenko on 2/19/14.
//
//

#include "FileSystem/MemoryMappedFile.h"

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

namespace DAVA
{

MemoryMappedFile::MemoryMappedFile(const FilePath& path, uint32 size)
    : size(size)
    , pointer(NULL)
    , fd(-1)
    , currPos(0)
    
{
    fd = open(path.GetAbsolutePathname().c_str(), O_RDONLY);
    if(fd != -1)
    {
        pointer = (uint8*)mmap(0, size, PROT_READ, MAP_PRIVATE, fd, 0);
    }
}

MemoryMappedFile::MemoryMappedFile(const FilePath& path)
    : size(0)
    , pointer(NULL)
    , fd(-1)
    , currPos(0)

{
    fd = open(path.GetAbsolutePathname().c_str(), O_RDONLY);
    if(fd != -1)
    {
        struct stat filestatus;
        stat( path.GetAbsolutePathname().c_str(), &filestatus );
        size = filestatus.st_size;
        pointer = (uint8*)mmap(0, size, PROT_READ, MAP_PRIVATE, fd, 0);
    }
}

MemoryMappedFile::MemoryMappedFile(uint32 size)
    : size(size)
    , pointer(NULL)
    , fd(-1)
    , currPos(0)

{
    pointer = (uint8*)mmap(0, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANON, fd, 0);
}

uint32 MemoryMappedFile::Read(void * pointerToData, uint32 dataSize)
{
    DVASSERT(fd != -1);
    Memcpy(pointerToData, pointer + currPos, dataSize);
    currPos += dataSize;
	return dataSize;
}

bool MemoryMappedFile::IsEof()
{
    return (currPos >= size);
}
    
bool MemoryMappedFile::Seek(int32 position, uint32 seekType)
{
    int32 pos = 0;
    switch(seekType)
    {
        case SEEK_FROM_START:
            currPos = position;
            break;
        case SEEK_FROM_CURRENT:
            currPos += position;
            break;
        case SEEK_FROM_END:
            currPos = size - 1 + position;
            break;
        default:
            return false;
    };
    
    
    if (currPos < 0)return false;
    if (currPos >= (int32)size)return false;
    

    return true;
}
    
MemoryMappedFile::~MemoryMappedFile()
{
    if(pointer != MAP_FAILED)
    {
        munmap(pointer, size);
        pointer = NULL;
    }
    if(fd != -1)
    {
        close(fd);
        fd = -1;
    }
}

uint8 * MemoryMappedFile::GetPointer(uint32 offset /*0*/)
{
    if(pointer != MAP_FAILED)
    {
        return (pointer + offset + currPos);
    }
    return NULL;
}
    
}