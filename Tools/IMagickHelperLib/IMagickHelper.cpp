#include "IMagickHelper.h"
#include <FileTools.h>
#include <Magick++.h>
#include <cstdlib>
#include <cstring>

using namespace std;

namespace IMagickHelper
{
Layer::Layer()
{
    memset(name, 0, sizeof(name));
}

Layer::Layer(int _x, int _y, int _dx, int _dy, const char* _name)
    : x(_x)
    , y(_y)
    , dx(_dx)
    , dy(_dy)
{
    memset(name, 0, sizeof(name));
    if (_name != nullptr)
    {
        size_t nameLength = strlen(_name);
        if (nameLength + 1 > MAX_NAME_SIZE)
            nameLength = MAX_NAME_SIZE - 1;
        memcpy(name, _name, nameLength);
    }
}

void CroppedData::Reset()
{
    layer_width = 0;
    layer_height = 0;
    layers_count = 0;
    delete[] layers;
}

namespace
{
inline bool ReadImage(std::vector<Magick::Image>& layers, const char* inName)
{
    try
    {
        Magick::readImages(&layers, inName);
    }
    catch (Magick::Warning& warning_)
    {
        printf("\n[IMAGE-MAGICK WARNING] Reading file %s\n: %s\n", inName, warning_.what());
    }
    catch (Magick::Error& error_)
    {
        printf("\n[IMAGE-MAGICK ERROR] Reading file %s\n: %s\n", inName, error_.what());
        return false;
    }
    catch (Magick::Exception& other_)
    {
        printf("\n[IMAGE-MAGICK EXCEPTION] Reading file %s\n: %s\n", inName, other_.what());
        return false;
    }

    if (layers.empty())
    {
        printf("[IMAGE-MAGICK ERROR]: Image being read does not contain any layers:\n%s", inName);
        return false;
    }

    return true;
}

inline bool WritePNGImage(Magick::Image& image, const std::string& outName, const char* inName)
{
    try
    {
        image.magick("PNG");
        image.write(outName);
    }
    catch (Magick::Warning&)
    {
        // silently ignore warnings
        // usual case - warning about RGB profile in grayscale image
        // image still will be saved correctly
    }
    catch (Magick::Error& error_)
    {
        printf("\n[IMAGE-MAGICK ERROR] %s\nWriting file %s\n:from source: %s", error_.what(), outName.c_str(), inName);
        return false;
    }
    catch (Magick::Exception& other_)
    {
        printf("\n[IMAGE-MAGICK EXCEPTION] %s\nWriting file %s\n:from source: %s", other_.what(), outName.c_str(), inName);
        return false;
    }

    return true;
}

inline bool CreateOutputDirectoryForOutputPath(const char* out_path, size_t& out_path_len)
{
    out_path_len = (out_path == nullptr) ? 0 : strlen(out_path);
    if ((out_path_len > 0) && !FileTool::CreateDir(out_path))
    {
        printf("[IMAGE-MAGICK-HELPER ERROR]: Can't create directory: %s\n", out_path);
        return false;
    }

    return true;
}
}

bool ConvertToPNG(const char* in_image_path, const char* out_path)
{
    size_t out_path_len = 0;
    if (CreateOutputDirectoryForOutputPath(out_path, out_path_len) == false)
    {
        return false;
    }

    std::vector<Magick::Image> layers;
    if (ReadImage(layers, in_image_path) == false)
    {
        return false;
    }

    string out_image_path = FileTool::WithNewExtension(in_image_path, ".png");
    if (out_path_len > 0)
    {
        out_image_path = FileTool::ReplaceDirectory(out_image_path, out_path);
    }

    Magick::Image& image = layers.front();
    int width = (int)image.columns();
    int height = (int)image.rows();
    image.crop(Magick::Geometry(width, height, 0, 0));
    return WritePNGImage(image, out_image_path, in_image_path);
}

bool ConvertToPNGCroppedGeometry(const char* in_image_path, const char* out_path, CroppedData& out_cropped_data, bool skipCompositeLayer)
{
    size_t out_path_len = 0;
    if (CreateOutputDirectoryForOutputPath(out_path, out_path_len) == false)
    {
        return false;
    }

    std::vector<Magick::Image> layers;
    if (ReadImage(layers, in_image_path) == false)
    {
        return false;
    }

    std::string out_image_base_path = FileTool::WithNewExtension(in_image_path, ".png");
    std::string out_image_basename = FileTool::GetBasename(out_image_base_path);
    std::string out_image_path = out_image_base_path;
    if (out_path_len > 0)
    {
        out_image_path = FileTool::ReplaceDirectory(out_image_path, out_path);
    }

    size_t width = layers.front().columns();
    size_t height = layers.front().rows();
    if (skipCompositeLayer && layers.size() > 1)
    {
        layers.erase(layers.begin());
    }

    int layerIndex = 0;
    for (auto& currentLayer : layers)
    {
        char c_buf[32] = {};
        sprintf(c_buf, "%d", layerIndex);
        out_image_path = FileTool::ReplaceBasename(out_image_path, out_image_basename + "_" + c_buf);
        currentLayer.crop(Magick::Geometry(width, height, 0, 0));
        if (WritePNGImage(currentLayer, out_image_path, in_image_path) == false)
        {
            return false;
        }
        ++layerIndex;
    }

    out_cropped_data.Reset();
    out_cropped_data.layer_width = static_cast<int>(width);
    out_cropped_data.layer_height = static_cast<int>(height);
    out_cropped_data.layers_count = layers.size();
    out_cropped_data.layers = new Layer[layers.size()]();

    Layer* layer = out_cropped_data.layers;
    for (const auto& currentLayer : layers)
    {
        Magick::Geometry bbox = currentLayer.page();
        int xOff = static_cast<int>(bbox.xOff()) * (bbox.xNegative() ? -1 : 1);
        int yOff = static_cast<int>(bbox.yOff()) * (bbox.yNegative() ? -1 : 1);
        *layer++ = Layer(xOff, yOff, (int)bbox.width(), (int)bbox.height(), currentLayer.label().c_str());
    }

    return true;
}

}