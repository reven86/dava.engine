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


#include "LightmapsPacker.h"

#include "Qt/Main/QtUtils.h"

void LightmapsPacker::ParseSpriteDescriptors()
{
    DAVA::FileList* fileList = new DAVA::FileList(outputDir);

    DAVA::UnorderedMap<DAVA::String, DAVA::Vector2> texturesSize;

    DAVA::int32 itemsCount = fileList->GetCount();
    for (DAVA::int32 i = 0; i < itemsCount; ++i)
    {
        const DAVA::FilePath& filePath = fileList->GetPathname(i);
        if (fileList->IsDirectory(i) || !filePath.IsEqualToExtension(".txt"))
        {
            continue;
        }

        LightmapAtlasingData data;

        data.meshInstanceName = filePath.GetBasename();

        DAVA::File* file = DAVA::File::Create(filePath, DAVA::File::OPEN | DAVA::File::READ);

        DAVA::char8 buf[512] = {};
        file->ReadLine(buf, sizeof(buf)); //textures count

        DAVA::uint32 readSize = file->ReadLine(buf, sizeof(buf)); //texture name
        DAVA::FilePath originalTextureName = outputDir + DAVA::String(buf, readSize);
        data.textureName = originalTextureName;

        file->ReadLine(buf, sizeof(buf)); //image size
        file->ReadLine(buf, sizeof(buf)); //frames count
        file->ReadLine(buf, sizeof(buf)); //frame rect

        DAVA::int32 x, y, dx, dy, unused0, unused1, unused2;
        sscanf(buf, "%d %d %d %d %d %d %d", &x, &y, &dx, &dy, &unused0, &unused1, &unused2);

        DAVA::String absoluteFileName = originalTextureName.GetAbsolutePathname();
        if (texturesSize.count(absoluteFileName) == 0)
        {
            texturesSize.emplace(absoluteFileName, GetTextureSize(originalTextureName));
        }

        DAVA::Vector2 textureSize = texturesSize[absoluteFileName];
        data.uvOffset = DAVA::Vector2(static_cast<DAVA::float32>(x) / textureSize.x, static_cast<DAVA::float32>(y) / textureSize.y);
        data.uvScale = DAVA::Vector2(static_cast<DAVA::float32>(dx) / textureSize.x, static_cast<DAVA::float32>(dy) / textureSize.y);

        file->Release();

        atlasingData.push_back(data);

        DAVA::FileSystem::Instance()->DeleteFile(filePath);
    }

    fileList->Release();
}

DAVA::Vector2 LightmapsPacker::GetTextureSize(const DAVA::FilePath& filePath)
{
    DAVA::Vector2 ret;

    DAVA::FilePath sourceTexturePathname = DAVA::FilePath::CreateWithNewExtension(filePath, DAVA::TextureDescriptor::GetLightmapTextureExtension());

    DAVA::Image* image = CreateTopLevelImage(sourceTexturePathname);
    if (image)
    {
        ret.x = static_cast<DAVA::float32>(image->GetWidth());
        ret.y = static_cast<DAVA::float32>(image->GetHeight());

        SafeRelease(image);
    }

    return ret;
}

DAVA::Vector<LightmapAtlasingData>* LightmapsPacker::GetAtlasingData()
{
    return &atlasingData;
}

void LightmapsPacker::CreateDescriptors()
{
    DAVA::FileList* fileList = new DAVA::FileList(outputDir);

    DAVA::int32 itemsCount = fileList->GetCount();
    for (DAVA::int32 i = 0; i < itemsCount; ++i)
    {
        const DAVA::FilePath& filePath = fileList->GetPathname(i);
        if (fileList->IsDirectory(i) || !filePath.IsEqualToExtension(".png"))
        {
            continue;
        }

        DAVA::TextureDescriptor* descriptor = new DAVA::TextureDescriptor();
        descriptor->Save(DAVA::TextureDescriptor::GetDescriptorPathname(filePath));
        delete descriptor;
    }

    fileList->Release();
}
