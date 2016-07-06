#include "DXTConverter.h"

#include "FileSystem/FilePath.h"
#include "Render/TextureDescriptor.h"
#include "Render/Image/Image.h"
#include "Render/Image/ImageSystem.h"
#include "Render/Image/LibDdsHelper.h"
#include "Render/GPUFamilyDescriptor.h"

namespace DAVA
{
FilePath DXTConverter::ConvertToDxt(const TextureDescriptor& descriptor, eGPUFamily gpuFamily)
{
    FilePath fileToConvert = descriptor.GetSourceTexturePathname();

    Vector<Image*> inputImages;
    auto loadResult = ImageSystem::Load(fileToConvert, inputImages);

    if (loadResult != eErrorCode::SUCCESS || inputImages.empty())
    {
        Logger::Error("[DXTConverter::ConvertToDxt] can't open %s", fileToConvert.GetStringValue().c_str());
        return FilePath();
    }

    Vector<Image*> imagesToSave;

    FilePath outputName = GetDXTOutput(descriptor, gpuFamily);

    DVASSERT(descriptor.compression);
    const TextureDescriptor::Compression* compression = &descriptor.compression[gpuFamily];

    if (inputImages.size() == 1)
    {
        Image* image = inputImages[0];

        if ((compression->compressToWidth != 0) && (compression->compressToHeight != 0))
        {
            Logger::FrameworkDebug("[DXTConverter::ConvertToDxt] downscale to compression size");
            bool resized = image->ResizeImage(compression->compressToWidth, compression->compressToHeight);
            if (resized == false)
            {
                Logger::Error("[DXTConverter::ConvertToDxt] can't resize image %s", fileToConvert.GetStringValue().c_str());
                return FilePath();
            }
        }

        if (descriptor.dataSettings.GetGenerateMipMaps())
        {
            imagesToSave = image->CreateMipMapsImages();
        }
        else
        {
            imagesToSave.push_back(SafeRetain(image));
        }
    }
    else
    {
        DVASSERT(inputImages[0]->mipmapLevel == 0 && "first mipmap image has not a level 0");
        uint32 firstImageIndex = 0;

        if ((compression->compressToWidth != 0) && (compression->compressToHeight != 0))
        {
            Logger::FrameworkDebug("[DXTConverter::ConvertToDxt] downscale to compression size");

            uint32 i = 0;
            for (; i < inputImages.size(); ++i)
            {
                if (inputImages[i]->GetWidth() == compression->compressToWidth &&
                    inputImages[i]->GetHeight() == compression->compressToHeight)
                {
                    break;
                }
            }

            DVASSERT(i < inputImages.size() && "new compressed size is not found in mipmaps");
            firstImageIndex = i;
        }

        if (descriptor.dataSettings.GetGenerateMipMaps())
        {
            auto mipmapCounter = 0;
            for (auto i = firstImageIndex; i < inputImages.size(); ++i, ++mipmapCounter)
            {
                imagesToSave.push_back(SafeRetain(inputImages[i]));
                imagesToSave.back()->mipmapLevel = mipmapCounter;
            }
        }
        else
        {
            imagesToSave.push_back(SafeRetain(inputImages[firstImageIndex]));
            imagesToSave.back()->mipmapLevel = -1;
        }
    }

    eErrorCode retCode = ImageSystem::Save(outputName, imagesToSave, static_cast<PixelFormat>(compression->format));
    for_each(inputImages.begin(), inputImages.end(), SafeRelease<Image>);
    for_each(imagesToSave.begin(), imagesToSave.end(), SafeRelease<Image>);
    if (eErrorCode::SUCCESS == retCode)
    {
        LibDdsHelper::AddCRCIntoMetaData(outputName);
        return outputName;
    }

    Logger::Error("[DXTConverter::ConvertToDxt] can't save %s", outputName.GetAbsolutePathname().c_str());
    return FilePath();
}

FilePath DXTConverter::ConvertCubemapToDxt(const TextureDescriptor& descriptor, eGPUFamily gpuFamily)
{
    FilePath fileToConvert = descriptor.GetSourceTexturePathname();
    FilePath outputName = GetDXTOutput(descriptor, gpuFamily);

    Vector<FilePath> faceNames;
    descriptor.GetFacePathnames(faceNames);

    if (faceNames.size() != DAVA::Texture::CUBE_FACE_COUNT)
    {
        Logger::Error("[DXTConverter::ConvertCubemapToDxt] %s has %d cubemap faces", fileToConvert.GetAbsolutePathname().c_str(), faceNames.size());
        return FilePath();
    }

    bool hasErrors = false;
    Vector<Vector<Image*>> imageSets(DAVA::Texture::CUBE_FACE_COUNT);

    for (uint32 i = 0; i < DAVA::Texture::CUBE_FACE_COUNT; ++i)
    {
        if (faceNames[i].IsEmpty() ||
            ImageSystem::Load(faceNames[i], imageSets[i]) != DAVA::eErrorCode::SUCCESS ||
            imageSets[i].empty())
        {
            Logger::Error("[DXTConverter::ConvertCubemapToDxt] can't load %s", fileToConvert.GetAbsolutePathname().c_str());
            hasErrors = true;
            break;
        }

        if (i > 0)
        {
            if (imageSets[i].size() != imageSets[0].size())
            {
                Logger::Error("[DXTConverter::ConvertCubemapToDxt] mipmap count is not equal for cubemaps of %s", fileToConvert.GetAbsolutePathname().c_str());
                hasErrors = true;
                break;
            }

            if (imageSets[0][0]->width != imageSets[i][0]->width || imageSets[0][0]->height != imageSets[i][0]->height)
            {
                Logger::Error("[DXTConverter::ConvertCubemapToDxt] cubemap sizes are not equal for %s", fileToConvert.GetAbsolutePathname().c_str());
                hasErrors = true;
                break;
            }
        }
    }

    if (!hasErrors)
    {
        DVASSERT(descriptor.compression);
        const TextureDescriptor::Compression* compression = &descriptor.compression[gpuFamily];

        if (imageSets[0].size() == 1)
        {
            if ((compression->compressToWidth != 0) && (compression->compressToHeight != 0))
            {
                Logger::FrameworkDebug("[DXTConverter::ConvertCubemapToDxt] convert to compression size");

                for (auto& imageSet : imageSets)
                {
                    imageSet[0]->ResizeImage(compression->compressToWidth, compression->compressToHeight);
                }
            }

            //generate mipmaps for every face
            if (descriptor.dataSettings.GetGenerateMipMaps())
            {
                for (auto& imageSet : imageSets)
                {
                    Image* image = imageSet[0];
                    imageSet = image->CreateMipMapsImages();
                    SafeRelease(image);
                }
            }
        }
        else
        {
            auto firstImageIndex = 0;
            if ((compression->compressToWidth != 0) && (compression->compressToHeight != 0))
            {
                Logger::FrameworkDebug("[DXTConverter::ConvertCubemapToDxt] downscale to compression size");

                uint32 i = 0;
                for (; i < imageSets[0].size(); ++i)
                {
                    if (imageSets[0][i]->GetWidth() == compression->compressToWidth &&
                        imageSets[0][i]->GetHeight() == compression->compressToHeight)
                    {
                        break;
                    }
                }

                DVASSERT(i < imageSets[0].size() && "new compressed size is not found in mipmaps");
                firstImageIndex = i;
            }

            if (descriptor.dataSettings.GetGenerateMipMaps())
            {
                for (auto& imageSet : imageSets)
                {
                    std::rotate(imageSet.begin(), imageSet.begin() + firstImageIndex, imageSet.end());
                    for_each(imageSet.rbegin(), imageSet.rbegin() + firstImageIndex, SafeRelease<Image>);
                    imageSet.resize(imageSet.size() - firstImageIndex);
                    auto mipmapCounter = 0;
                    for (auto& image : imageSet)
                    {
                        image->mipmapLevel = mipmapCounter++;
                    }
                }
            }
            else
            {
                for (auto& imageSet : imageSets)
                {
                    for_each(imageSet.begin() + 1, imageSet.end(), SafeRelease<Image>);
                    imageSet.resize(1);
                    imageSet[0]->mipmapLevel = -1;
                }
            }
        }

        auto saveResult = ImageSystem::SaveAsCubeMap(outputName, imageSets, static_cast<PixelFormat>(compression->format));
        if (saveResult == eErrorCode::SUCCESS)
        {
            LibDdsHelper::AddCRCIntoMetaData(outputName);
        }
        else
        {
            hasErrors = true;
        }
    }

    for (auto& imagesSet : imageSets)
    {
        for_each(imagesSet.begin(), imagesSet.end(), SafeRelease<Image>);
    }

    return (hasErrors ? FilePath() : outputName);
}

FilePath DXTConverter::GetDXTOutput(const TextureDescriptor& descriptor, eGPUFamily gpuFamily)
{
    return descriptor.CreateMultiMipPathnameForGPU(gpuFamily);
}
};
