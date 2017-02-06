#include "Tools/TexturePacker/DefinitionFile.h"
#include "Tools/TexturePacker/PngImage.h"
#include "Tools/TexturePacker/FramePathHelper.h"

#include <FileSystem/FileSystem.h>
#include <Logger/Logger.h>
#include <Render/Image/LibPSDHelper.h>
#include <Utils/StringFormat.h>

#include <libpng/png.h>
#include <libpsd/libpsd.h>
#include <sys/types.h>
#include <sys/stat.h>

namespace DAVA
{
namespace DefinitionFileLocal
{
bool WritePNGImage(int width, int height, char* imageData, const char* outName, int channels, int bit_depth);
}

void DefinitionFile::LoadPNG(const FilePath& _filename, const FilePath& pathToProcess)
{
    DVASSERT(pathToProcess.IsDirectoryPathname());

    String nameWithoutExt = _filename.GetBasename();
    FilePath corespondingPngImage = FilePath::CreateWithNewExtension(_filename, ".png");

    filename = pathToProcess + (nameWithoutExt + String(".txt"));
    frameCount = 1;

    PngImageExt image;
    image.Read(corespondingPngImage);
    spriteWidth = image.GetWidth();
    spriteHeight = image.GetHeight();

    frameNames.resize(frameCount);
    frameRects.resize(frameCount);
    frameRects[0].x = 0;
    frameRects[0].y = 0;
    frameRects[0].dx = spriteWidth;
    frameRects[0].dy = spriteHeight;

    FilePath fileWrite = FramePathHelper::GetFramePathAbsolute(pathToProcess, nameWithoutExt, 0);
    FileSystem::Instance()->CopyFile(_filename, fileWrite);
}

bool DefinitionFile::LoadPNGDef(const FilePath& _filename, const FilePath& pathToProcess)
{
    DVASSERT(pathToProcess.IsDirectoryPathname());

    Logger::FrameworkDebug("* Load PNG Definition: %s", _filename.GetAbsolutePathname().c_str());

    FILE* fp = fopen(_filename.GetAbsolutePathname().c_str(), "rt");
    fscanf(fp, "%d", &frameCount);

    String nameWithoutExt = _filename.GetBasename();
    FilePath corespondingPngImage = _filename.GetDirectory() + (nameWithoutExt + String(".png"));

    filename = pathToProcess + (nameWithoutExt + String(".txt"));

    PngImageExt image;
    image.Read(corespondingPngImage);
    spriteWidth = image.GetWidth() / frameCount;
    spriteHeight = image.GetHeight();

    Logger::FrameworkDebug("* frameCount: %d spriteWidth: %d spriteHeight: %d", frameCount, spriteWidth, spriteHeight);

    frameRects.resize(frameCount);
    frameNames.resize(frameCount);
    for (uint32 k = 0; k < frameCount; ++k)
    {
        PngImageExt frameX;
        frameX.Create(spriteWidth, spriteHeight);
        frameX.DrawImage(0, 0, &image, Rect2i(k * spriteWidth, 0, spriteWidth, spriteHeight));

        Rect2i reducedRect;
        frameX.FindNonOpaqueRect(reducedRect);
        Logger::FrameworkDebug("%s - reduced_rect(%d %d %d %d)", nameWithoutExt.c_str(), reducedRect.x, reducedRect.y, reducedRect.dx, reducedRect.dy);

        PngImageExt frameX2;
        frameX2.Create(reducedRect.dx, reducedRect.dy);
        frameX2.DrawImage(0, 0, &frameX, reducedRect);

        FilePath fileWrite = FramePathHelper::GetFramePathAbsolute(pathToProcess, nameWithoutExt, k);
        frameX2.Write(fileWrite);

        frameRects[k].x = reducedRect.x;
        frameRects[k].y = reducedRect.y;
        frameRects[k].dx = reducedRect.dx;
        frameRects[k].dy = reducedRect.dy;
    }

    fclose(fp);
    return true;
}

bool DefinitionFile::Load(const FilePath& _filename)
{
    filename = _filename;
    FILE* fp = fopen(filename.GetAbsolutePathname().c_str(), "rt");
    if (!fp)
    {
        Logger::Error("*** ERROR: Can't open definition file: %s", filename.GetAbsolutePathname().c_str());
        return false;
    }
    fscanf(fp, "%d %d", &spriteWidth, &spriteHeight);
    fscanf(fp, "%d", &frameCount);

    frameRects.resize(frameCount);
    frameNames.resize(frameCount);
    for (uint32 i = 0; i < frameCount; ++i)
    {
        char frameName[128];
        fscanf(fp, "%d %d %d %d %s\n", &frameRects[i].x, &frameRects[i].y, &frameRects[i].dx, &frameRects[i].dy, frameName);
        Logger::FrameworkDebug("[DefinitionFile] frame: %d w: %d h: %d", i, frameRects[i].dx, frameRects[i].dy);
        frameNames[i] = String(frameName);
    }

    fclose(fp);
    Logger::FrameworkDebug("Loaded definition: %s frames: %d", filename.GetAbsolutePathname().c_str(), frameCount);

    return true;
}

Size2i DefinitionFile::GetFrameSize(uint32 frame) const
{
    return Size2i(frameRects[frame].dx, frameRects[frame].dy);
}

int DefinitionFile::GetFrameWidth(uint32 frame) const
{
    return frameRects[frame].dx;
}

int DefinitionFile::GetFrameHeight(uint32 frame) const
{
    return frameRects[frame].dy;
}

bool DefinitionFile::LoadPSD(const FilePath& fullname, const FilePath& processDirectoryPath, uint32 maxTextureSize,
                             bool retainEmptyPixesl, bool useLayerNames, bool verboseOutput)
{
    if (FileSystem::Instance()->CreateDirectory(processDirectoryPath) == FileSystem::DIRECTORY_CANT_CREATE)
    {
        Logger::Error("============================ ERROR ============================");
        Logger::Error("| Can't create output directory: ");
        Logger::Error("| %s", processDirectoryPath.GetAbsolutePathname().c_str());
        Logger::Error("===============================================================");
        return false;
    }

    auto psdNameString = fullname.GetAbsolutePathname();
    const char* psdName = psdNameString.c_str();

    psd_context* psd = nullptr;
    auto status = psd_image_load(&psd, const_cast<psd_char*>(psdName));
    if ((psd == nullptr) || (status != psd_status_done))
    {
        Logger::Error("============================ ERROR ============================");
        Logger::Error("| Unable to load PSD from file (result code = %d):", static_cast<int>(status));
        Logger::Error("| %s", psdName);
        Logger::Error("| Try to re-save this file by using `Save as...` in Photoshop");
        Logger::Error("===============================================================");
        return false;
    }

    SCOPE_EXIT
    {
        psd_image_free(psd);
    };

    auto outImageBasePath = fullname;
    outImageBasePath.ReplaceExtension(".png");

    auto outImageBaseName = outImageBasePath.GetBasename();
    filename = processDirectoryPath + outImageBaseName + ".txt";

    frameCount = psd->layer_count;
    frameRects.resize(frameCount);
    frameNames.resize(frameCount);
    spriteWidth = psd->width;
    spriteHeight = psd->height;

    outImageBasePath.ReplaceDirectory(processDirectoryPath);
    for (uint32 lIndex = 0; lIndex < frameCount; ++lIndex)
    {
        auto& layer = psd->layer_records[lIndex];
        auto layerName = String(reinterpret_cast<const char*>(layer.layer_name));

        if (layer.layer_type != psd_layer_type_normal)
        {
            Logger::Error("============================== ERROR ==============================");
            Logger::Error("| File contains unsupported layer (%s) with type %d", layerName.c_str(), static_cast<int>(layer.layer_type));
            Logger::Error("| %s", psdName);
            Logger::Error("===================================================================");
            return false;
        }

        if (layer.width * layer.height == 0)
        {
            Logger::Error("============================== ERROR ==============================");
            Logger::Error("| File contains empty layer %d (%s)", lIndex, static_cast<int>(layer.layer_type), layerName.c_str());
            Logger::Error("| %s", psdName);
            Logger::Error("===================================================================");
            return false;
        }

        outImageBasePath.ReplaceBasename(outImageBaseName + "_" + std::to_string(lIndex));

        if (verboseOutput && layerName.empty())
        {
            Logger::Warning("=========================== WARNING ===========================");
            Logger::Warning("| PSD file: %s", psdName);
            Logger::Warning("| Contains layer without a name: %u", static_cast<int32>(lIndex));
            Logger::Warning("===============================================================");
        }
        else if (verboseOutput && std::find(frameNames.begin(), frameNames.end(), layerName) != frameNames.end())
        {
            Logger::Warning("=========================== WARNING ===========================");
            Logger::Warning("| PSD file:");
            Logger::Warning("| %s", psdName);
            Logger::Warning("| Contains two or more layers with the same name: %s", layerName.c_str());
            Logger::Warning("===============================================================");
        }

        if (layer.opacity < 255)
        {
            Logger::Warning("============================ Warning ============================");
            Logger::Warning("| File contains layer `%s` with opacity less than 100%% (%.0f)", layerName.c_str(), 100.0f * static_cast<float>(layer.opacity) / 255.0f);
            Logger::Warning("| %s", psdName);
            Logger::Warning("=================================================================");
        }

        if (verboseOutput && (layer.visible == false))
        {
            Logger::Warning("============================ WARNING ============================");
            Logger::Warning("| File contains invisible layer %d (%s)", lIndex, layerName.c_str());
            Logger::Warning("| %s", psdName);
            Logger::Warning("=================================================================");
        }

        frameNames[lIndex] = useLayerNames && !layerName.empty() ? layerName : DAVA::Format("frame%d", lIndex);

        uint32* sourceData = reinterpret_cast<uint32*>(layer.image_data);
        for (psd_int i = 0, e = layer.width * layer.height; i < e; ++i)
        {
            auto alpha = (sourceData[i] & 0xff000000) >> 24;
            alpha = static_cast<psd_uchar>(layer.opacity * alpha / 255) << 24;
            sourceData[i] = alpha | (sourceData[i] & 0x0000ff00) | (sourceData[i] & 0x000000ff) << 16 | (sourceData[i] & 0xff0000) >> 16;
        }

        auto dataToSave = reinterpret_cast<char*>(layer.image_data);
        int imageLeft = layer.left;
        int imageTop = layer.top;
        int imageWidth = layer.width;
        int imageHeight = layer.height;

        if ((layer.left < 0) || (layer.top < 0) || (layer.right > psd->width) || (layer.bottom > psd->height))
        {
            if (verboseOutput)
            {
                Logger::Warning("============================ Warning ============================");
                Logger::Warning("| File contains layer `%s` which does not fit canvas", layerName.c_str());
                Logger::Warning("| %s", psdName);
                Logger::Warning("| Layer: (%d, %d, %d, %d), canvas: (%d, %d)", layer.left, layer.top, layer.width, layer.height, psd->width, psd->height);
                Logger::Warning("=================================================================");
            }

            int xOffset = Max(0, -imageLeft);
            int yOffset = Max(0, -imageTop);

            Rect2i layerRect(imageLeft, imageTop, imageWidth, imageHeight);
            Rect2i psdRect(0, 0, psd->width, psd->height);
            Rect2i intersectedRect = psdRect.Intersection(layerRect);

            imageLeft = intersectedRect.x;
            imageTop = intersectedRect.y;
            imageWidth = intersectedRect.dx;
            imageHeight = intersectedRect.dy;

            if (intersectedRect.dx * intersectedRect.dy <= 0)
            {
                Logger::Error("============================ ERROR ============================");
                Logger::Error("| File contains completely hidden layer %d (%s)", lIndex, layerName.c_str());
                Logger::Error("| %s", psdName);
                Logger::Error("===============================================================");
                return false;
            }

            dataToSave = reinterpret_cast<char*>(calloc(imageWidth * imageHeight, 4));

            size_type rowSize = 4 * imageWidth;
            uint32* croppedRGBA = reinterpret_cast<uint32*>(dataToSave);
            for (int y = 0; y < imageHeight; ++y)
            {
                auto src = sourceData + xOffset + (yOffset + y) * layer.width;
                auto dst = croppedRGBA + y * imageWidth;
                memcpy(dst, src, rowSize);
                DVASSERT(memcmp(src, dst, rowSize) == 0);
            }
        }

        if (retainEmptyPixesl)
        {
            if ((imageLeft > 0) || (imageTop > 0) || (imageLeft + imageWidth < psd->width) || (imageTop + imageHeight < psd->height))
            {
                auto wholeImageData = calloc(psd->width * psd->height, 4);
                uint32* srcRGBA = reinterpret_cast<uint32*>(dataToSave);
                uint32* dstRGBA = reinterpret_cast<uint32*>(wholeImageData);
                size_type rowSize = 4 * imageWidth;
                for (int y = 0; y < imageHeight; ++y)
                {
                    auto src = srcRGBA + y * imageWidth;
                    auto dst = dstRGBA + imageLeft + (y + imageTop) * psd->width;
                    memcpy(dst, src, rowSize);
                    DVASSERT(memcmp(src, dst, rowSize) == 0);
                }
                if (dataToSave != reinterpret_cast<char*>(layer.image_data))
                {
                    free(dataToSave);
                }
                dataToSave = reinterpret_cast<char*>(wholeImageData);
            }
            imageLeft = 0;
            imageTop = 0;
            imageWidth = psd->width;
            imageHeight = psd->height;
        }
        frameRects[lIndex] = Rect2i(imageLeft, imageTop, imageWidth, imageHeight);

        bool writeSucceed = DefinitionFileLocal::WritePNGImage(imageWidth, imageHeight, dataToSave, outImageBasePath.GetAbsolutePathname().c_str(), 4, 8);
        if (dataToSave != reinterpret_cast<char*>(layer.image_data))
        {
            free(dataToSave);
        }

        if (writeSucceed == false)
        {
            Logger::Error("============================ ERROR ============================");
            Logger::Error("| Failed to write PNG to file:");
            Logger::Error("| %s", outImageBasePath.GetAbsolutePathname().c_str());
            Logger::Error("| Input file (layer %u):", static_cast<int32>(lIndex));
            Logger::Error("| %s", psdName);
            Logger::Error("===============================================================");
            return false;
        }
    }

    return true;
}

namespace DefinitionFileLocal
{
bool WritePNGImage(int width, int height, char* imageData, const char* outName, int channels, int bit_depth)
{
    FILE* fp = fopen(outName, "wb");
    if (fp == nullptr)
        return false;

    SCOPE_EXIT
    {
        fclose(fp);
    };

    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (png_ptr == nullptr)
        return false;

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == nullptr)
    {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        return false;
    }

