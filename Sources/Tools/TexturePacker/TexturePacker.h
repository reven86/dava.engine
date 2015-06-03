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


#ifndef __DAVAENGINE_TEXTURE_PACKER_H__
#define __DAVAENGINE_TEXTURE_PACKER_H__

#include "Base/BaseTypes.h"
#include "Render/RenderBase.h"
#include "Render/Texture.h"
#include "Math/Math2D.h"
#include "TextureCompression/TextureConverter.h"
#include "TexturePacker/ImagePacker.h"

namespace DAVA
{

class DefinitionFile;
class TextureDescriptor;
class PngImageExt;
class FilePath;
class ImagePacker;

struct SizeSortItem
{
	int					imageSize;
	DefinitionFile *	defFile;
	int					frameIndex;
};
    
class TexturePacker 
{
public:

	static const uint32 DEFAULT_TEXTURE_SIZE = 2048;
	static const uint32 TSIZE_4096 = 4096;
	static const Set<PixelFormat> PIXEL_FORMATS_WITH_COMPRESSION;
	static const uint32 DEFAULT_MARGIN = 1;

	struct FilterItem
	{
		int8 minFilter;
		int8 magFilter;
        int8 mipFilter;
		
        FilterItem(int8 minF, int8 magF, int8 mipF)
		{
			minFilter = minF;
			magFilter = magF;
            mipFilter = mipF;
		}
	};

public:
	TexturePacker();
	
	// pack textures to single texture
	void PackToTextures(const FilePath & excludeFolder, const FilePath & outputPath, List<DefinitionFile*> & defsList, eGPUFamily forGPU);
	// page each PSD file to separate texture
	void PackToTexturesSeparate(const FilePath & excludeFolder, const FilePath & outputPath, List<DefinitionFile*> & defsList, eGPUFamily forGPU);
	// pack one sprite and use several textures if more than one needed
	void PackToMultipleTextures(const FilePath & excludeFolder, const FilePath & outputPath, const char* basename, List<DefinitionFile*> & remainingList, eGPUFamily forGPU);

	bool TryToPack(const Rect2i & textureRect, List<DefinitionFile*> & defsList);
	bool WriteDefinition(const FilePath & excludeFolder, const FilePath & outputPath, const String & textureName, DefinitionFile * defFile);
	bool WriteMultipleDefinition(const FilePath & excludeFolder, const FilePath & outputPath, const String & _textureName, DefinitionFile * defFile);

	int TryToPackFromSortVector(ImagePacker * packer, Vector<SizeSortItem> & tempSortVector);
	float TryToPackFromSortVectorWeight(ImagePacker * packer, Vector<SizeSortItem> & tempSortVector);

    Rect2i GetOriginalSizeRect(const PackedInfo& _input);

	void UseOnlySquareTextures();

	void SetMaxTextureSize(uint32 maxTextureSize);
	
    void SetConvertQuality(TextureConverter::eConvertQuality quality);

	// set visible 1 pixel border for each texture
	void SetTwoSideMargin(bool val=true) { useTwoSideMargin = val; }

	// set space in pixels between two neighboring textures. value is omitted if two-side margin is set
	void SetTexturesMargin(uint32 margin) { texturesMargin = margin; }

	const Set<String>& GetErrors() const;
	
private:
    
    void ExportImage(PngImageExt *image, const FilePath &exportedPathname, eGPUFamily forGPU);
    TextureDescriptor * CreateDescriptor(eGPUFamily forGPU);
	
	rhi::TextureAddrMode GetDescriptorWrapMode();
	FilterItem GetDescriptorFilter(bool generateMipMaps = false);
    
    bool CheckFrameSize(const Size2i &spriteSize, const Size2i &frameSize);
    
	void WriteDefinitionString(FILE *fp, const Rect2i & writeRect, const Rect2i &originRect, int textureIndex, const String& frameName);
	void DrawToFinalImage(PngImageExt & finalImage, PngImageExt & drawedImage, const PackedInfo & drawRect, const Rect2i &frameRect);

    
	ImagePacker *			lastPackedPacker;
	Vector<ImagePacker*> usedPackers;

	Vector<SizeSortItem> sortVector;
	uint32 maxTextureSize;

	bool onlySquareTextures;
    bool NeedSquareTextureForCompression(eGPUFamily forGPU);
	
    TextureConverter::eConvertQuality quality;

	bool useTwoSideMargin;
	uint32 texturesMargin;
    
	Set<String> errors;
	void AddError(const String& errorMsg);

};

};


#endif // __DAVAENGINE_TEXTURE_PACKER_H__

