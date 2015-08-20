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


#ifndef __TEXTURE_DESCRIPTOR_UTILS_H__
#define __TEXTURE_DESCRIPTOR_UTILS_H__

#include "DAVAEngine.h"
#include "CommandLine/SceneUtils/SceneUtils.h"
#include "TextureCompression/TextureConverter.h"


//using namespace DAVA;

class TextureDescriptorUtils
{
public:
    static void ResaveDescriptorsForFolder(const DAVA::FilePath &folderPathname);
	static void CopyCompressionParamsForFolder(const DAVA::FilePath &folderPathname);
    static void CreateDescriptorsForFolder(const DAVA::FilePath &folderPathname);
	static void SetCompressionParamsForFolder(const DAVA::FilePath &folderPathname, const DAVA::Map<DAVA::eGPUFamily, DAVA::TextureDescriptor::Compression> & compressionParams, bool convertionEnabled, bool force, DAVA::TextureConverter::eConvertQuality quality, bool generateMipMaps);

	static void SetCompressionParams(const DAVA::FilePath &descriptorPathname, const DAVA::Map<DAVA::eGPUFamily, DAVA::TextureDescriptor::Compression> & compressionParams, bool convertionEnabled, bool force, DAVA::TextureConverter::eConvertQuality quality, bool generateMipMaps);
    static bool CreateDescriptorIfNeed(const DAVA::FilePath &originalPathname);
    
private:
    
	static void ResaveDescriptor(const DAVA::FilePath & descriptorPathname);
    static void CopyCompressionParams(const DAVA::FilePath &descriptorPathname);

	static bool IsCorrectDirectory(DAVA::FileList *fileList, const DAVA::int32 fileIndex);
	static bool IsDescriptorPathname(const DAVA::FilePath &pathname);
};



#endif // __TEXTURE_DESCRIPTOR_UTILS_H__