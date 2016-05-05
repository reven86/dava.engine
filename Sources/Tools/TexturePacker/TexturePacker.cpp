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
#include "CommandLine/CommandLineParser.h"
#include "TexturePacker/Spritesheet.h"
#include "TexturePacker/PngImage.h"
#include "TexturePacker/DefinitionFile.h"
#include "Render/TextureDescriptor.h"
#include "Render/GPUFamilyDescriptor.h"
#include "Render/PixelFormatDescriptor.h"
#include "Render/Image/ImageSystem.h"
#include "Render/Image/ImageFormatInterface.h"
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
{
    quality = TextureConverter::ECQ_VERY_HIGH;
    if (CommandLineParser::Instance()->IsFlagSet("--quality"))
    {
        String qualityName = CommandLineParser::Instance()->GetParamForFlag("--quality");
        int32 q = atoi(qualityName.c_str());
        if ((q >= TextureConverter::ECQ_FASTEST) && (q <= TextureConverter::ECQ_VERY_HIGH))
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

DAVA::int32 TexturePacker::TryToPack(SpritesheetLayout* sheet, Vector<SpriteItem>& tempSortVector, bool fullPackOnly)
{
    uint32 weight = 0;

    for (uint32 i = 0; i < tempSortVector.size();)
    {
        const DefinitionFile::Pointer& defFile = tempSortVector[i].defFile;
        uint32 frame = tempSortVector[i].frameIndex;
        if (sheet->AddSprite(defFile->GetFrameSize(frame), &defFile->frameRects[frame]))
        {
            weight += tempSortVector[i].spriteWeight;
            tempSortVector.erase(tempSortVector.begin() + i);
        }
        else if (fullPackOnly)
        {
            return weight;
        }
        else
        {
            ++i;
        }
    }
    return weight;
}

void TexturePacker::PackToTexturesSeparate(const FilePath& outputPath, const DefinitionFile::Collection& defsList, const Vector<eGPUFamily>& forGPUs)
{
    Logger::FrameworkDebug("Packing to separate textures");

    for (const DefinitionFile::Pointer& defFile : defsList)
    {
        PackToMultipleTextures(outputPath, defFile->filename.GetBasename().c_str(), { defFile }, forGPUs);
    }
}

void TexturePacker::PackToTextures(const FilePath& outputPath, const DefinitionFile::Collection& defsList, const Vector<eGPUFamily>& forGPUs)
{
    PackToMultipleTextures(outputPath, "texture", defsList, forGPUs);
}

void TexturePacker::PackToMultipleTextures(const FilePath& outputPath, const char* basename, const DefinitionFile::Collection& defList, const Vector<eGPUFamily>& forGPUs)
{
    DVASSERT_MSG(packAlgorithms.empty() == false, "Packing algorithm was not specified");

    Vector<ImageExportKeys> imageExportKeys = GetExportKeys(forGPUs);
    Vector<SpriteItem> spritesToPack = PrepareSpritesVector(defList);
    Vector<std::unique_ptr<SpritesheetLayout>> resultSheets = PackSprites(spritesToPack, imageExportKeys);
    SaveResultSheets(outputPath, basename, defList, resultSheets, imageExportKeys);
}

Vector<SpriteItem> TexturePacker::PrepareSpritesVector(const DefinitionFile::Collection& defList)
{
    Vector<SpriteItem> spritesToPack;
    for (const DefinitionFile::Pointer& defFile : defList)
    {
        for (uint32 frame = 0; frame < defFile->frameCount; ++frame)
        {
            SpriteItem spriteItem;
            spriteItem.spriteWeight = defFile->GetFrameWidth(frame) * defFile->GetFrameHeight(frame);
            spriteItem.defFile = defFile;
            spriteItem.frameIndex = frame;
            spritesToPack.push_back(spriteItem);
        }
    }

    std::stable_sort(spritesToPack.begin(), spritesToPack.end(),
                     [](const SpriteItem& a, const SpriteItem& b) { return a.spriteWeight > b.spriteWeight; });

    return spritesToPack;
}

Vector<std::unique_ptr<SpritesheetLayout>> TexturePacker::PackSprites(Vector<SpriteItem>& spritesToPack, const Vector<ImageExportKeys>& imageExportKeys)
{
    Vector<std::unique_ptr<SpritesheetLayout>> resultSheets;

    while (false == spritesToPack.empty())
    {
        Logger::FrameworkDebug("* Packing attempts started: ");

        std::unique_ptr<SpritesheetLayout> bestSheet;
        int32 bestSpritesWeight = 0;
        int32 bestSheetWeight = 0;
        bool wasFullyPacked = false;
        Vector<SpriteItem> bestSpritesRemaining;

        bool needOnlySquareTexture = onlySquareTextures || NeedSquareTextureForCompression(imageExportKeys);
        for (uint32 yResolution = 8; yResolution <= maxTextureSize; yResolution *= 2)
        {
            for (uint32 xResolution = 8; xResolution <= maxTextureSize; xResolution *= 2)
            {
                if (needOnlySquareTexture && (xResolution != yResolution))
                    continue;

                int32 sheetWeight = xResolution * yResolution;

                if (wasFullyPacked && sheetWeight >= bestSheetWeight)
                    continue;

                for (PackingAlgorithm alg : packAlgorithms)
                {
                    std::unique_ptr<SpritesheetLayout> sheet = SpritesheetLayout::Create(xResolution, yResolution, useTwoSideMargin, texturesMargin, alg);

                    Vector<SpriteItem> tempSpritesRemaining = spritesToPack;
                    int32 spritesWeight = TryToPack(sheet.get(), tempSpritesRemaining, wasFullyPacked);

                    bool nowFullyPacked = tempSpritesRemaining.empty();

                    if (wasFullyPacked && !nowFullyPacked)
                        continue;

                    if (nowFullyPacked || spritesWeight > bestSpritesWeight || (spritesWeight == bestSpritesWeight && sheetWeight < bestSheetWeight))
                    {
                        bestSpritesWeight = spritesWeight;
                        bestSheetWeight = sheetWeight;
                        bestSheet = std::move(sheet);
                        bestSpritesRemaining.swap(tempSpritesRemaining);

                        if (nowFullyPacked)
                        {
                            wasFullyPacked = true;
                            break;
                        }
                    }
                }
            }
        }

        DVASSERT_MSG(bestSpritesWeight > 0, "Can't pack any sprite");

        spritesToPack.swap(bestSpritesRemaining);
        resultSheets.emplace_back(std::move(bestSheet));
    }

    return resultSheets;
}

void TexturePacker::SaveResultSheets(const FilePath& outputPath, const char* basename, const DefinitionFile::Collection& defList, const Vector<std::unique_ptr<SpritesheetLayout>>& resultSheets, const Vector<ImageExportKeys>& imageExportKeys)
{
    Logger::FrameworkDebug("* Writing %d final texture(s)", resultSheets.size());

    Vector<PngImageExt> finalImages(resultSheets.size());
    for (uint32 i = 0; i < finalImages.size(); ++i)
    {
        finalImages[i].Create(resultSheets[i]->GetRect().dx, resultSheets[i]->GetRect().dy);
    }

    for (const DefinitionFile::Pointer& defFile : defList)
    {
        for (uint32 frame = 0; frame < defFile->frameCount; ++frame)
        {
            const SpriteBoundsRect* packedInfo = nullptr;
            uint32 sheetIndex = 0;
            FilePath imagePath;

            for (sheetIndex = 0; sheetIndex < resultSheets.size(); ++sheetIndex)
            {
                packedInfo = resultSheets[sheetIndex]->GetSpriteBoundsRect(&defFile->frameRects[frame]);

                if (packedInfo)
                {
                    FilePath withoutExt(defFile->filename);
                    withoutExt.TruncateExtension();
                    imagePath = FramePathHelper::GetFramePathRelative(withoutExt, frame);
                    break;
                }
            }

            if (nullptr != packedInfo)
            {
                Logger::FrameworkDebug("[MultiPack] pack to texture: %d", sheetIndex);

                PngImageExt image;
                image.Read(imagePath);
                DrawToFinalImage(finalImages[sheetIndex], image, *packedInfo, defFile->frameRects[frame]);
            }
        }
    }

    for (uint32 imageNum = 0; imageNum < finalImages.size(); ++imageNum)
    {
        char temp[256];
        sprintf(temp, "%s%d", basename, imageNum);
        FilePath textureName = outputPath + temp;
        ExportImage(finalImages[imageNum], imageExportKeys, textureName);
    }

    for (const DefinitionFile::Pointer& defFile : defList)
    {
        String fileName = defFile->filename.GetFilename();
        FilePath textureName = outputPath + "texture";

        if (!WriteMultipleDefinition(resultSheets, outputPath, basename, *(defFile.Get())))
        {
            AddError(Format("* ERROR: Failed to write definition - %s.", fileName.c_str()));
        }
    }
}

bool TexturePacker::WriteMultipleDefinition(const Vector<std::unique_ptr<SpritesheetLayout>>& usedSheets, const FilePath& outputPath,
                                            const String& _textureName, const DefinitionFile& defFile)
{
    String fileName = defFile.filename.GetFilename();
    Logger::FrameworkDebug("* Write definition: %s", fileName.c_str());

    FilePath defFilePath = outputPath + fileName;
    FILE* fp = fopen(defFilePath.GetAbsolutePathname().c_str(), "wt");
    if (fp == nullptr)
    {
        AddError(Format("Unable to open file for writing: %s", defFilePath.GetAbsolutePathname().c_str()));
        return false;
    }

    String textureExtension = TextureDescriptor::GetDescriptorExtension();

    Vector<int> frameToSheetIndex;
    frameToSheetIndex.resize(defFile.frameCount);

    Map<int, int> sheetIndexToFileIndex;

    // find used texture indexes for this sprite
    for (uint32 frame = 0; frame < defFile.frameCount; ++frame)
    {
        const SpriteBoundsRect* packedInfo = 0;
        uint32 sheetIndex = 0;
        for (; sheetIndex < usedSheets.size(); ++sheetIndex)
        {
            packedInfo = usedSheets[sheetIndex]->GetSpriteBoundsRect(&defFile.frameRects[frame]);
            if (packedInfo)
                break;
        }
        // save packer index for frame
        frameToSheetIndex[frame] = sheetIndex;
        // add value to map to show that this packerIndex was used
        sheetIndexToFileIndex[sheetIndex] = -1;
    }

    // write real used packers count
    fprintf(fp, "%d\n", (int)sheetIndexToFileIndex.size());

    int realIndex = 0;
    // write user texture indexes
    for (uint32 i = 0; i < usedSheets.size(); ++i)
    {
        auto itFound = sheetIndexToFileIndex.find(i);
        if (itFound != sheetIndexToFileIndex.end())
        {
            // here we write filename for i-th texture and write to map real index in file for this texture
            fprintf(fp, "%s%d%s\n", _textureName.c_str(), i, textureExtension.c_str());
            itFound->second = realIndex++;
        }
    }

    fprintf(fp, "%d %d\n", defFile.spriteWidth, defFile.spriteHeight);
    fprintf(fp, "%d\n", defFile.frameCount);
    for (uint32 frame = 0; frame < defFile.frameCount; ++frame)
    {
        const SpriteBoundsRect* spriteBounds = nullptr;
        for (const std::unique_ptr<SpritesheetLayout>& atlas : usedSheets)
        {
            spriteBounds = atlas->GetSpriteBoundsRect(&defFile.frameRects[frame]);
            if (spriteBounds)
                break;
        }
        int packerIndex = sheetIndexToFileIndex[frameToSheetIndex[frame]]; // here get real index in file for our used texture
        if (spriteBounds)
        {
            const Rect2i& origRect = defFile.frameRects[frame];
            const Rect2i& writeRect = spriteBounds->spriteRect;
            String frameName = defFile.frameNames.size() > 0 ? defFile.frameNames[frame] : String();
            WriteDefinitionString(fp, writeRect, origRect, packerIndex, frameName);

            if (!CheckFrameSize(Size2i(defFile.spriteWidth, defFile.spriteHeight), writeRect.GetSize()))
            {
                Logger::Warning("In sprite %s.psd frame %d has size bigger than sprite size. Frame will be cropped.", defFile.filename.GetBasename().c_str(), frame);
            }
        }
        else
        {
            AddError(Format("*** FATAL ERROR: Can't find rect in all of packers for frame - %d. Definition file - %s.",
                            frame,
                            fileName.c_str()));

            fclose(fp);
            FileSystem::Instance()->DeleteFile(outputPath + fileName);
            return false;
        }
    }

    fclose(fp);
    return true;
}

void TexturePacker::SetUseOnlySquareTextures()
{
    onlySquareTextures = true;
}

void TexturePacker::SetMaxTextureSize(uint32 _maxTextureSize)
{
    maxTextureSize = _maxTextureSize;
}

void TexturePacker::SetAlgorithms(const Vector<PackingAlgorithm>& algorithms)
{
    packAlgorithms = algorithms;
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
                      ImageQuality& imageQuality)
{
    const String gpuNameFlag = "--" + GPUFamilyDescriptor::GetGPUName(forGPU);
    // gpu flag has at least one additional parameter: pixel format or image format, or both of them
    // these params may follow in arbitrary order
    Vector<String> gpuParams = CommandLineParser::Instance()->GetParamsForFlag(gpuNameFlag);

    // suppose pixel parameter is at first position and read it
    uint8 gpuParamPosition = 0;
    uint8 pixelParamsRead = GetPixelParameterAt(gpuParams, gpuParamPosition, pixelFormat);

    if (pixelParamsRead == 1)
    {
        ++gpuParamPosition;
    }

    // read image parameters
    uint8 imageParamsRead = GetImageParametersAt(gpuParams, gpuParamPosition, imageFormat, imageQuality);

    // read pixel parameter in case if image parameters were on first position
    if (!pixelParamsRead && imageParamsRead)
    {
        gpuParamPosition += imageParamsRead;
        pixelParamsRead = GetPixelParameterAt(gpuParams, gpuParamPosition, pixelFormat);
    }

    return (pixelParamsRead || imageParamsRead);
}

Vector<TexturePacker::ImageExportKeys> TexturePacker::GetExportKeys(const Vector<eGPUFamily>& forGPUs)
{
    Vector<ImageExportKeys> compressionTargets;

    size_type count = forGPUs.size();
    compressionTargets.resize(count);
    for (size_type i = 0; i < count; ++i)
    {
        compressionTargets[i].forGPU = forGPUs[i];
    }

    for (auto& keys : compressionTargets)
    {
        if (keys.forGPU == eGPUFamily::GPU_ORIGIN)
        {
            keys.pixelFormat = PixelFormat::FORMAT_RGBA8888;
            keys.imageFormat = ImageFormat::IMAGE_FORMAT_PNG;
        }
        else if (GPUFamilyDescriptor::IsGPUForDevice(keys.forGPU))
        {
            { // read GPU parameters
                const String gpuNameFlag = "--" + GPUFamilyDescriptor::GetGPUName(keys.forGPU);
                bool gpuParametersRead = false;
                if (CommandLineParser::Instance()->IsFlagSet(gpuNameFlag) || (count > 1))
                {
                    gpuParametersRead = GetGpuParameters(keys.forGPU, keys.pixelFormat, keys.imageFormat, keys.imageQuality);
                }

                if (!gpuParametersRead)
                {
                    AddError(Format("Cannot read compression options for GPU '%s'", GlobalEnumMap<eGPUFamily>::Instance()->ToString(keys.forGPU)));
                    continue;
                }
            }

            { // validate pixel and image formats
                if (keys.pixelFormat == PixelFormat::FORMAT_INVALID)
                { //we should always set target pixel format in flags.txt
                    AddError(Format("Compression format was not selected for GPU '%s'", GlobalEnumMap<eGPUFamily>::Instance()->ToString(keys.forGPU)));
                    keys.imageFormat = ImageFormat::IMAGE_FORMAT_UNKNOWN;
                    continue;
                }

                if (keys.imageFormat == ImageFormat::IMAGE_FORMAT_UNKNOWN)
                {
                    keys.imageFormat = GPUFamilyDescriptor::GetCompressedFileFormat(keys.forGPU, keys.pixelFormat);
                }
                else if (keys.imageFormat == ImageFormat::IMAGE_FORMAT_PVR || keys.imageFormat == ImageFormat::IMAGE_FORMAT_DDS)
                {
                    if (GPUFamilyDescriptor::GetCompressedFileFormat(keys.forGPU, keys.pixelFormat) != keys.imageFormat)
                    {
                        keys.imageFormat = ImageFormat::IMAGE_FORMAT_UNKNOWN;
                    }
                }

                if (keys.imageFormat == ImageFormat::IMAGE_FORMAT_UNKNOWN)
                {
                    keys.pixelFormat = PixelFormat::FORMAT_INVALID;
                    AddError(Format("Incorrect settings were selected for GPU '%s'", GlobalEnumMap<eGPUFamily>::Instance()->ToString(keys.forGPU)));
                    continue;
                }
            }

            { // define internal format convertation options. should be refactored after PVR/DXT formats will be moved into ImageConvert
                if (keys.imageFormat == ImageFormat::IMAGE_FORMAT_WEBP
                    || keys.imageFormat == ImageFormat::IMAGE_FORMAT_JPEG
                    || keys.imageFormat == ImageFormat::IMAGE_FORMAT_TGA
                    || keys.imageFormat == ImageFormat::IMAGE_FORMAT_PNG
                    )
                {
                    keys.toConvertOrigin = (keys.pixelFormat == PixelFormat::FORMAT_RGBA8888) ? true : ImageConvert::CanConvertFromTo(PixelFormat::FORMAT_RGBA8888, keys.pixelFormat);
                    if (!keys.toConvertOrigin)
                    {
                        AddError(Format("Can't convert to '%s'", GlobalEnumMap<PixelFormat>::Instance()->ToString(keys.pixelFormat)));

                        keys.pixelFormat = PixelFormat::FORMAT_INVALID;
                        keys.imageFormat = ImageFormat::IMAGE_FORMAT_UNKNOWN;
                        continue;
                    }
                }
                else if (keys.imageFormat == ImageFormat::IMAGE_FORMAT_PVR || keys.imageFormat == ImageFormat::IMAGE_FORMAT_DDS)
                {
                    keys.toComressForGPU = GPUFamilyDescriptor::IsFormatSupported(keys.forGPU, keys.pixelFormat);
                    if (!keys.toComressForGPU)
                    {
                        AddError(Format("DVASSERT: Compression format '%s' is not supported for GPU '%s'",
                                        GlobalEnumMap<PixelFormat>::Instance()->ToString(keys.pixelFormat),
                                        GlobalEnumMap<eGPUFamily>::Instance()->ToString(keys.forGPU)));

                        keys.pixelFormat = PixelFormat::FORMAT_INVALID;
                        keys.imageFormat = ImageFormat::IMAGE_FORMAT_UNKNOWN;
                        continue;
                    }
                }
            }
        }
    }

    return compressionTargets;
}

void TexturePacker::ExportImage(const PngImageExt& image, const Vector<ImageExportKeys>& keys, const FilePath& pathnameWithoutExtension)
{
    std::unique_ptr<TextureDescriptor> descriptor(new TextureDescriptor());

    { // prepare general info for sprite drawing
        descriptor->drawSettings.wrapModeS = descriptor->drawSettings.wrapModeT = GetDescriptorWrapMode();
        descriptor->SetGenerateMipmaps(CommandLineParser::Instance()->IsFlagSet(String("--generateMipMaps")));

        TexturePacker::FilterItem ftItem = GetDescriptorFilter(descriptor->GetGenerateMipMaps());
        descriptor->drawSettings.minFilter = ftItem.minFilter;
        descriptor->drawSettings.magFilter = ftItem.magFilter;
        descriptor->drawSettings.mipFilter = ftItem.mipFilter;
        descriptor->pathname = pathnameWithoutExtension + TextureDescriptor::GetDescriptorExtension();
    }

    for (const auto& key : keys)
    {
        if (key.imageFormat == ImageFormat::IMAGE_FORMAT_UNKNOWN || key.pixelFormat == PixelFormat::FORMAT_INVALID)
        {
            Logger::Error("Cannot export texture %s for GPU %s", pathnameWithoutExtension.GetStringValue().c_str(), GlobalEnumMap<eGPUFamily>::Instance()->ToString(key.forGPU));
            continue;
        }

        descriptor->compression[key.forGPU].format = key.pixelFormat;
        descriptor->compression[key.forGPU].imageFormat = key.imageFormat;

        PngImageExt imageForGPU(image);
        if (key.imageFormat == ImageFormat::IMAGE_FORMAT_DDS || key.imageFormat == ImageFormat::IMAGE_FORMAT_PVR)
        {
            descriptor->dataSettings.sourceFileFormat = IMAGE_FORMAT_PNG;
        }
        else
        {
            descriptor->dataSettings.sourceFileFormat = key.imageFormat;
            if (key.toConvertOrigin)
            {
                imageForGPU.ConvertToFormat(key.pixelFormat);
            }
        }

        String srcExtension = ImageSystem::Instance()->GetExtensionsFor(descriptor->dataSettings.sourceFileFormat)[0];
        descriptor->dataSettings.sourceFileExtension = srcExtension;

        imageForGPU.DitherAlpha();
        imageForGPU.Write(descriptor->GetSourceTexturePathname(), key.imageQuality);

        if (key.toComressForGPU)
        {
            TextureConverter::ConvertTexture(*descriptor, key.forGPU, false, quality);
        }
    }

    if (keys.size() == 1)
    {
        descriptor->Export(descriptor->pathname, keys[0].forGPU);
        if (keys[0].toComressForGPU)
        {
            FileSystem::Instance()->DeleteFile(descriptor->GetSourceTexturePathname());
        }
    }
    else
    {
        descriptor->Save(descriptor->pathname);
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
    else if (CommandLineParser::Instance()->IsFlagSet("--wrapMirror"))
    {
        return rhi::TEXADDR_MIRROR;
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

bool TexturePacker::NeedSquareTextureForCompression(const Vector<ImageExportKeys>& keys)
{
    for (const auto& key : keys)
    {
        bool needSquare = (key.toComressForGPU && PIXEL_FORMATS_WITH_COMPRESSION.find(key.pixelFormat) != PIXEL_FORMATS_WITH_COMPRESSION.end());
        if (needSquare)
        {
            return true;
        }
    }

    return false;
}

bool TexturePacker::CheckFrameSize(const Size2i& spriteSize, const Size2i& frameSize)
{
    bool isSizeCorrect = ((frameSize.dx <= spriteSize.dx) && (frameSize.dy <= spriteSize.dy));

    return isSizeCorrect;
}

void TexturePacker::DrawToFinalImage(PngImageExt& finalImage, PngImageExt& drawedImage, const SpriteBoundsRect& packedCell, const Rect2i& alphaOffsetRect)
{
    finalImage.DrawImage(packedCell, alphaOffsetRect, &drawedImage);

    if (CommandLineParser::Instance()->IsFlagSet("--debug"))
    {
        finalImage.DrawRect(packedCell.marginsRect, 0xFF0000FF);
    }
}

void TexturePacker::WriteDefinitionString(FILE* fp, const Rect2i& writeRect, const Rect2i& originRect, int textureIndex, const String& frameName)
{
    if (CommandLineParser::Instance()->IsFlagSet("--disableCropAlpha"))
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
