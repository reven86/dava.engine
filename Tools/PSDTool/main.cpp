#include <vector>
#include <string>

#include "IMagickHelper.h"


class ImageConverter
{
public:
    unsigned    ComandLineProcessing ( int argc, char **argv );
    unsigned    ConvertImage         ( const std::string &image_path );
    
    ImageConverter();

private:
    const char  *_out_dir;
    bool         _croped_layers;

};


ImageConverter::ImageConverter()
{
    _out_dir          = 0;
    _croped_layers    = false;
}

unsigned ImageConverter::ComandLineProcessing( int argc, char** argv )
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
                case EOutDir      : _out_dir       = argv[i];  break;
                case ECropedLayers: _croped_layers = !strcmp( argv[i], "true" ); break;
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

    if( _croped_layers )
    {
        IMagickHelper::CroppedData cropped_data;
        result = IMagickHelper::ConvertToPNGCroppedGeometry( image_path.c_str(), _out_dir, &cropped_data, true );

        for( unsigned i = 0; i < cropped_data.rects_array_size; i++ )
        {
            DAVA::Rect2i &rect = cropped_data.rects_array[ i ];
            printf( "CroppedRect[%i] %i %i %i %i\n", i, rect.x, rect.y, rect.dx, rect.dy );
        }
    }
    else
    {
        result = IMagickHelper::ConvertToPNG( image_path.c_str(), _out_dir );
    }

    return result;
}

//D:\Dava\wot.blitz\DataSource\Gfx\Particles\exp_anim_one.psd -out_dir D:\png -cropped_layers true
int main( int argc, char **argv )
{
    ImageConverter converter;
    converter.ComandLineProcessing( argc, argv );
    return 1;
}