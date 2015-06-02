#include <vector>
#include <string>

#include "IMagickHelper.h"


class ImageConverter
{
public:
    unsigned ComandLineProcessing( int argc, const char **argv );
    unsigned ConvertImage( const std::string &image_path );
    
    ImageConverter();

private:
    const char *outDir;
    bool cropedLayers;

};


ImageConverter::ImageConverter()
{
    outDir = 0;
    cropedLayers = false;
}

unsigned ImageConverter::ComandLineProcessing( int argc, const char** argv )
{
    if( argc > 1 )
    {
        enum
        {   
            EInvalid,
            EOutDir,
            ECropedLayers
        } state = EInvalid;

        for( int i = 2; i < argc; i++ )
        {
            if( state == EInvalid )
            {
                if( !strcmp( argv[i], "-out_dir" ) )
                {
                    state = EOutDir;
                }
                else
                if( !strcmp( argv[i], "-cropped_layers" ) )
                {
                    state = ECropedLayers;
                }
            }
            else
            {
                switch( state )
                {
                case EOutDir: outDir = argv[i]; break;
                case ECropedLayers: cropedLayers = !strcmp( argv[i], "true" ); break;
                case EInvalid: break;
                }

                state = EInvalid;
            }
        }

        ConvertImage( argv[1] );
    }

    return 1;
}

unsigned ImageConverter::ConvertImage( const std::string &image_path )
{
    bool result = false;

    if( cropedLayers )
    {
        IMagickHelper::CroppedData cropped_data;
        result = IMagickHelper::ConvertToPNGCroppedGeometry( image_path.c_str(), outDir, &cropped_data, false );

        for( unsigned i = 0; i < cropped_data.rects_array_size; i++ )
        {
            IMagickHelper::Rect &rect = cropped_data.rects_array[ i ];
            printf ( "CroppedRect[%i] %i %i %i %i\n", i, rect.x, rect.y, rect.dx, rect.dy );
        }
    }
    else
    {
        result = IMagickHelper::ConvertToPNG( image_path.c_str(), outDir );
    }

    return result;
}

//D:\Dava\wot.blitz\DataSource\Gfx\Particles\exp_anim_one.psd -out_dir D:\png -cropped_layers true
int main( int argc, const char **argv )
{
    ImageConverter converter;
    converter.ComandLineProcessing( argc, argv );
    return 1;
}
