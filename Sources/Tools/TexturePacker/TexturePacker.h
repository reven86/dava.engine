#ifndef __DAVAENGINE_TEXTURE_PACKER_H__
#define __DAVAENGINE_TEXTURE_PACKER_H__

#include "Base/BaseTypes.h"
#include "Render/RenderBase.h"
#include "Render/Texture.h"
#include "Math/Math2D.h"

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

	static const int32 TEXTURE_SIZE = 2048;
	
	struct FilterItem
	{
		int8 minFilter;
		int8 magFilter;
		
		FilterItem(int8 minF, int8 magF)
		{
			minFilter = minF;
			magFilter = magF;
		}
	};

public:
	TexturePacker();
	
	// pack textures to single texture
	void PackToTextures(const FilePath & excludeFolder, const FilePath & outputPath, List<DefinitionFile*> & defsList, eGPUFamily forGPU);
	// page each PSD file to separate texture
	void PackToTexturesSeparate(const FilePath & excludeFolder, const FilePath & outputPath, List<DefinitionFile*> & defsList, eGPUFamily forGPU);
	// pack one sprite and use several textures if more than one needed
	void PackToMultipleTextures(const FilePath & excludeFolder, const FilePath & outputPath, List<DefinitionFile*> & remainingList, eGPUFamily forGPU);

	bool TryToPack(const Rect2i & textureRect, List<DefinitionFile*> & defsList);
	bool WriteDefinition(const FilePath & excludeFolder, const FilePath & outputPath, const String & textureName, DefinitionFile * defFile);
	bool WriteMultipleDefinition(const FilePath & excludeFolder, const FilePath & outputPath, const String & _textureName, DefinitionFile * defFile);

	int TryToPackFromSortVector(ImagePacker * packer, Vector<SizeSortItem> & tempSortVector);
	float TryToPackFromSortVectorWeight(ImagePacker * packer, Vector<SizeSortItem> & tempSortVector);

	Rect2i ReduceRectToOriginalSize(const Rect2i & _input);

	void UseOnlySquareTextures();

	void SetMaxTextureSize(int32 maxTextureSize);
	
private:
    
    void ExportImage(PngImageExt *image, const FilePath &exportedPathname, eGPUFamily forGPU);
    TextureDescriptor * CreateDescriptor(eGPUFamily forGPU);
	
	Texture::TextureWrap GetDescriptorWrapMode();
	FilterItem GetDescriptorFilter(bool generateMipMaps = false);
    
    bool CheckFrameSize(const Size2i &spriteSize, const Size2i &frameSize);
    
    
	ImagePacker *			lastPackedPacker;
	Vector<ImagePacker*> usedPackers;

	Vector<SizeSortItem> sortVector;
	int32 maxTextureSize;

	bool onlySquareTextures;
    bool NeedSquareTextureForCompression(eGPUFamily forGPU);
	bool IsFormatSupportedForGPU(PixelFormat format, eGPUFamily forGPU);
};

};


#endif // __DAVAENGINE_TEXTURE_PACKER_H__

