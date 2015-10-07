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


#include "TexturePacker/TexturePacker.h"
#include "TexturePacker/CommandLineParser.h"
#include "TexturePacker/ImagePacker.h"
#include "TexturePacker/PngImage.h"
#include "TexturePacker/DefinitionFile.h"
#include "Render/TextureDescriptor.h"
#include "Render/GPUFamilyDescriptor.h"
#include "Render/PixelFormatDescriptor.h"
#include "Render/Image/ImageSystem.h"
#include "Render/Image/ImageConvert.h"
#include "FileSystem/FileSystem.h"
#include "TextureCompression/TextureConverter.h"
#include "FramePathHelper.h"
#include "Utils/StringFormat.h"
#include "Base/GlobalEnum.h"


#ifdef WIN32
#define snprintf _snprintf
#endif

namespace DAVA
{

static Set<PixelFormat> InitPixelFormatsWithCompression()
{
    Set<PixelFormat> set;
    set.insert(FORMAT_PVR4);
    set.insert(FORMAT_PVR2);
    set.insert(FORMAT_DXT1);
    set.insert(FORMAT_DXT1A);
    set.insert(FORMAT_DXT3);
    set.insert(FORMAT_DXT5);
    set.insert(FORMAT_DXT5NM);
    set.insert(FORMAT_ETC1);
    set.insert(FORMAT_ATC_RGB);
    set.insert(FORMAT_ATC_RGBA_EXPLICIT_ALPHA);
    set.insert(FORMAT_ATC_RGBA_INTERPOLATED_ALPHA);
	set.insert(FORMAT_PVR2_2);
	set.insert(FORMAT_PVR4_2);
	set.insert(FORMAT_EAC_R11_UNSIGNED);
	set.insert(FORMAT_EAC_R11_SIGNED);
	set.insert(FORMAT_EAC_RG11_UNSIGNED);
	set.insert(FORMAT_EAC_RG11_SIGNED);
	set.insert(FORMAT_ETC2_RGB);
	set.insert(FORMAT_ETC2_RGBA);
	set.insert(FORMAT_ETC2_RGB_A1);

    return set;
}

const Set<PixelFormat> TexturePacker::PIXEL_FORMATS_WITH_COMPRESSION = InitPixelFormatsWithCompression();

TexturePacker::TexturePacker()
    : lastPackedPacker(nullptr)
{
	quality = TextureConverter::ECQ_VERY_HIGH;
	if (CommandLineParser::Instance()->IsFlagSet("--quality"))
	{
		String qualityName = CommandLineParser::Instance()->GetParamForFlag("--quality");
		int32 q = atoi(qualityName.c_str());
		if((q >= TextureConverter::ECQ_FASTEST) && (q <= TextureConverter::ECQ_VERY_HIGH))
		{
			quality = (TextureConverter::eConvertQuality)q;
		}
	}
    
	maxTextureSize = DEFAULT_TEXTURE_SIZE;
	onlySquareTextures = false;
	errors.clear();
	useTwoSideMargin = false;
	texturesMargin = 1;
}

TexturePacker::~TexturePacker()
{
    SafeDelete(lastPackedPacker);
    for (auto &imagePacker : usedPackers)
    {
        SafeDelete(imagePacker);
    }
}

bool TexturePacker::TryToPack(const Rect2i & textureRect, List<DefinitionFile*> & /*defsList*/)
{
	ImagePacker * packer = new ImagePacker(textureRect, useTwoSideMargin, texturesMargin);
	
	// Packing of sorted by size images
	for (int i = 0; i < (int)sortVector.size(); ++i)
	{
		DefinitionFile * defFile = sortVector[i].defFile;
		int frame = sortVector[i].frameIndex;

		if (!packer->AddImage(defFile->GetFrameSize(frame), &defFile->frameRects[frame]))
		{
			SafeDelete(packer);
			return false;
		}

        Logger::FrameworkDebug("p: %s %d",defFile->filename.GetAbsolutePathname().c_str(), frame);
	}
    Logger::FrameworkDebug("* %d x %d - success", textureRect.dx, textureRect.dy);
	
    SafeDelete(lastPackedPacker);
	
	lastPackedPacker = packer;
	return true;
}

int TexturePacker::TryToPackFromSortVector(ImagePacker * packer,Vector<SizeSortItem> & tempSortVector)
{
	int packedCount = 0;
	// Packing of sorted by size images
	for (int i = 0; i < (int)tempSortVector.size(); ++i)
	{
		DefinitionFile * defFile = tempSortVector[i].defFile;
		int frame = tempSortVector[i].frameIndex;
		if (packer->AddImage(defFile->GetFrameSize(frame), &defFile->frameRects[frame]))
		{
			packedCount++;
			tempSortVector.erase(tempSortVector.begin() + i);
			i--;
		}
	}
	return packedCount;
}

float TexturePacker::TryToPackFromSortVectorWeight(ImagePacker * packer,Vector<SizeSortItem> & tempSortVector)
{
	float weight = 0.0f;
	
	// Packing of sorted by size images
	for (int i = 0; i < (int)tempSortVector.size(); ++i)
	{
		DefinitionFile * defFile = tempSortVector[i].defFile;
		int frame = tempSortVector[i].frameIndex;
		if (packer->AddImage(defFile->GetFrameSize(frame), &defFile->frameRects[frame]))
		{
			weight += (defFile->GetFrameWidth(frame) * defFile->GetFrameHeight(frame));// * weightCoeff;
			tempSortVector.erase(tempSortVector.begin() + i);
			i--;
		}
	}
	return weight;
}


bool sortFn(const SizeSortItem & a, const SizeSortItem & b)
{
	return a.imageSize > b.imageSize;	
}

void TexturePacker::PackToTexturesSeparate(const FilePath & excludeFolder, const FilePath & outputPath, List<DefinitionFile*> & defsList, eGPUFamily forGPU)
{
	Logger::FrameworkDebug("Packing to separate textures");

    ImageExportKeys imageExportKeys = GetExportKeys(forGPU);

    SafeDelete(lastPackedPacker);
    for (auto& defFile : defsList)
	{
		sortVector.clear();
		
		for (int frame = 0; frame < defFile->frameCount; ++frame)
		{
			SizeSortItem sortItem;
			sortItem.imageSize = defFile->GetFrameWidth(frame) * defFile->GetFrameHeight(frame);
			sortItem.defFile = defFile;
			sortItem.frameIndex = frame;
			sortVector.push_back(sortItem);
		}
		std::stable_sort(sortVector.begin(), sortVector.end(), sortFn);

		
		// try to pack for each resolution
		uint32 bestResolution = (maxTextureSize) * (maxTextureSize);
		uint32 bestXResolution, bestYResolution;
		bool packWasSuccessfull = false;
		
        Logger::FrameworkDebug("* Packing attempts started: ");
		
		for (uint32 yResolution = 8; yResolution <= maxTextureSize; yResolution *= 2)
			for (uint32 xResolution = 8; xResolution <= maxTextureSize; xResolution *= 2)
			{
				Rect2i textureRect = Rect2i(0, 0, xResolution, yResolution);
				uint32 currentResolution = xResolution * yResolution;

				if ((currentResolution < bestResolution) || (currentResolution == bestResolution && !packWasSuccessfull))
				{
					if (TryToPack(textureRect, defsList))
					{
						packWasSuccessfull = true;
						bestResolution = currentResolution;
						bestXResolution = xResolution;
						bestYResolution = yResolution;
						break;
					}
				}
				else
					break;
			}

        Logger::FrameworkDebug("");

        String fileBasename = defFile->filename.GetBasename() + "_";

		if (packWasSuccessfull)
		{
            fileBasename.append("0");
            FilePath textureName = outputPath + fileBasename;
            Logger::FrameworkDebug("* Writing final texture (%d x %d): %s", bestXResolution, bestYResolution , textureName.GetAbsolutePathname().c_str());
			
			PngImageExt finalImage;
			finalImage.Create(bestXResolution, bestYResolution);
			
            String fileName = defFile->filename.GetFilename();

			// Writing 
			for (int frame = 0; frame < defFile->frameCount; ++frame)
			{
				PackedInfo *destRect = lastPackedPacker->SearchRectForPtr(&defFile->frameRects[frame]);
				if (!destRect)
				{
					AddError(Format("*** ERROR: Can't find rect for frame - %d. Definition - %s.",
									frame,
									fileName.c_str()));
				}
				else
				{
					FilePath withoutExt(defFile->filename);
					withoutExt.TruncateExtension();

					PngImageExt image;
					image.Read(FramePathHelper::GetFramePathRelative(withoutExt, frame));
					DrawToFinalImage(finalImage, image, *destRect, defFile->frameRects[frame]);
				}
			}
			
            if (!WriteDefinition(excludeFolder, outputPath, fileBasename, defFile))
			{
				AddError(Format("* ERROR: Failed to write definition - %s.", fileName.c_str()));
			}

            ExportImage(&finalImage, imageExportKeys, FilePath(textureName));
		}
        else
        {
            Logger::FrameworkDebug("Can't pack to separate output texture");
            List<DefinitionFile*> defList = {defFile};
            PackToMultipleTextures(excludeFolder, outputPath, fileBasename.c_str(), defList, forGPU);
        }
	}
}

void TexturePacker::PackToTextures(const FilePath & excludeFolder, const FilePath & outputPath, List<DefinitionFile*> & defsList, eGPUFamily forGPU)
{
	Logger::FrameworkDebug("Packing to single output texture");
    SafeDelete(lastPackedPacker);
	for (List<DefinitionFile*>::iterator dfi = defsList.begin(); dfi != defsList.end(); ++dfi)
	{
		DefinitionFile * defFile = *dfi;
		for (int frame = 0; frame < defFile->frameCount; ++frame)
		{
			SizeSortItem sortItem;
			sortItem.imageSize = defFile->GetFrameWidth(frame) * defFile->GetFrameHeight(frame);
			sortItem.defFile = defFile;
			sortItem.frameIndex = frame;
			sortVector.push_back(sortItem);
		}
	}

    ImageExportKeys imageExportKeys = GetExportKeys(forGPU);

	std::stable_sort(sortVector.begin(), sortVector.end(), sortFn);

	// try to pack for each resolution
	uint32 bestResolution = (maxTextureSize) * (maxTextureSize);
	uint32 bestXResolution = 0, bestYResolution = 0;
	bool packWasSuccessfull = false;
	
    Logger::FrameworkDebug("* Packing tries started: ");
	
    bool needOnlySquareTexture = onlySquareTextures || NeedSquareTextureForCompression(imageExportKeys);
	for (uint32 yResolution = 8; yResolution <= maxTextureSize; yResolution *= 2)
		 for (uint32 xResolution = 8; xResolution <= maxTextureSize; xResolution *= 2)
		 {
			 if (needOnlySquareTexture && (xResolution != yResolution))continue;
			 
			 Rect2i textureRect = Rect2i(0, 0, xResolution, yResolution);
			 uint32 currentResolution = xResolution * yResolution;

			 if (currentResolution < bestResolution || (currentResolution == bestResolution && !packWasSuccessfull))
			 {
				 if (TryToPack(textureRect, defsList))
				 {
					 packWasSuccessfull = true;
					 bestResolution = currentResolution;
					 bestXResolution = xResolution;
					 bestYResolution = yResolution;
					 break;
				 }
			 }
			 else
				 break;
		 }
    Logger::FrameworkDebug("\n");

	if ( packWasSuccessfull )
	{
		FilePath textureName = outputPath + "texture";
        Logger::FrameworkDebug("* Writing final texture (%d x %d): %s", bestXResolution, bestYResolution , textureName.GetAbsolutePathname().c_str());
	
		PngImageExt finalImage;
		finalImage.Create(bestXResolution, bestYResolution);
		
		// Writing 
		for (List<DefinitionFile*>::iterator dfi = defsList.begin(); dfi != defsList.end(); ++dfi)
		{
			DefinitionFile * defFile = *dfi;
			String fileName = defFile->filename.GetFilename();
			
			for (int frame = 0; frame < defFile->frameCount; ++frame)
			{
				auto* destRect = lastPackedPacker->SearchRectForPtr(&defFile->frameRects[frame]);
				if (!destRect)
				{
					AddError(Format("*** ERROR: Can't find rect for frame - %d. Definition - %s. ",
									frame,
									fileName.c_str()));
				}
				else
				{
					FilePath withoutExt(defFile->filename);
					withoutExt.TruncateExtension();

					PngImageExt image;
					image.Read(FramePathHelper::GetFramePathRelative(withoutExt, frame));
					DrawToFinalImage(finalImage, image, *destRect, defFile->frameRects[frame]);
				}
			}
			
			if (!WriteDefinition(excludeFolder, outputPath, "texture", defFile))
			{
				AddError(Format("* ERROR: Failed to write definition - %s.", fileName.c_str()));
			}
		}

        ExportImage(&finalImage, imageExportKeys, textureName);
	}else
	{
		Logger::FrameworkDebug("Can't pack to single output texture");
		PackToMultipleTextures(excludeFolder, outputPath, "texture", defsList, forGPU);
	}
}

void TexturePacker::PackToMultipleTextures(const FilePath & excludeFolder, const FilePath & outputPath, const char* basename, List<DefinitionFile*> & defList, eGPUFamily forGPU)
{
    Logger::FrameworkDebug("Packing to multiple output textures");

    ImageExportKeys imageExportKeys = GetExportKeys(forGPU);

    for (int i = 0; i < (int)sortVector.size(); ++i)
    {
        DefinitionFile * defFile = sortVector[i].defFile;
        int frame = sortVector[i].frameIndex;

        Logger::FrameworkDebug("[MultiPack] prepack: %s frame: %d w:%d h:%d", defFile->filename.GetAbsolutePathname().c_str(), frame, defFile->frameRects[frame].dx, defFile->frameRects[frame].dy);
    }

    Vector<ImagePacker*> & packers = usedPackers;

    Vector<SizeSortItem> sortVectorWork = sortVector;

    while (sortVectorWork.size() > 0)
    {
        // try to pack for each resolution
        float maxValue = 0.0f;

        Logger::FrameworkDebug("* Packing attempts started: ");

        ImagePacker * bestPackerForThisStep = 0;
        Vector<SizeSortItem> newWorkVector;

        bool needOnlySquareTexture = onlySquareTextures || NeedSquareTextureForCompression(imageExportKeys);
        for (uint32 yResolution = 8; yResolution <= maxTextureSize; yResolution *= 2)
            for (uint32 xResolution = 8; xResolution <= maxTextureSize; xResolution *= 2)
            {
                if (needOnlySquareTexture && (xResolution != yResolution))continue;

                Rect2i textureRect = Rect2i(0, 0, xResolution, yResolution);
                ImagePacker * packer = new ImagePacker(textureRect, useTwoSideMargin, texturesMargin);

                Vector<SizeSortItem> tempSortVector = sortVectorWork;
                float n = TryToPackFromSortVectorWeight(packer, tempSortVector);

                if (n > maxValue)
                {
                    maxValue = n;
                    SafeDelete(bestPackerForThisStep);
                    bestPackerForThisStep = packer;
                    newWorkVector = tempSortVector;
                }
                else
                {
                    SafeDelete(packer);
                }
            }

        sortVectorWork = newWorkVector;

        if (bestPackerForThisStep)
            packers.push_back(bestPackerForThisStep);
    }

    Logger::FrameworkDebug("* Writing %d final textures", (int)packers.size());

    Vector<PngImageExt*> finalImages;
    finalImages.reserve(packers.size());

    for (int imageIndex = 0; imageIndex < (int)packers.size(); ++imageIndex)
    {
        PngImageExt * image = new PngImageExt();
        ImagePacker * packer = packers[imageIndex];
        image->Create(packer->GetRect().dx, packer->GetRect().dy);
        finalImages.push_back(image);
    }

    for (List<DefinitionFile*>::iterator defi = defList.begin(); defi != defList.end(); ++defi)
    {
        DefinitionFile * defFile = *defi;

        for (int frame = 0; frame < defFile->frameCount; ++frame)
        {
            PackedInfo* packedInfo = nullptr;
            ImagePacker * foundPacker = 0;
            int packerIndex = 0;
            FilePath imagePath;

            for (packerIndex = 0; packerIndex < (int)packers.size(); ++packerIndex)
            {
                packedInfo = packers[packerIndex]->SearchRectForPtr(&defFile->frameRects[frame]);

                if (packedInfo)
                {
                    foundPacker = packers[packerIndex];
                    FilePath withoutExt(defFile->filename);
                    withoutExt.TruncateExtension();

                    imagePath = FramePathHelper::GetFramePathRelative(withoutExt, frame);
                    break;
                }
            }

            if (foundPacker)
            {
                Logger::FrameworkDebug("[MultiPack] pack to texture: %d", packerIndex);
                
				PngImageExt image;
				image.Read(imagePath);
				DrawToFinalImage(*finalImages[packerIndex], image, *packedInfo, defFile->frameRects[frame]);
			}
		}
	}
	
	for (int imageNum = 0; imageNum < (int)packers.size(); ++imageNum)
	{
		char temp[256];
		sprintf(temp, "%s%d", basename, imageNum);
		FilePath textureName = outputPath + temp;
        ExportImage(finalImages[imageNum], imageExportKeys, textureName);
    }

    for (List<DefinitionFile*>::iterator defi = defList.begin(); defi != defList.end(); ++defi)
    {
        DefinitionFile * defFile = *defi;
        String fileName = defFile->filename.GetFilename();
        FilePath textureName = outputPath + "texture";

        if (!WriteMultipleDefinition(excludeFolder, outputPath, basename, defFile))
        {
            AddError(Format("* ERROR: Failed to write definition - %s.", fileName.c_str()));
        }
    }
    for (auto &finalImage : finalImages)
    {
        SafeDelete(finalImage);
    }
}


Rect2i TexturePacker::GetOriginalSizeRect(const PackedInfo& _input)
{
    Rect2i r = _input.rect;
    r.x += _input.leftMargin;
    r.y += _input.topMargin;
    r.dx -= (_input.leftMargin + _input.rightMargin);
    r.dy -= (_input.topMargin + _input.bottomMargin);
    return r;
}

bool TexturePacker::WriteDefinition(const FilePath & /*excludeFolder*/, const FilePath & outputPath, const String & _textureName, DefinitionFile * defFile)
{
	String fileName = defFile->filename.GetFilename();
    Logger::FrameworkDebug("* Write definition: %s", fileName.c_str());
	
	FilePath defFilePath = outputPath + fileName;
	FILE * fp = fopen(defFilePath.GetAbsolutePathname().c_str(), "wt");
	if (!fp)return false;
	
	fprintf(fp, "%d\n", 1);
	
	String textureExtension = TextureDescriptor::GetDescriptorExtension();
	fprintf(fp, "%s%s\n", _textureName.c_str(), textureExtension.c_str());
	
	fprintf(fp, "%d %d\n", defFile->spriteWidth, defFile->spriteHeight);
	fprintf(fp, "%d\n", defFile->frameCount); 
	for (int frame = 0; frame < defFile->frameCount; ++frame)
	{
		PackedInfo* packedInfo = lastPackedPacker->SearchRectForPtr(&defFile->frameRects[frame]);
		if (!packedInfo)
		{
			AddError(Format("*** ERROR: Can't find rect for frame - %d. Definition - %s. ",
				frame,
				fileName.c_str()));
		}
		else
		{
			Rect2i origRect = defFile->frameRects[frame];
			Rect2i imageRect = GetOriginalSizeRect(*packedInfo);
            String frameName = defFile->frameNames.size() > 0 ? defFile->frameNames[frame] : String();
			WriteDefinitionString(fp, imageRect, origRect, 0, frameName);

			if(!CheckFrameSize(Size2i(defFile->spriteWidth, defFile->spriteHeight), imageRect.GetSize()))
			{
				Logger::Warning("In sprite %s.psd frame %d has size bigger than sprite size. Frame will be cropped.", defFile->filename.GetBasename().c_str(), frame);
			}
		}
	}
	
	for (int pathInfoLine = 0; pathInfoLine < (int)defFile->pathsInfo.size(); ++pathInfoLine)
	{
		String & line = defFile->pathsInfo[pathInfoLine];
		fprintf(fp, "%s", line.c_str());
	}
	
	fclose(fp);
	return true;
}

bool TexturePacker::WriteMultipleDefinition(const FilePath & /*excludeFolder*/, const FilePath & outputPath, const String & _textureName, DefinitionFile * defFile)
{
	String fileName = defFile->filename.GetFilename();
    Logger::FrameworkDebug("* Write definition: %s", fileName.c_str());
	
	FilePath defFilePath = outputPath + fileName;
	FILE * fp = fopen(defFilePath.GetAbsolutePathname().c_str(), "wt");
	if (!fp)return false;
	
	String textureExtension = TextureDescriptor::GetDescriptorExtension();
	
	Vector<int> packerIndexArray;
	packerIndexArray.resize(defFile->frameCount);
	
	Map<int, int> packerIndexToFileIndex;
	
	// find used texture indexes for this sprite
	for (int frame = 0; frame < defFile->frameCount; ++frame)
	{
		PackedInfo* packedInfo = 0;
		int packerIndex = 0;
		for (packerIndex = 0; packerIndex < (int)usedPackers.size(); ++packerIndex)
		{
			packedInfo = usedPackers[packerIndex]->SearchRectForPtr(&defFile->frameRects[frame]);
			if (packedInfo) break;
		}
		// save packer index for frame
		packerIndexArray[frame] = packerIndex;
		// add value to map to show that this packerIndex was used
		packerIndexToFileIndex[packerIndex] = -1;
	}
		
	// write real used packers count
	fprintf(fp, "%d\n", (int)packerIndexToFileIndex.size());
	
	int realIndex = 0;
	// write user texture indexes
	for (int i = 0; i < (int)usedPackers.size(); ++i)
	{
		Map<int, int>::iterator isUsed = packerIndexToFileIndex.find(i);
		if (isUsed != packerIndexToFileIndex.end())
		{
			// here we write filename for i-th texture and write to map real index in file for this texture
			fprintf(fp, "%s%d%s\n", _textureName.c_str(), i, textureExtension.c_str());
			packerIndexToFileIndex[i] = realIndex++;
		}
	}
	
	fprintf(fp, "%d %d\n", defFile->spriteWidth, defFile->spriteHeight);
	fprintf(fp, "%d\n", defFile->frameCount); 
	for (int frame = 0; frame < defFile->frameCount; ++frame)
	{
		PackedInfo* packedInfo = 0;
		for (int packerIndex = 0; packerIndex < (int)usedPackers.size(); ++packerIndex)
		{
			packedInfo = usedPackers[packerIndex]->SearchRectForPtr(&defFile->frameRects[frame]);
			if (packedInfo) break;
		}
		int packerIndex = packerIndexToFileIndex[packerIndexArray[frame]]; // here get real index in file for our used texture
		if (packedInfo)
		{
			Rect2i origRect = defFile->frameRects[frame];
            Rect2i writeRect = GetOriginalSizeRect(*packedInfo);
            String frameName = defFile->frameNames.size() > 0 ? defFile->frameNames[frame] : String();
			WriteDefinitionString(fp, writeRect, origRect, packerIndex, frameName);

            if(!CheckFrameSize(Size2i(defFile->spriteWidth, defFile->spriteHeight), writeRect.GetSize()))
            {
                Logger::Warning("In sprite %s.psd frame %d has size bigger than sprite size. Frame will be cropped.", defFile->filename.GetBasename().c_str(), frame);
            }
		}else
		{
			AddError(Format("*** FATAL ERROR: Can't find rect in all of packers for frame - %d. Definition file - %s.",
								frame,
								fileName.c_str()));

			fclose(fp);
			FileSystem::Instance()->DeleteFile(outputPath + fileName);
			return false;
		}
	}
	
	for (int pathInfoLine = 0; pathInfoLine < (int)defFile->pathsInfo.size(); ++pathInfoLine)
	{
		String & line = defFile->pathsInfo[pathInfoLine];
		fprintf(fp, "%s", line.c_str());
	}
	
	fclose(fp);
	return true;
}

void TexturePacker::UseOnlySquareTextures()
{
	onlySquareTextures = true;
}

void TexturePacker::SetMaxTextureSize(uint32 _maxTextureSize)
{
	maxTextureSize = _maxTextureSize;
}

uint8 GetPixelParameterAt(const Vector<String>& gpuParams, uint8 gpuParamPosition, PixelFormat& pixelFormat)
{
    if (gpuParamPosition < gpuParams.size())
    {
        PixelFormat format = PixelFormatDescriptor::GetPixelFormatByName(FastName(gpuParams[gpuParamPosition].c_str()));
        if (format != FORMAT_INVALID)
        {
            pixelFormat = format;
            return 1;
        }
    }

    return 0;
}

uint8 GetImageParametersAt(const Vector<String>& gpuParams, uint8 gpuParamPosition, ImageFormat& imageFormat, ImageQuality& imageQuality)
{
    uint8 paramsRead = 0;

    if (gpuParamPosition < gpuParams.size())
    {
        ImageFormat format = ImageSystem::Instance()->GetImageFormatByName(gpuParams[gpuParamPosition]);
        if (format != IMAGE_FORMAT_UNKNOWN)
        {
            imageFormat = format;
            ++paramsRead;
            ++gpuParamPosition;

            if (gpuParamPosition < gpuParams.size())
            {
                if (CompareCaseInsensitive(gpuParams[gpuParamPosition], "lossless") == 0)
                {
                    imageQuality = ImageQuality::LOSSLESS_IMAGE_QUALITY;
                    ++paramsRead;
                }
                else
                {
                    int res = -1;
                    bool parseOk = ParseFromString(gpuParams[gpuParamPosition], res);

                    if (parseOk && res >= 0 && res <= 100)
                    {
                        imageQuality = static_cast<ImageQuality>(res);
                        ++paramsRead;
                    }
                }
            }
        }
    }

    return paramsRead;
}

bool GetGpuParameters(eGPUFamily forGPU, PixelFormat& pixelFormat, ImageFormat& imageFormat,
                      ImageQuality& imageQuality, uint8& pixelParamsRead, uint8& imageParamsRead)
{
    const String gpuNameFlag = "--" + GPUFamilyDescriptor::GetGPUName(forGPU);
    if (!CommandLineParser::Instance()->IsFlagSet(gpuNameFlag))
    {
        pixelParamsRead = imageParamsRead = 0;
        return false;
    }

    // gpu flag has at least one additional parameter: pixel format or image format, or both of them
    // these params may follow in arbitrary order
    Vector<String> gpuParams = CommandLineParser::Instance()->GetParamsForFlag(gpuNameFlag);

    // suppose pixel parameter is at first position and read it
    uint8 gpuParamPosition = 0;
    pixelParamsRead = GetPixelParameterAt(gpuParams, gpuParamPosition, pixelFormat);

    if (pixelParamsRead == 1)
    {
        ++gpuParamPosition;
    }

    // read image parameters
    imageParamsRead = GetImageParametersAt(gpuParams, gpuParamPosition, imageFormat, imageQuality);

    // read pixel parameter in case if image parameters were on first position
    if (!pixelParamsRead && imageParamsRead)
    {
        gpuParamPosition += imageParamsRead;
        pixelParamsRead = GetPixelParameterAt(gpuParams, gpuParamPosition, pixelFormat);
    }

    return (pixelParamsRead || imageParamsRead);
}

TexturePacker::ImageExportKeys TexturePacker::GetExportKeys(eGPUFamily forGPU)
{
    ImageExportKeys keys;

    keys.forGPU = forGPU;

    if (GPUFamilyDescriptor::IsGPUForDevice(forGPU))
    {
        uint8 pixelParamsRead = 0;
        uint8 imageParamsRead = 0;
        GetGpuParameters(forGPU, keys.pixelFormat, keys.imageFormat, keys.imageQuality, pixelParamsRead, imageParamsRead);

        if (pixelParamsRead)
        {
            bool compressedImageFormatRead = false;
            if (imageParamsRead)
            {
                auto wrapper = ImageSystem::Instance()->GetImageFormatInterface(keys.imageFormat);
                if (keys.imageFormat == IMAGE_FORMAT_PVR || keys.imageFormat == IMAGE_FORMAT_DDS)
                {
                    if (GPUFamilyDescriptor::GetCompressedFileFormat(forGPU, keys.pixelFormat) == keys.imageFormat)
                    {
                        compressedImageFormatRead = true;
                    }
                    else
                    {
                        AddError(Format("Compression format '%s' can't be saved to %s image for GPU '%s'",
                            GlobalEnumMap<PixelFormat>::Instance()->ToString(keys.pixelFormat),
                            wrapper->Name(),
                            GPUFamilyDescriptor::GetGPUName(forGPU).c_str()));

                        keys.imageFormat = IMAGE_FORMAT_UNKNOWN;
                    }
                }
                else
                {
                    if (wrapper->IsFormatSupported(keys.pixelFormat))
                    {
                        if (ImageConvert::CanConvertFromTo(FORMAT_RGBA8888, keys.pixelFormat))
                        {
                            keys.toConvertOrigin = true;
                        }
                        else
                        {
                            AddError(Format("Can't convert to '%s'", GlobalEnumMap<PixelFormat>::Instance()->ToString(keys.pixelFormat)));
                        }
                    }
                    else
                    {
                        AddError(Format("Format '%s' is not supported for %s images.",
                            GlobalEnumMap<PixelFormat>::Instance()->ToString(keys.pixelFormat),
                            wrapper->Name()));
                    }
                }
            }

            if (!imageParamsRead || compressedImageFormatRead)
            {
                if (GPUFamilyDescriptor::IsFormatSupported(forGPU, keys.pixelFormat))
                {
                    keys.toComressForGPU = true;
                }
                else
                {
                    AddError(Format("Compression format '%s' is not supported for GPU '%s'",
                        GlobalEnumMap<PixelFormat>::Instance()->ToString(keys.pixelFormat),
                        GPUFamilyDescriptor::GetGPUName(forGPU).c_str()));
                }
            }
        }
        else if (imageParamsRead)
        {
            if (keys.imageFormat == IMAGE_FORMAT_PVR || keys.imageFormat == IMAGE_FORMAT_DDS)
            {
                AddError(Format("Compression format is not specified for '%s' token",
                    ImageSystem::Instance()->GetImageFormatInterface(keys.imageFormat)->Name()));
                keys.imageFormat = IMAGE_FORMAT_UNKNOWN;
            }
        }
        else
        {
            Logger::Warning("params for GPU %s were not set.\n", GPUFamilyDescriptor::GetGPUName(forGPU).c_str());
        }
    }

    if (keys.imageFormat == IMAGE_FORMAT_UNKNOWN || keys.toComressForGPU)
    {
        keys.imageFormat = IMAGE_FORMAT_PNG;
    }

    return keys;
}

void TexturePacker::ExportImage(PngImageExt *image, const ImageExportKeys& keys, FilePath exportedPathname)
{
    std::unique_ptr<TextureDescriptor> descriptor(new TextureDescriptor());

    descriptor->drawSettings.wrapModeS = descriptor->drawSettings.wrapModeT = GetDescriptorWrapMode();
    descriptor->SetGenerateMipmaps(CommandLineParser::Instance()->IsFlagSet(String("--generateMipMaps")));

    TexturePacker::FilterItem ftItem = GetDescriptorFilter(descriptor->GetGenerateMipMaps());
    descriptor->drawSettings.minFilter = ftItem.minFilter;
    descriptor->drawSettings.magFilter = ftItem.magFilter;
    descriptor->drawSettings.mipFilter = ftItem.mipFilter;

    if (keys.toComressForGPU)
    {
        descriptor->exportedAsGpuFamily = keys.forGPU;
        descriptor->format = keys.pixelFormat;
        descriptor->compression[keys.forGPU].format = keys.pixelFormat;
    }

    if (keys.toConvertOrigin)
    {
        image->ConvertToFormat(keys.pixelFormat);
    }

    const String extension = ImageSystem::Instance()->GetExtensionsFor(keys.imageFormat)[0];
    exportedPathname.ReplaceExtension(extension);

    descriptor->dataSettings.sourceFileFormat = keys.imageFormat;
    descriptor->dataSettings.sourceFileExtension = extension;
    descriptor->pathname = TextureDescriptor::GetDescriptorPathname(exportedPathname);
    descriptor->Export(descriptor->pathname);

    image->DitherAlpha();
    image->Write(exportedPathname, keys.imageQuality);

    if (keys.toComressForGPU)
    {
        TextureConverter::ConvertTexture(*descriptor, keys.forGPU, false, quality);
        FileSystem::Instance()->DeleteFile(exportedPathname);
    }
}

rhi::TextureAddrMode TexturePacker::GetDescriptorWrapMode()
{
	if (CommandLineParser::Instance()->IsFlagSet("--wrapClampToEdge"))
	{
		return rhi::TEXADDR_CLAMP;
	}
	else if (CommandLineParser::Instance()->IsFlagSet("--wrapRepeat"))
	{
        return rhi::TEXADDR_WRAP;
	}
	
	// Default Wrap mode
    return rhi::TEXADDR_CLAMP;
}

TexturePacker::FilterItem TexturePacker::GetDescriptorFilter(bool generateMipMaps)
{
	// Default filter
    TexturePacker::FilterItem filterItem(rhi::TEXFILTER_LINEAR, rhi::TEXFILTER_LINEAR, generateMipMaps ? rhi::TEXMIPFILTER_LINEAR : rhi::TEXMIPFILTER_NONE);
															
	
	if (CommandLineParser::Instance()->IsFlagSet("--magFilterNearest"))
	{
        filterItem.magFilter = rhi::TEXFILTER_NEAREST;
	}
	if (CommandLineParser::Instance()->IsFlagSet("--magFilterLinear"))
	{
        filterItem.magFilter = rhi::TEXFILTER_LINEAR;
	}
	if (CommandLineParser::Instance()->IsFlagSet("--minFilterNearest"))
	{
        filterItem.minFilter = rhi::TEXFILTER_NEAREST;
	}
	else if (CommandLineParser::Instance()->IsFlagSet("--minFilterLinear"))
	{
        filterItem.minFilter = rhi::TEXFILTER_LINEAR;
	}
	else if (CommandLineParser::Instance()->IsFlagSet("--minFilterNearestMipmapNearest"))
	{
        filterItem.minFilter = rhi::TEXFILTER_NEAREST;
        filterItem.mipFilter = rhi::TEXMIPFILTER_NEAREST;
	}
	else if (CommandLineParser::Instance()->IsFlagSet("--minFilterLinearMipmapNearest"))
	{
        filterItem.minFilter = rhi::TEXFILTER_LINEAR;
        filterItem.mipFilter = rhi::TEXMIPFILTER_NEAREST;
	}
	else if (CommandLineParser::Instance()->IsFlagSet("--minFilterNearestMipmapLinear"))
	{
        filterItem.minFilter = rhi::TEXFILTER_NEAREST;
        filterItem.mipFilter = rhi::TEXMIPFILTER_LINEAR;
	}
	else if (CommandLineParser::Instance()->IsFlagSet("--minFilterLinearMipmapLinear"))
	{
        filterItem.minFilter = rhi::TEXFILTER_LINEAR;
        filterItem.mipFilter = rhi::TEXMIPFILTER_LINEAR;
	}

	return filterItem;
}
    
bool TexturePacker::NeedSquareTextureForCompression(ImageExportKeys keys)
{
    return (keys.toComressForGPU &&
        PIXEL_FORMATS_WITH_COMPRESSION.find(keys.pixelFormat) != PIXEL_FORMATS_WITH_COMPRESSION.end());
}

bool TexturePacker::CheckFrameSize(const Size2i &spriteSize, const Size2i &frameSize)
{
    bool isSizeCorrect = ((frameSize.dx <= spriteSize.dx) && (frameSize.dy <= spriteSize.dy));
    
    return isSizeCorrect;
}

void TexturePacker::DrawToFinalImage( PngImageExt & finalImage, PngImageExt & drawedImage, const PackedInfo& drawRect, const Rect2i &alphaOffsetRect )
{
	finalImage.DrawImage(drawRect, alphaOffsetRect, &drawedImage);

	if (CommandLineParser::Instance()->IsFlagSet("--debug"))
	{
		finalImage.DrawRect(drawRect.rect, 0xFF0000FF);
	}
}

void TexturePacker::WriteDefinitionString(FILE *fp, const Rect2i & writeRect, const Rect2i &originRect, int textureIndex, const String& frameName)
{
	if(CommandLineParser::Instance()->IsFlagSet("--disableCropAlpha"))
	{
		fprintf(fp, "%d %d %d %d %d %d %d %s\n", writeRect.x, writeRect.y, writeRect.dx, writeRect.dy, 0, 0, textureIndex, frameName.c_str());
	}
	else
	{
		fprintf(fp, "%d %d %d %d %d %d %d %s\n", writeRect.x, writeRect.y, writeRect.dx, writeRect.dy, originRect.x, originRect.y, textureIndex, frameName.c_str());
	}
}

const Set<String>& TexturePacker::GetErrors() const
{
	return errors;
}
    
void TexturePacker::SetConvertQuality(TextureConverter::eConvertQuality _quality)
{
    quality = _quality;
}


void TexturePacker::AddError(const String& errorMsg)
{
	Logger::Error(errorMsg.c_str());
	errors.insert(errorMsg);
}

};
