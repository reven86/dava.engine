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
#ifdef __DAVAENGINE_NACL__

#include "Render/Image/Image.h"
#include "Render/Image/ImageSystem.h"
#include "Render/Image/LibDdsHelper.h"

#include "Render/Texture.h"
#include "Render/RenderManager.h"
#include "Render/PixelFormatDescriptor.h"


#include "FileSystem/File.h"
#include "FileSystem/FileSystem.h"

#include "Utils/Utils.h"
#include "Utils/CRC32.h"

//#include <libatc/TextureConverter.h>
namespace DAVA
{
    LibDdsHelper::LibDdsHelper(){ }
    
     bool LibDdsHelper::IsImage(File *file) const{return false; }
    
     eErrorCode LibDdsHelper::ReadFile(File *infile, Vector<Image *> &imageSet, int32 baseMipMap ) const
     {
         return ERROR_FILE_FORMAT_INCORRECT;
     }
    
    //input data only in RGBA8888
     eErrorCode LibDdsHelper::WriteFile(const FilePath & fileName, const Vector<Image *> &imageSet, PixelFormat compressionFormat) const
    {
        return ERROR_FILE_FORMAT_INCORRECT;
    }
    
    //input data only in RGBA8888
     eErrorCode LibDdsHelper::WriteFileAsCubeMap(const FilePath & fileName, const Vector<Vector<Image *> > &imageSet, PixelFormat compressionFormat) const
    {
         return ERROR_FILE_FORMAT_INCORRECT;
    }
    
    uint32 LibDdsHelper::GetDataSize(File * file) const{
        return 0;
    }
    Size2i LibDdsHelper::GetImageSize(File *infile) const
    {
        return Size2i(0,0);
    }
    
    
    
     eErrorCode LibDdsHelper::ReadFile(File * file, Vector<Image*> &imageSet, int32 baseMipMap , bool forceSoftwareConvertation ){return ERROR_FILE_FORMAT_INCORRECT; }
    
     bool LibDdsHelper::DecompressImageToRGBA(const DAVA::Image & image, Vector<DAVA::Image*> &imageSet, bool forceSoftwareConvertation){return false; }
    
     uint32 LibDdsHelper::GetMipMapLevelsCount(const FilePath & fileName){return 0; }
     uint32 LibDdsHelper::GetMipMapLevelsCount(File * file){return 0; }
    
     bool LibDdsHelper::AddCRCIntoMetaData(const FilePath &filePathname) const{return false; }
     uint32 LibDdsHelper::GetCRCFromFile(const FilePath &filePathname) const{return 0; }
    
    
     PixelFormat GetPixelFormat(const FilePath & fileName){return FORMAT_INVALID; }
     PixelFormat GetPixelFormat(File * file){return FORMAT_INVALID; }
    
     bool GetTextureSize(const FilePath & fileName, uint32 & width, uint32 & height){return false; }
     bool GetTextureSize(File * file, uint32 & width, uint32 & height){return false; }
    
     bool GetCRCFromDDSHeader(const FilePath &filePathname, uint32* tag, uint32* outputCRC){return false; }
    
    //input data only in RGBA8888
     bool WriteDxtFile(const FilePath & fileNameOriginal, const Vector<Image *> &imageSet, PixelFormat compressionFormat){return false; }
     bool WriteAtcFile(const FilePath & fileNameOriginal, const Vector<Image *> &imageSet, PixelFormat compressionFormat){return false; }
    
     bool WriteDxtFileAsCubemap(const FilePath & fileNameOriginal, const Vector<Vector<Image *> > &imageSet, PixelFormat compressionFormat){return false; }
     bool WriteAtcFileAsCubemap(const FilePath & fileNameOriginal, const Vector<Vector<Image *> > &imageSet, PixelFormat compressionFormat){return false; }


};

#endif //#ifndef __DAVAENGINE_NACL__