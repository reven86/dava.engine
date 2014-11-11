#include "IMagickHelper.h"

#include <Magick++.h>
#include <magick/MagickCore.h>
#include <magick/property.h>

#include "FileSystem/FilePath.h"
#include "FileSystem/FileSystem.h"

#include <stdio.h>
#include <string>

using namespace DAVA;

namespace IMagickHelper
{

void CroppedData::Reset()
{
    if( rects_array!= 0 )
    {
        free( rects_array );
    }

    layer_width      = 0;
    layer_height     = 0;
    rects_array_size = 0;
}

CroppedData::CroppedData() :
    rects_array     ( 0 )

{
    Reset();
}

CroppedData::~CroppedData()
{
    Reset();
}

class Initialization
{
public:
    Initialization()
    {
        new FileSystem();

    }
    ~Initialization()
    {
        FileSystem::Instance()->Release();
    }

} _Initialization;


bool ConvertToPNG ( const char *in_image_path, const char *out_path )
{
    try 
    {
        unsigned  out_path_len = out_path != 0 ? strlen( out_path ) : 0;

        if(    out_path_len
            && !FileSystem::Instance()->CreateDirectory( out_path, true )
          )
        {
             Logger::Error("Can't create directory: %s", out_path );
             return false;
        }

        Vector<Magick::Image> layers;
        FilePath out_image_path = FilePath::CreateWithNewExtension( in_image_path, ".png" );

        if( out_path_len )
        {
            out_image_path.ReplaceDirectory( String( out_path )  );
        }

        Magick::readImages( &layers, in_image_path );

        if (layers.size() == 0)
        {
            Logger::Error("Number of layers is too low: %s",in_image_path );
            return false;
        }

        Magick::Image & image = layers[0];

        int width  = (int)image.columns();
        int height = (int)image.rows();

        image.crop(Magick::Geometry(width,height, 0, 0));
        image.magick( "PNG" );

        image.write( out_image_path.GetAbsolutePathname() );
    }
    catch( Magick::Exception &error_ )
    {
        Logger::Error("Caught exception: %s file: %s", error_.what(), in_image_path );
        return false;
    }

    return true;
}


bool ConvertToPNGCroppedGeometry ( const char *in_image_path, const char *out_path, CroppedData *out_cropped_data, bool skip_first_layer )
{
    try 
    {
        unsigned  out_path_len = out_path != 0 ? strlen( out_path ) : 0;

        if(    out_path_len
            && !FileSystem::Instance()->CreateDirectory( out_path, true )
          )
        {
            Logger::Error( "Can't create directory: %s", out_path );
            return false;
        }

        Vector<Magick::Image> layers;
        Magick::readImages( &layers, in_image_path );

        if (layers.size() == 0)
        {
            Logger::Error("Number of layers is too low: %s",in_image_path );
            return false;
        }

        if ( skip_first_layer && layers.size() == 1)
        {
            layers.push_back(layers[0]);
        }
        
        char c_buf[32];

        int width = (int)layers[0].columns();
        int height = (int)layers[0].rows();

        FilePath out_image_base_path = FilePath::CreateWithNewExtension( in_image_path, ".png" );

        for(int k = skip_first_layer ? 1 : 0; k < (int)layers.size(); ++k)
        {
            FilePath out_image_path = out_image_base_path;
            
            if( out_path_len )
            {
                out_image_path.ReplaceDirectory( String( out_path )  );
            }
                        
            sprintf( c_buf, "%i", k - (skip_first_layer ? 1 : 0 ) );
            
            out_image_path.ReplaceBasename( out_image_path.GetBasename()
                                          + "_"
                                          + c_buf
                                          );

            Magick::Image & currentLayer = layers[k];

            const Magick::Geometry croppedGeometry( width, height, 0, 0 );
            currentLayer.crop( croppedGeometry );
            currentLayer.magick( "PNG" );
            currentLayer.write( out_image_path.GetAbsolutePathname().c_str()  );
        }

        if( out_cropped_data != 0 )
        {
            out_cropped_data->Reset();
            out_cropped_data->layer_width       = width;
            out_cropped_data->layer_height      = height;
            out_cropped_data->rects_array_size  = (unsigned) layers.size();
            out_cropped_data->rects_array       = (Rect2i*)malloc(  layers.size() * sizeof( Rect2i ) );

            Rect2i *rects_array = out_cropped_data->rects_array;

            for(int k = 0; k < (int)layers.size(); ++k)
            {
                Magick::Image & currentLayer = layers[k];
                Magick::Geometry bbox = currentLayer.page();
 
                int xOff = (int)bbox.xOff() * ( bbox.xNegative() ? -1 : 1 );
                int yOff = (int)bbox.yOff() * ( bbox.yNegative() ? -1 : 1 );

                rects_array[ k ] = Rect2i(xOff, yOff, (int32)bbox.width(), (int32)bbox.height());
            }
        }
    }
    catch( Magick::Exception &error_ )
    {
        Logger::Error("Caught exception: %s file: %s", error_.what(), in_image_path );
        return 0;
    }

    return true;
}

}