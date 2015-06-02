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


namespace IMagickHelper
{
    struct Layer
    {
		static const int NAME_SIZE = 255;

        int x, y;
        int dx, dy;

		char name[NAME_SIZE];

        Layer();
        Layer( int _x, int _y, int _dx, int _dy );
    };
  
    struct IMAGICKHELPER_EXPORT CroppedData
    {
        int layer_width ;
        int layer_height;

        Layer *layers_array;

        unsigned layers_array_size;
        
        void Reset();

        CroppedData() ;
        ~CroppedData();
    };

    IMAGICKHELPER_EXPORT bool ConvertToPNG ( const char *in_image_path, const char *out_path );
    IMAGICKHELPER_EXPORT bool ConvertToPNGCroppedGeometry ( const char *in_image_path, const char *out_path, CroppedData *out_cropped_data, bool skip_first_layer = false );
}


#endif /* defined(__IMagickHelper__) */