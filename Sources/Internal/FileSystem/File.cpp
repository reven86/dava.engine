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


#include "FileSystem/File.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/ResourceArchive.h"
#include "FileSystem/DynamicMemoryFile.h"

#include "Utils/StringFormat.h"

#if defined (__DAVAENGINE_WINDOWS__)
#include <io.h>
#elif defined (__DAVAENGINE_ANDROID__) || defined (__DAVAENGINE_MACOS__) || defined (__DAVAENGINE_IPHONE__)
#include <unistd.h>
#endif

#include <sys/stat.h>
#include <time.h>

namespace DAVA 
{

File::~File()
{
    // Though File object is created through Create methods returning nullptr on error
    // pointer should be checked against nullptr as class File can have inheritors
    // which do not initialize file pointer (e.g. DynamicMemoryFile)
    if (file != nullptr)
    {
        fclose(file);
        file = nullptr;
    }
}

File * File::Create(const FilePath &filePath, uint32 attributes)
{
    return FileSystem::Instance()->CreateFileForFrameworkPath(filePath, attributes);
}


File * File::CreateFromSystemPath(const FilePath &filename, uint32 attributes)
{
	FileSystem * fileSystem = FileSystem::Instance();
	for (List<FileSystem::ResourceArchiveItem>::iterator ai = fileSystem->resourceArchiveList.begin();
		ai != fileSystem->resourceArchiveList.end(); ++ai)
	{
		FileSystem::ResourceArchiveItem & item = *ai;

		String filenamecpp = filename.GetAbsolutePathname();

		String::size_type pos = filenamecpp.find(item.attachPath);
		if (pos == 0)
		{
			String relfilename = filenamecpp.substr(item.attachPath.length());
			int32 size = item.archive->LoadResource(relfilename, 0);
			if ( size == -1 )
			{
				return 0;
			}

			uint8 * buffer = new uint8[size];
			item.archive->LoadResource(relfilename, buffer);
			DynamicMemoryFile * file =  DynamicMemoryFile::Create(buffer, size, attributes);
            SafeDeleteArray(buffer);
			return file;
		}
	}
    
    bool isDirectory = FileSystem::Instance()->IsDirectory(filename);
    if(isDirectory)
    {
        return NULL;
    }
    
    return PureCreate(filename, attributes);
}

File * File::PureCreate(const FilePath & filePath, uint32 attributes)
{
    FILE * file = 0;
    uint32 size = 0;
    if((attributes & File::OPEN) && (attributes & File::READ))
    {
        if(attributes & File::WRITE)
        {
            file = fopen(filePath.GetAbsolutePathname().c_str(), "r+b");
        }
        else
        {
            file = fopen(filePath.GetAbsolutePathname().c_str(), "rb");
        }
        
        if (!file) return NULL;
        fseek(file, 0, SEEK_END);
        size = static_cast<uint32>(ftell(file));
        fseek(file, 0, SEEK_SET);
    }
    else if ((attributes & File::CREATE) && (attributes & File::WRITE))
    {
        file = fopen(filePath.GetAbsolutePathname().c_str(), "wb");
        if (!file)return NULL;
    }
    else if ((attributes & File::APPEND) && (attributes & File::WRITE))
    {
        file = fopen(filePath.GetAbsolutePathname().c_str(), "ab");
        if (!file)return NULL;
        fseek(file, 0, SEEK_END);
        size = static_cast<uint32>(ftell(file));
    }
    else 
    {
        return nullptr;
    }
    
    
    File * fileInstance = new File();
    fileInstance->filename = filePath;
    fileInstance->size = size;
    fileInstance->file = file;
    return fileInstance;
}
    

const FilePath & File::GetFilename()
{
	return filename;
}

uint32 File::Write(const void * pointerToData, uint32 dataSize)
{
#if defined(__DAVAENGINE_ANDROID__) 
    uint32 posBeforeWrite = GetPos();
#endif

    //! Do not change order fread return not bytes -- items
	uint32 lSize = (uint32) fwrite(pointerToData, 1, dataSize, file);

#if defined(__DAVAENGINE_ANDROID__) 
    //for Android value returned by 'fwrite()' is incorrect in case of full disk, that's why we calculate 'lSize' using 'GetPos()'
    lSize = GetPos() - posBeforeWrite;
#endif

	size += lSize;

    return lSize;
}

uint32 File::Read(void * pointerToData, uint32 dataSize)
{
	//! Do not change order (1, dataSize), cause fread return count of size(2nd param) items
	//! May be performance issues
	return (uint32) fread(pointerToData, 1, dataSize, file);
}

uint32 File::ReadString(char8 * destinationBuffer, uint32 destinationBufferSize)
{
	uint32 writeIndex = 0;
	uint8 currentChar = 0;

    if(destinationBufferSize > 0)
    {
	    while(Read(&currentChar, 1) > 0)
	    {
		    if(writeIndex < destinationBufferSize)
		    {
			    destinationBuffer[writeIndex] = currentChar;
        		writeIndex++;
		    }
            else 
		    {
                currentChar = 0;
			    Logger::Warning("File::ReadString buffer size is too small for this string.");
		    }

            if(currentChar == 0)
            {
                writeIndex--;
                break;	
            }
	    }

        destinationBuffer[writeIndex] = 0;
    }

	return writeIndex;
}
    
uint32 File::ReadString(String & destinationString)
{
    uint32 writeIndex = 0;
	while(!IsEof())
	{
		uint8 currentChar;
		Read(&currentChar, 1);
		
        if(0 != currentChar)
        {
	    	destinationString += currentChar;
    		writeIndex++;
        }
        else
        {
    		break;
        }
	}
	return writeIndex - 1;
}


uint32 File::ReadLine(void * pointerToData, uint32 bufferSize)
{
    uint32 ret = 0;

    if(bufferSize > 0)
    {
        uint8 *inPtr = (uint8 *) pointerToData;
        while(!IsEof() && bufferSize > 1)
        {
            uint8 nextChar;
            if(GetNextChar(&nextChar))
            {
                *inPtr = nextChar;
                inPtr++;
                bufferSize--;
            }
            else
            {
                break;
            }
        }
        *inPtr = 0;
        inPtr++;
        ret = (uint32)(inPtr - (uint8*)pointerToData);
    }

    return ret;
}

String File::ReadLine()
{
    String destinationString;
    while (!IsEof())
    {
        uint8 nextChar;
        if (GetNextChar(&nextChar))
        {
            destinationString += nextChar;
        }
        else
        {
            break;
        }
    }
    return destinationString;
}


bool File::GetNextChar(uint8 *nextChar)
{
    uint32 actuallyRead = Read(nextChar, 1);
    if (actuallyRead != 1)
    {
        //seems IsEof()
        return false;
    }

    if (0 == *nextChar)
    {
        // 0 terminated string
        return false;
    }
    else if ('\r' == *nextChar)
    {
        // we don't need to return \r as a charracter
        return GetNextChar(nextChar);
    }
    else if ('\n' == *nextChar)
    {
        // there was a last charracter in string ended by \n, then we cannot read more
        return false;
    }
    else
    {
        // some regular charracter readed
        return true;
    }
}

uint32 File::GetPos()
{
    return static_cast<uint32>(ftell(file));
}

uint32 File::GetSize()
{
    return size;
}

bool File::Seek(int32 position, uint32 seekType) 
{
	int realSeekType = 0;
	switch(seekType)
	{
		case SEEK_FROM_START:
			realSeekType = SEEK_SET;
			break;
		case SEEK_FROM_CURRENT:
			realSeekType = SEEK_CUR;
			break;
		case SEEK_FROM_END:
			realSeekType = SEEK_END;
			break;
        default:
            DVASSERT(0 && "Invalid seek type");
            break;
    }
    return 0 == fseek(file, position, realSeekType);
}

bool File::Flush()
{
    return 0 == fflush(file);
}

bool File::IsEof()
{
    return (feof(file) != 0);
}

bool File::Truncate(int32 size)
{
#if defined (__DAVAENGINE_WINDOWS__)
    return (0 == _chsize(_fileno(file), size));
#elif defined (__DAVAENGINE_MACOS__) || defined (__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)
    return (0 == ftruncate(fileno(file), size));
#else
#error No implementation for current platform
    return false;
#endif
}

bool File::WriteString(const String & strtowrite, bool shouldNullBeWritten)
{
	const char * str = strtowrite.c_str();
    uint32 null = (shouldNullBeWritten) ? (1) : (0);
	return (Write((void*)str, (uint32)(strtowrite.length() + null)) == strtowrite.length() + null);
}
    
bool File::WriteNonTerminatedString(const String & strtowrite)
{
    const char * str = strtowrite.c_str();
    return (Write((void*)str, (uint32)(strtowrite.length() )) == strtowrite.length() );
}

bool File::WriteLine(const String & string)
{
	uint32 written = 0;
	const char * str = string.c_str();
	const char * endLine = "\r\n";
	uint32 endLength = (uint32)strlen(endLine);
	uint32 strLength = (uint32)string.length();

	written += Write(str, strLength);
	written += Write(endLine, endLength);

	return (written == strLength+endLength);

}

String File::GetModificationDate(const FilePath & filePathname)
{
    String realPathname = filePathname.GetAbsolutePathname();
    
    struct stat fileInfo = {0};
    int32 ret = stat(realPathname.c_str(), &fileInfo);
    if(0 == ret)
    {
#if defined (__DAVAENGINE_WINDOWS__)
		tm* utcTime = gmtime(&fileInfo.st_mtime);
#elif defined (__DAVAENGINE_ANDROID__)
		tm* utcTime = gmtime((const time_t *)&fileInfo.st_mtime);
#elif defined (__DAVAENGINE_MACOS__) || defined (__DAVAENGINE_IPHONE__)
        tm* utcTime = gmtime(&fileInfo.st_mtimespec.tv_sec);
#endif
        return String(Format("%04d.%02d.%02d %02d:%02d:%02d",
                       utcTime->tm_year + 1900, utcTime->tm_mon + 1, utcTime->tm_mday,
                       utcTime->tm_hour, utcTime->tm_min, utcTime->tm_sec));
    }
    return String("");
}

}
