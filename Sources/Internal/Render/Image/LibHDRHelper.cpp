#include "Render/Image/LibHDRHelper.h"

#include "FileSystem/File.h"
#include "Render/Image/Image.h"
#include "Render/Image/ImageConvert.h"
#include "Utils/StringUtils.h"

namespace DAVA
{
namespace LibHDRDetails
{
const String kRadianceHeader = "#?RADIANCE";
const String kRadianceFormatEntry = "FORMAT=";
const String kRadiance32Bit_RLE_RGBE = "32-BIT_RLE_RGBE";
using RGBE = uint8[4];

uint8* ReadScanline(uint8* ptr, int width, RGBE* scanline);
Vector4 RGBEToFloat(const RGBE& data);
}

LibHDRHelper::LibHDRHelper()
    : ImageFormatInterface(ImageFormat::IMAGE_FORMAT_HDR, "Radiance (HDR)", { ".hdr" }, { FORMAT_RGBA32F })
{
}

bool LibHDRHelper::CanProcessFileInternal(File* infile) const
{
    String buffer(1024, 0);
    uint32 bytesRead = infile->ReadLine(&buffer[0], static_cast<uint32>(buffer.size()));
    ;
    return strcmp(buffer.c_str(), LibHDRDetails::kRadianceHeader.c_str()) == 0;
}

ImageInfo LibHDRHelper::GetImageInfo(File* infile) const
{
    String buffer(1024, 0);
    infile->ReadLine(&buffer[0], static_cast<uint32>(buffer.size()));
    ;
    if (strcmp(buffer.c_str(), LibHDRDetails::kRadianceHeader.c_str()))
    {
        Logger::Error("HDR file contain invalid header: %s", buffer.c_str());
        return ImageInfo();
    }

    std::fill(buffer.begin(), buffer.end(), 0);
    infile->ReadLine(&buffer[0], static_cast<uint32>(buffer.size()));
    ;
    while (!infile->IsEof() && ((buffer.front() == 0) || (buffer.find('#') == 0)))
    {
        std::fill(buffer.begin(), buffer.end(), 0);
        infile->ReadLine(&buffer[0], static_cast<uint32>(buffer.size()));
        ;
    }
    std::transform(buffer.begin(), buffer.end(), buffer.begin(), toupper);

    if (buffer.find(LibHDRDetails::kRadianceFormatEntry) != 0)
    {
        Logger::Error("HDR file contain invalid format signature: %s", buffer.c_str());
        return ImageInfo();
    }

    String format = buffer.substr(LibHDRDetails::kRadianceFormatEntry.size(), LibHDRDetails::kRadiance32Bit_RLE_RGBE.size());
    if (format != LibHDRDetails::kRadiance32Bit_RLE_RGBE)
    {
        Logger::Error("Invalid (or unsupported format) in HDR: %s", buffer.c_str());
        return ImageInfo();
    }

    std::fill(buffer.begin(), buffer.end(), 0);
    infile->ReadLine(&buffer[0], static_cast<uint32>(buffer.size()));
    ;
    while (!infile->IsEof() && ((buffer.front() == 0) || (buffer.find('#') == 0)))
    {
        std::fill(buffer.begin(), buffer.end(), 0);
        infile->ReadLine(&buffer[0], static_cast<uint32>(buffer.size()));
        ;
    }
    buffer.erase(std::remove_if(buffer.begin(), buffer.end(), [](int8 c) {
                     return std::isspace(c) != 0;
                 }),
                 buffer.end());
    std::transform(buffer.begin(), buffer.end(), buffer.begin(), toupper);

    size_t xpos = buffer.find('X');
    size_t ypos = buffer.find('Y');
    if ((xpos == String::npos) || (ypos == String::npos))
    {
        Logger::Error("No dimension specified (or can not be parsed) in HDR file: %s", buffer.c_str());
        return ImageInfo();
    }

    String ws;
    String hs;
    if (xpos < ypos)
    {
        if (ypos < xpos + 2)
        {
            Logger::Error("Invalid dimension format in HDR file: %s", buffer.c_str());
            return ImageInfo();
        }
        ws = buffer.substr(xpos + 1, ypos - xpos - 2);
        hs = buffer.substr(ypos + 1);
    }
    else
    {
        if (xpos < ypos + 2)
        {
            Logger::Error("Invalid dimension format in HDR file: %s", buffer.c_str());
            return ImageInfo();
        }
        hs = buffer.substr(ypos + 1, xpos - ypos - 2);
        ws = buffer.substr(xpos + 1);
    }

    ImageInfo result;
    result.format = PixelFormat::FORMAT_RGBA32F;
    result.faceCount = 1;
    result.mipmapsCount = 1;
    result.width = ParseStringTo<uint32>(ws);
    result.height = ParseStringTo<uint32>(hs);
    result.dataSize = 4 * sizeof(float32) * result.width * result.height;
    return result;
}

eErrorCode LibHDRHelper::ReadFile(File* infile, Vector<Image*>& imageSet, const ImageSystem::LoadingParams& loadingParams) const
{
    ImageInfo desc = GetImageInfo(infile);

    if ((desc.width < 8) || (desc.width > 0x7fff))
        return eErrorCode::ERROR_READ_FAIL;

    Vector<LibHDRDetails::RGBE> rgbeData(desc.width * desc.height);
    {
        // inData contains RLE compressed data, provided size is just an estimation of the buffer
        // actual decoded data will be placed into `rgbeData` container
        Vector<uint8> inData(ImageUtils::GetSizeInBytes(desc.width, desc.height, PixelFormat::FORMAT_RGBA8888));
        infile->Read(inData.data(), static_cast<uint32>(inData.size()));

        uint8* ptr = inData.data();
        for (uint32 y = 0; y < desc.height; ++y)
        {
            LibHDRDetails::RGBE* rowPtr = rgbeData.data() + desc.width * (desc.height - 1 - y);
            ptr = LibHDRDetails::ReadScanline(ptr, desc.width, rowPtr);
        }
    }

    Image* image = Image::Create(desc.width, desc.height, PixelFormat::FORMAT_RGBA32F);
    Vector4* floatData = reinterpret_cast<Vector4*>(image->data);
    for (const LibHDRDetails::RGBE& rgbe : rgbeData)
        *floatData++ = LibHDRDetails::RGBEToFloat(rgbe);

    imageSet.push_back(image);
    return eErrorCode::SUCCESS;
}

eErrorCode LibHDRHelper::WriteFileAsCubeMap(const FilePath& fileName, const Vector<Vector<Image*>>& imageSet, PixelFormat pixelFormat, ImageQuality quality) const
{
    Logger::Error("[%s] Writing is not yet supported for HDR", __FUNCTION__);
    return DAVA::eErrorCode::ERROR_WRITE_FAIL;
}

eErrorCode LibHDRHelper::WriteFile(const FilePath& fileName, const Vector<Image*>& imageSet, PixelFormat piexelFormat, ImageQuality quality) const
{
    Logger::Error("[%s] Writing is not yet supported for HDR", __FUNCTION__);
    return DAVA::eErrorCode::ERROR_WRITE_FAIL;
}

/*
 * Internal implementation
 */
namespace LibHDRDetails
{
uint8* ReadScanline(uint8* ptr, int width, RGBE* scanline)
{
    if (*ptr++ == 2)
    {
        (*scanline)[1] = *ptr++;
        (*scanline)[2] = *ptr++;

        ++ptr;

        for (int i = 0; i < 4; ++i)
        {
            for (int j = 0; j < width;)
            {
                uint8 code = *ptr++;
                if (code > 128)
                {
                    code &= 127;
                    uint8 val = *ptr++;
                    while (code--)
                        scanline[j++][i] = val;
                }
                else
                {
                    while (code--)
                        scanline[j++][i] = *ptr++;
                }
            }
        }
    }
    else
    {
        DVASSERT(0, "Legacy HDR files are not supported");
    }

    return ptr;
}

inline float32 ConvertComponent(int8 expo, uint8 val)
{
    return (expo > 0) ? static_cast<float32>(val * (1 << expo)) : static_cast<float32>(val) / static_cast<float32>(1 << -expo);
}

Vector4 RGBEToFloat(const RGBE& data)
{
    int8 expo = data[3] - 128;
    return Vector4(ConvertComponent(expo, data[0]), ConvertComponent(expo, data[1]), ConvertComponent(expo, data[2]), 256.0f) / 256.0f;
}
}
};
