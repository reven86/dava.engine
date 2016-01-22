#ifndef __IMagickHelper__
#define __IMagickHelper__

#ifdef WIN32

#ifdef IMAGICKHELPER_STATIC_DEFINE
#  define IMAGICKHELPER_EXPORT
#  define IMAGICKHELPER_NO_EXPORT
#else
#  ifndef IMAGICKHELPER_EXPORT
#    ifdef IMagickHelper_EXPORTS
#      define IMAGICKHELPER_EXPORT __declspec(dllexport)
#    else
#      define IMAGICKHELPER_EXPORT __declspec(dllimport)
#    endif
#  endif
#endif

#else

#ifdef IMAGICKHELPER_STATIC_DEFINE
#  define IMAGICKHELPER_EXPORT
#  define IMAGICKHELPER_NO_EXPORT
#else
#  ifndef IMAGICKHELPER_EXPORT
#    ifdef IMagickHelper_EXPORTS
#      define IMAGICKHELPER_EXPORT __attribute__((visibility("default")))
#    else
#      define IMAGICKHELPER_EXPORT __attribute__((visibility("default")))
#    endif
#  endif
#endif

#endif

#include <string>
#include <vector>

namespace IMagickHelper
{
struct IMAGICKHELPER_EXPORT Layer
{
	static const size_t maxNameSize = 256;
    int x = 0;
    int y = 0;
    int dx = 0;
    int dy = 0;
	char name[maxNameSize];
	Layer();
    Layer(int _x, int _y, int _dx, int _dy, const std::string& _name);
};

struct IMAGICKHELPER_EXPORT CroppedData
{
    void Reset();

    int layer_width = 0;
    int layer_height = 0;
	Layer* layers = nullptr;
	size_t layers_count = 0;
};

IMAGICKHELPER_EXPORT bool ConvertToPNG(const char* in_image_path, const char* out_path);
IMAGICKHELPER_EXPORT bool ConvertToPNGCroppedGeometry(const char* in_image_path, const char* out_path, CroppedData& out_cropped_data, bool skip_first_layer = false);
}


#endif /* defined(__IMagickHelper__) */