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

LightmapsPacker::LightmapsPacker()
{
}

void LightmapsPacker::ParseSpriteDescriptors()
{
	FileList * fileList = new FileList(outputDir);

	char8 buf[512];
	uint32 readSize; 

	int32 itemsCount = fileList->GetCount();
	for(int32 i = 0; i < itemsCount; ++i)
	{
		const FilePath & filePath = fileList->GetPathname(i);
		if(fileList->IsDirectory(i) || !filePath.IsEqualToExtension(".txt"))
		{
			continue;
		}

		LightmapAtlasingData data;

		data.meshInstanceName = filePath.GetBasename();
        
		File * file = File::Create(filePath, File::OPEN | File::READ);
		
		file->ReadLine(buf, sizeof(buf)); //textures count

		readSize = file->ReadLine(buf, sizeof(buf)); //texture name
		FilePath originalTextureName = outputDir + String(buf, readSize);
		data.textureName = originalTextureName;

		file->ReadLine(buf, sizeof(buf)); //image size

		file->ReadLine(buf, sizeof(buf)); //frames count

		file->ReadLine(buf, sizeof(buf)); //frame rect
		int32 x, y, dx, dy, unused0, unused1, unused2;
		sscanf(buf, "%d %d %d %d %d %d %d", &x, &y, &dx, &dy, &unused0, &unused1, &unused2);
		dx++;//cause TexturePacker::ReduceRectToOriginalSize removed one pixel by default
		dy++;

		Vector2 textureSize = GetTextureSize(originalTextureName);
		data.uvOffset = Vector2((float32)x/textureSize.x, (float32)y/textureSize.y);
		data.uvScale = Vector2((float32)dx/textureSize.x, (float32)dy/textureSize.y);
		
		file->Release();

		atlasingData.push_back(data);

		FileSystem::Instance()->DeleteFile(filePath);
	}

	fileList->Release();
}

Vector2 LightmapsPacker::GetTextureSize(const FilePath & filePath)
{
	Vector2 ret;

	FilePath sourceTexturePathname = FilePath::CreateWithNewExtension(filePath, TextureDescriptor::GetLightmapTextureExtension());

	Image * image = CreateTopLevelImage(sourceTexturePathname);
    if(image)
    {
        ret.x = (float32)image->GetWidth();
        ret.y = (float32)image->GetHeight();
        
        SafeRelease(image);
    }

	return ret;
}

Vector<LightmapAtlasingData> * LightmapsPacker::GetAtlasingData()
{
	return &atlasingData;
}

void LightmapsPacker::CreateDescriptors()
{
	FileList * fileList = new FileList(outputDir);

	int32 itemsCount = fileList->GetCount();
	for(int32 i = 0; i < itemsCount; ++i)
	{
		const FilePath & filePath = fileList->GetPathname(i);
		if(fileList->IsDirectory(i) || !filePath.IsEqualToExtension(".png"))
		{
			continue;
		}

		TextureDescriptor *descriptor = new TextureDescriptor();
		descriptor->Save(TextureDescriptor::GetDescriptorPathname(filePath));
		delete descriptor;
	}

	fileList->Release();
}
