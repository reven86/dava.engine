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



#include "DLCUnpacker.h"
#include "libpng/zlib.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/File.h"
#include "FileSystem/KeyedArchive.h"
#include "FileSystem/ResourceArchive.h"

#include <sstream>
#include <iostream>

namespace DAVA
{
    
void DLCUnpacker::Unpack(const FilePath & filePathSrc, const FilePath & unpackPath, const FilePath & fileForPaths)
{
    Logger::FrameworkDebug("DLC Complete");
    
    FilePath filePathDest = FilePath::CreateWithNewExtension(filePathSrc, "");
    
    File* sFile = File::Create( filePathSrc, File::OPEN|File::READ );
    File* dFile = File::Create( filePathDest, File::CREATE|File::WRITE);
    
    int32 result = DLCUnpacker::Inflate(sFile, dFile);
    
    SafeRelease(sFile);
    SafeRelease(dFile);
    
    if ( result != Z_OK )
    {
        return ;
    }
    
    ResourceArchive * resArchive = new ResourceArchive();
    result = resArchive->Open(filePathDest.GetAbsolutePathname());

    // Save paths for check all files every start app
    KeyedArchive * archive = NULL;
    if (!fileForPaths.IsEmpty())
    {
        archive = new KeyedArchive();
        archive->SetUInt32("size", resArchive->GetFileCount());
    }
    
    for (int32 i = 0; i < resArchive->GetFileCount(); ++i)
    {
        FilePath filePath = unpackPath + resArchive->GetResourcePathname(i);
        
        result = FileSystem::Instance()->CreateDirectory(filePath.GetDirectory(), true);
        
        File *wFile = File::Create(filePath, File::CREATE | File::WRITE);
        int size = resArchive->LoadResource(i, NULL);
        char* buff = new char[size];
        result = resArchive->LoadResource(i, buff);
        wFile->Write(buff, size);
        wFile->Release();
        delete[] buff;
        
        // KeyedArchive with paths , key is index
        if ( archive != NULL )
        {
            String key = "";
            String value = resArchive->GetResourcePathname(i);
            std::stringstream stream;
            stream << i;
            stream >> key;
            archive->SetString( key, value );
        }
    }
    
    if ( archive != NULL )
    {
        archive->Save(fileForPaths.GetAbsolutePathname());
        SafeRelease(archive);
    }
    
    SafeRelease(resArchive);
    FileSystem::Instance()->DeleteFile(filePathDest);
}
    
/* Decompress from file source to file dest until stream ends or EOF.
 inf() returns Z_OK on success, Z_MEM_ERROR if memory could not be
 allocated for processing, Z_DATA_ERROR if the deflate data is
 invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
 the version of the library linked do not match, or Z_ERRNO if there
 is an error reading or writing the files. */

int32 DLCUnpacker::Inflate(File *source, File *dest)
{
    int32 ret;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];
    
    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);
    if (ret != Z_OK)
        return ret;
    
    /* decompress until deflate stream ends or end of file */
    do
    {
        strm.avail_in = source->Read(in, CHUNK);
        if (strm.avail_in == 0)
            break;
        strm.next_in = in;
        
        /* run inflate() on input until output buffer not full */
        do
        {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
//            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            switch (ret)
            {
                case Z_NEED_DICT:
                    ret = Z_DATA_ERROR;     /* and fall through */
                case Z_DATA_ERROR:
                case Z_MEM_ERROR:
                    (void)inflateEnd(&strm);
                    return ret;
            }
            have = CHUNK - strm.avail_out;
            if ( dest->Write(out, have) != have )
            {
                (void)inflateEnd(&strm);
                return Z_ERRNO;
            }
        }
        while (strm.avail_out == 0);
        
        /* done when inflate() says it's done */
    }
    while (ret != Z_STREAM_END);
    
    /* clean up and return */
    (void)inflateEnd(&strm);
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}
    
    
}