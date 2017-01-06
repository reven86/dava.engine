#include "Render/Image/LibHDRHelper.h"

#include "FileSystem/File.h"
#include "Render/Image/Image.h"
#include "Render/Image/ImageConvert.h"

namespace DAVA
{
namespace LibHDRDetails
{
const String kRadianceHeader = "#?RADIANCE";
const String kRadianceFormatEntry = "FORMAT=";
const String kRadiance32Bit_RLE_RGBE = "32-BIT_RLE_RGBE";
using RGBE = uint8[4];

uint8* readScanline(uint8* ptr, int width, RGBE* scanline);
Vector4 rgbeToFloat(const RGBE& data);
}

LibHDRHelper::LibHDRHelper()
    : ImageFormatInterface(ImageFormat::IMAGE_FORMAT_HDR, "Radiance (HDR)", { ".hdr" }, { FORMAT_RGBA32F })
{
}

ImageInfo LibHDRHelper::GetImageInfo(File* infile) const
{
    String buffer;
    buffer.resize(1024);
    infile->ReadLine(&buffer[0], buffer.size());

    if (strcmp(buffer.c_str(), LibHDRDetails::kRadianceHeader.c_str()))
        return ImageInfo();

    std::fill(buffer.begin(), buffer.end(), 0);
    infile->ReadLine(&buffer[0], buffer.size());
    while ((buffer[0] == 0) || (buffer.find('#') == 0))
    {
        std::fill(buffer.begin(), buffer.end(), 0);
        infile->ReadLine(&buffer[0], buffer.size());
    }
    std::transform(buffer.begin(), buffer.end(), buffer.begin(), toupper);

    if (buffer.find(LibHDRDetails::kRadianceFormatEntry) != 0)
        return ImageInfo();

    std::string format = buffer.substr(LibHDRDetails::kRadianceFormatEntry.size(), LibHDRDetails::kRadiance32Bit_RLE_RGBE.size());
    if (format != LibHDRDetails::kRadiance32Bit_RLE_RGBE)
        return ImageInfo();

    std::fill(buffer.begin(), buffer.end(), 0);
    infile->ReadLine(&buffer[0], buffer.size());
    while ((buffer[0] == 0) || (buffer.find('#') == 0))
    {
        std::fill(buffer.begin(), buffer.end(), 0);
        infile->ReadLine(&buffer[0], buffer.size());
    }
    std::transform(buffer.begin(), buffer.end(), buffer.begin(), toupper);
    // line = removeWhitespace(line);

    size_t xpos = buffer.find('X');
    size_t ypos = buffer.find('Y');
    if ((xpos == std::string::npos) || (ypos == std::string::npos))
        return ImageInfo();

    String ws;
    String hs;
    if (xpos < ypos)
    {
        ws = buffer.substr(xpos + 1, ypos - xpos - 2);
        hs = buffer.substr(ypos + 1);
    }
    else
    {
        hs = buffer.substr(ypos + 1, xpos - ypos - 2);
        ws = buffer.substr(xpos + 1);
    }

    ImageInfo result;
    result.format = PixelFormat::FORMAT_RGBA32F;
    result.faceCount = 1;
    result.mipmapsCount = 1;
    result.width = ParseStringTo<uint32>(ws);
    result.height = ParseStringTo<uint32>(hs);
    result.dataSize = 4 * sizeof(float) * result.width * result.height;
    return result;
}

eErrorCode LibHDRHelper::ReadFile(File* infile, Vector<Image*>& imageSet, const ImageSystem::LoadingParams& loadingParams) const
{
    ImageInfo desc = GetImageInfo(infile);

    if ((desc.width < 8) || (desc.width > 0x7fff))
        return eErrorCode::ERROR_READ_FAIL;

    Vector<uint8> inData(ImageUtils::GetSizeInBytes(desc.width, desc.height, PixelFormat::FORMAT_RGBA8888));
    infile->Read(inData.data(), inData.size()); /* read RLE compressed data */

    Vector<LibHDRDetails::RGBE> rgbeData(desc.width * desc.height); /* stores RGBE data */

    uint8* ptr = inData.data();
    for (int32 y = 0; y < desc.height; ++y)
    {
        LibHDRDetails::RGBE* rowPtr = rgbeData.data() + desc.width * (desc.height - 1 - y);
        ptr = LibHDRDetails::readScanline(ptr, desc.width, rowPtr); // TODO : optimize, convert to rgba32f immediately
    }

    Image* image = Image::Create(desc.width, desc.height, PixelFormat::FORMAT_RGBA32F);
    Vector4* floatData = reinterpret_cast<Vector4*>(image->data);
    for (uint32 i = 0; i < desc.width * desc.height; ++i)
        floatData[i] = LibHDRDetails::rgbeToFloat(rgbeData[i]);

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
uint8* readScanline(uint8* ptr, int width, RGBE* scanline)
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
                unsigned char code = *ptr++;
                if (code > 128)
                {
                    code &= 127;
                    unsigned char val = *ptr++;
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

inline float convertComponent(int8 expo, uint8 val)
{
    return (expo > 0) ? static_cast<float>(val * (1 << expo)) : static_cast<float>(val) / static_cast<float>(1 << -expo);
}

Vector4 rgbeToFloat(const RGBE& data)
{
    int8 expo = data[3] - 128;
    return Vector4(convertComponent(expo, data[0]), convertComponent(expo, data[1]), convertComponent(expo, data[2]), 256.0f) / 256.0f;
}
}
};
