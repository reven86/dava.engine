#include "IMagickHelper.h"
#include "FileTools.h"
#include <libpsd.h>
#include <png.h>
#include <cstdlib>

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

bool CreateOutputDirectoryForOutputPath(const char* out_path, size_t& out_path_len)
{
    out_path_len = (out_path == nullptr) ? 0 : strlen(out_path);
    if ((out_path_len > 0) && !FileTool::CreateDir(out_path))
    {
        printf("[IMAGE-MAGICK-HELPER ERROR]: Can't create directory: %s\n", out_path);
        return false;
    }

    return true;
}

bool WritePNGImage(int width, int height, char* imageData, const char* outName, int channels, int bit_depth)
{
    FILE* fp = fopen(outName, "wb");
    if (fp == nullptr)
    {
        return false;
    }
    
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
        png_write_row(png_ptr, (png_bytep)(&imageData[y * rowSize]));
    png_write_end(png_ptr, info_ptr);
    png_destroy_info_struct(png_ptr, &info_ptr);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    
    fclose(fp);
    return true;
}

bool ConvertToPNG(const char* in_image_path, const char* out_path)
{
    abort();
    return false;
}
    
template <typename F>
struct AtExit
{
    AtExit(F f) : func(f) { }
    ~AtExit() { func(); }
    F func;
};
    
bool ConvertToPNGCroppedGeometry(const char* in_image_path, const char* out_path, CroppedData& out_cropped_data, bool skipCompositeLayer)
{
    size_t out_path_len = 0;
    if (CreateOutputDirectoryForOutputPath(out_path, out_path_len) == false)
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
    
    psd_context* psd = nullptr;
    auto status = psd_image_load(&psd, (psd_char*)in_image_path);
    if ((psd == nullptr) || (status != psd_status_done))
    {
        fprintf(stderr, "Unable to load PSD from file %s\n", in_image_path);
        return false;
    }
    
    auto onExit = [psd](){
        psd_image_free(psd);
    };
    AtExit<decltype(onExit)> guard(onExit);
    
    out_cropped_data.Reset();
    out_cropped_data.layer_width = psd->width;
    out_cropped_data.layer_height = psd->height;
    out_cropped_data.layers_count = psd->layer_count;
    out_cropped_data.layers = new IMagickHelper::Layer[psd->layer_count]();
    
    for (int i = 0, e = psd->layer_count; i < e; ++i)
    {
        char c_buf[32] = {};
        sprintf(c_buf, "%d", static_cast<int>(i));
        out_image_path = FileTool::ReplaceBasename(out_image_path, out_image_basename + "_" + c_buf);
        
        auto& layer = psd->layer_records[i];
        
        if (layer.width * layer.height == 0)
        {
            fprintf(stderr, "=========================== WARNING ===========================\n");
            fprintf(stderr, "|\n");
            fprintf(stderr, "| File contains empty layer\n| %s\n", in_image_path);
            fprintf(stderr, "|\n");
            fprintf(stderr, "===============================================================\n");
            out_cropped_data.layers_count = 0;
            delete [] out_cropped_data.layers;
            out_cropped_data.layers = nullptr;
            return false;
        }
        
        auto& outLayer = out_cropped_data.layers[i];
        outLayer.x = layer.left;
        outLayer.y = layer.top;
        outLayer.dx = layer.width;
        outLayer.dy = layer.height;
        memcpy(outLayer.name, layer.layer_name, 256);
        uint32_t* p = reinterpret_cast<uint32_t*>(layer.image_data);
        for (int i = 0, e = layer.width * layer.height; i < e; ++i)
        {
            p[i] = (p[i] & 0x000000ff) << 16 | (p[i] & 0x0000ff00) | (p[i] & 0xff0000) >> 16 | (p[i] & 0xff000000);
        }
        
        auto data = reinterpret_cast<char*>(layer.image_data);
        if (WritePNGImage(layer.width, layer.height, data, out_image_path.c_str(), 4, 8) == false)
        {
            fprintf(stderr, "Failed to write PNG for file %s, layer %d\n", in_image_path, int(i));
            return false;
        }
    }
    
    return true;
}
}