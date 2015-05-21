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



#include "Render/Highlevel/Heightmap.h"
#include "Render/Image/Image.h"
#include "Render/Image/ImageSystem.h"
#include "FileSystem/File.h"
#include "FileSystem/FileSystem.h"
#include "Utils/Utils.h"


namespace DAVA
{

Heightmap::Heightmap()
    :   data(NULL)
    ,   size(0)
    ,   tileSize(0)
{
    //TODO: remove it. Used only for test
    SetTileSize(17);
}

Heightmap::~Heightmap()
{
    ReleaseData();
}

void Heightmap::ReleaseData()
{
    SafeDeleteArray(data);
    size = 0;
}
    
bool Heightmap::BuildFromImage(const DAVA::Image *image)
{
    DVASSERT(image);
    
    Image *heightImage = Image::CreateFromData(image->GetWidth(), image->GetHeight(), image->GetPixelFormat(), image->GetData());
    heightImage->FlipVertical();
    
    if(size != heightImage->width)
    {
        ReleaseData();
        AllocateData(heightImage->width);
    }
    
    if(FORMAT_A16 == heightImage->format)
    {
        uint16 *dstData = data;
        uint16 *srcData = (uint16*)heightImage->data;
        for(int32 i = size*size - 1; i >= 0; --i)
        {
            uint16 packed = *srcData++;
            uint16 unpacked = ((packed & 0xFF) << 8) | ((packed & 0xFF00) >> 8);
            
            *dstData++ = unpacked;
        }
    }
    else if(FORMAT_A8 == heightImage->format)
    {
        uint16 *dstData = data;
        uint8 *srcData = heightImage->data;

        for(int32 i = size*size - 1; i >= 0; --i)
        {
            *dstData++ = *srcData++ * IMAGE_CORRECTION;
        }
    }
    else 
    {
        Logger::Error("Heightmap build from wrong formatted image: format = %d", heightImage->format);
        return false;
    }
    
    
    SafeRelease(heightImage);
    return true;
}

void Heightmap::SaveToImage(const FilePath & filename)
{
    Image * image = Image::Create(size, size, FORMAT_A16);
    
    uint16 * unpackedBytes = new uint16[size * size];

    for (int32 k = 0; k < size * size; ++k)
    {
        unpackedBytes[k] = ((data[k] & 0xFF) << 8) | ((data[k] & 0xFF00) >> 8);
    }
    Memcpy(image->data, unpackedBytes, size*size*sizeof(uint16));
    image->FlipVertical();

    SafeDeleteArray(unpackedBytes);
    
    ImageSystem::Instance()->Save(filename, image, image->format);
    SafeRelease(image);
}
  
bool Heightmap::AllocateData(int32 newSize)
{
    size = newSize;
    data = new uint16[size * size];

    return (NULL != data);
}

    
    
uint16 * Heightmap::Data()
{
    return data;
}

int32 Heightmap::Size() const
{
    return size;
}
    
int32 Heightmap::GetTileSize() const
{
    return tileSize;
}

void Heightmap::SetTileSize(int32 newSize)
{
    tileSize = newSize;
}

void Heightmap::Save(const FilePath &filePathname)
{
    if (0 == size)
    {
        Logger::Error("Heightmap::Save: size is empty");
        return;
    }

    if(!filePathname.IsEqualToExtension(FileExtension()))
    {
        Logger::Error("Heightmap::Save wrong extension: %s", filePathname.GetAbsolutePathname().c_str());
        return;
    }

    
    File * file = File::Create(filePathname, File::CREATE | File::WRITE);
    if (!file)
    {
        Logger::Error("Heightmap::Save failed to create file: %s", filePathname.GetAbsolutePathname().c_str());
        return;
    }
    
    file->Write(&size, sizeof(size));
    file->Write(&tileSize, sizeof(tileSize));

    if(size && tileSize)
    {
        int32 blockCount = (size-1) / (tileSize - 1);
        for(int32 iRow = 0; iRow < blockCount; ++iRow)
        {
            for(int32 iCol = 0; iCol < blockCount; ++iCol)
            {
                int32 tileY = iRow * size * (tileSize - 1);
                int32 tileX = iCol * (tileSize - 1);
                for(int32 iTileRow = 0; iTileRow < tileSize; ++iTileRow, tileY += size)
                {
                    file->Write(data + tileY + tileX, tileSize * sizeof(data[0]));
                }
            }
        }
    }
    
    SafeRelease(file);
}
    
bool Heightmap::Load(const FilePath &filePathname)
{
    if(!filePathname.IsEqualToExtension(FileExtension()))
    {
        Logger::Error("Heightmap::Load failed with wrong extension: %s", filePathname.GetAbsolutePathname().c_str());
        return false;
    }

    
    File * file = File::Create(filePathname, File::OPEN | File::READ);
    if (!file)
    {
        Logger::Error("Heightmap::Load failed to create file: %s", filePathname.GetAbsolutePathname().c_str());
        return false;
    }
    
    int32 newSize = 0;
    
    file->Read(&newSize, sizeof(newSize));
    file->Read(&tileSize, sizeof(tileSize));
    
    if(size != newSize)
    {
        ReleaseData();
        AllocateData(newSize);
    }

    if(size && tileSize)
    {
        int32 blockCount = (size-1) / (tileSize - 1);
        for(int32 iRow = 0; iRow < blockCount; ++iRow)
        {
            for(int32 iCol = 0; iCol < blockCount; ++iCol)
            {
                int32 tileY = iRow * size * (tileSize - 1);
                int32 tileX = iCol * (tileSize - 1);
                for(int32 iTileRow = 0; iTileRow < tileSize; ++iTileRow, tileY += size)
                {
                    file->Read(data + tileY + tileX, tileSize * sizeof(data[0]));
                }
            }
        }
    }
    
    SafeRelease(file);
    return true;
}

const String Heightmap::FileExtension()
{
    return ".heightmap";
}
    
    
Heightmap * Heightmap::Clone(DAVA::Heightmap *clonedHeightmap)
{
    Heightmap *createdHeightmap = clonedHeightmap;
    if(createdHeightmap)
    {
        if(createdHeightmap->Size() != size)
        {
            createdHeightmap->ReleaseData();
            createdHeightmap->AllocateData(size);
            
            if(!createdHeightmap->Data())
            {
                return NULL;
            }
        }
    }
    else
    {
        createdHeightmap = CreateHeightmapForSize(size);
        if(!createdHeightmap)   return NULL;
    }
    
    memmove(createdHeightmap->data, data, size * size * sizeof(uint16));
    createdHeightmap->SetTileSize(tileSize); //TODO: is it true?

    return createdHeightmap;
}
    
Heightmap * Heightmap::CreateHeightmapForSize(int32 newSize)
{
    Heightmap *createdHeightmap = new Heightmap();
    if(!createdHeightmap)   return NULL;
    
    createdHeightmap->AllocateData(newSize);
    
    if(!createdHeightmap->data)
    {
        SafeRelease(createdHeightmap);
        return NULL;
    }

    return createdHeightmap;
}

};