    png_init_io(png_ptr, fp);

    png_byte colorType = 0;
    switch (channels)
    {
    case 1:
    {
        colorType = PNG_COLOR_TYPE_GRAY;
        break;
    }
    case 2:
    {
        colorType = PNG_COLOR_TYPE_GRAY_ALPHA;
        break;
    }

    case 3:
    {
        colorType = PNG_COLOR_TYPE_RGB;
        break;
    }
    case 4:
    {
        colorType = PNG_COLOR_TYPE_RGBA;
        break;
    }

    default:
    {
        printf("Invalid image configuration: %d channels, %d bit depth", channels, bit_depth);
        png_destroy_info_struct(png_ptr, &info_ptr);
        png_destroy_write_struct(&png_ptr, &info_ptr);
        return false;
    }
    }

    int rowSize = width * channels * bit_depth / 8;

    png_set_IHDR(png_ptr, info_ptr, width, height, bit_depth, colorType,
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    png_write_info(png_ptr, info_ptr);
    for (int y = 0; y < height; ++y)
        png_write_row(png_ptr, reinterpret_cast<png_bytep>(&imageData[y * rowSize]));
    png_write_end(png_ptr, info_ptr);
    png_destroy_info_struct(png_ptr, &info_ptr);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    return true;
}
}
}
