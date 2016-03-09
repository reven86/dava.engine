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

#include "CommandLine/SceneUtils/SceneUtils.h"
#include "TextureCompression/TextureConverter.h"

namespace TextureDescriptorUtils
{
void ResaveDescriptorsForFolder(const DAVA::FilePath& folder);
void ResaveDescriptor(const DAVA::FilePath& descriptorPath);

void CreateDescriptorsForFolder(const DAVA::FilePath& folder, const DAVA::FilePath& presetPath);
bool CreateOrUpdateDescriptor(const DAVA::FilePath& texturePath, const DAVA::FilePath& presetPath = DAVA::FilePath());

void SetCompressionParamsForFolder(const DAVA::FilePath& folder, const DAVA::Map<DAVA::eGPUFamily, DAVA::TextureDescriptor::Compression>& compressionParams, bool convertionEnabled, bool force, DAVA::TextureConverter::eConvertQuality quality, bool generateMipMaps);
void SetCompressionParams(const DAVA::FilePath& descriptorPath, const DAVA::Map<DAVA::eGPUFamily, DAVA::TextureDescriptor::Compression>& compressionParams, bool convertionEnabled, bool force, DAVA::TextureConverter::eConvertQuality quality, bool generateMipMaps);

void SetPresetForFolder(const DAVA::FilePath& folder, const DAVA::FilePath& presetPath, bool toConvert, DAVA::TextureConverter::eConvertQuality quality);
void SetPreset(const DAVA::FilePath& descriptorPath, const DAVA::FilePath& presetPath, bool toConvert, DAVA::TextureConverter::eConvertQuality quality);
};



#endif // __TEXTURE_DESCRIPTOR_UTILS_H__