#include "IMagickHelper.h"

#include <Magick++.h>
#include <magick/MagickCore.h>
#include <magick/property.h>

#include <FileTools.h>
#include <stdio.h>
#include <vector>

using namespace std;

namespace IMagickHelper
{

Layer::Layer()
{
    x  = 0;
    y  = 0;
    dx = 0;
    dy = 0;

	name[0] = 0;
}

Layer::Layer( int _x, int _y, int _dx, int _dy )
{
    x  = _x;
    y  = _y;
    dx = _dx;
    dy = _dy;

	name[0] = 0;
}

void CroppedData::Reset()
{
    if( layers_array!= 0 )
    {
        free( layers_array );
    }

    layer_width      = 0;
    layer_height     = 0;
    layers_array_size = 0;
}

CroppedData::CroppedData() :
    layers_array     ( 0 )

{
    Reset();
}

CroppedData::~CroppedData()
{
    Reset();
}

bool ConvertToPNG ( const char *in_image_path, const char *out_path )
{

    try 
    {
        unsigned  out_path_len = out_path != 0 ? strlen( out_path ) : 0;

        if(    out_path_len
            && !FileTool::CreateDir( out_path )
          )
        {
             printf("Can't create directory: %s", out_path );
             return false;
        }

        std::vector<Magick::Image> layers;
        string out_image_path = FileTool::WithNewExtension( in_image_path, ".png" );

        if( out_path_len )
        {
            out_image_path = FileTool::ReplaceDirectory( out_image_path, out_path  );
        }

        Magick::readImages( &layers, in_image_path );

        if (layers.size() == 0)
        {
            printf("Number of layers is too low: %s",in_image_path );
            return false;
        }

        Magick::Image & image = layers[0];

        int width  = (int)image.columns();
        int height = (int)image.rows();

        image.crop(Magick::Geometry(width,height, 0, 0));
        image.magick( "PNG" );

        image.write( out_image_path.c_str() );
    }
    catch( Magick::Exception &error_ )
    {
        printf("Caught exception: %s file: %s", error_.what(), in_image_path );
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
            && !FileTool::CreateDir( out_path )
          )
        {
            printf( "Can't create directory: %s", out_path );
            return false;
        }

        std::vector<Magick::Image> layers;
        Magick::readImages( &layers, in_image_path );

        if (layers.size() == 0)
        {
            printf("Number of layers is too low: %s",in_image_path );
            return false;
        }

        if ( skip_first_layer && layers.size() == 1)
        {
            layers.push_back(layers[0]);
        }
        
        char c_buf[32];

        int width = (int)layers[0].columns();
        int height = (int)layers[0].rows();

        std::string out_image_base_path = FileTool::WithNewExtension( in_image_path, ".png" );

        std::string out_image_basename = FileTool::GetBasename( out_image_base_path );

        for(int k = skip_first_layer ? 1 : 0; k < (int)layers.size(); ++k)
        {
            std::string out_image_path = out_image_base_path;
            
            if( out_path_len )
            {
                out_image_path = FileTool::ReplaceDirectory( out_image_path, out_path  );
            }
                        
            sprintf( c_buf, "%i", k - (skip_first_layer ? 1 : 0 ) );
            
            out_image_path = FileTool::ReplaceBasename( out_image_path, out_image_basename
                                          + "_"
                                          + c_buf
                                          );

            Magick::Image & currentLayer = layers[k];

            const Magick::Geometry croppedGeometry( width, height, 0, 0 );
            currentLayer.crop( croppedGeometry );
            currentLayer.magick( "PNG" );
            currentLayer.write( out_image_path.c_str()  );
        }

        if( out_cropped_data != 0 )
        {
            out_cropped_data->Reset();
            out_cropped_data->layer_width       = width;
            out_cropped_data->layer_height      = height;
            out_cropped_data->layers_array_size  = (unsigned) layers.size();
            out_cropped_data->layers_array       = (Layer*)malloc(  layers.size() * sizeof( Layer ) );

            Layer *rects_array = out_cropped_data->layers_array;

            for(int k = 0; k < (int)layers.size(); ++k)
            {
                Magick::Image & currentLayer = layers[k];
                Magick::Geometry bbox = currentLayer.page();
 
                int xOff = (int)bbox.xOff() * ( bbox.xNegative() ? -1 : 1 );
                int yOff = (int)bbox.yOff() * ( bbox.yNegative() ? -1 : 1 );

                rects_array[k] = Layer(xOff, yOff, (int)bbox.width(), (int)bbox.height());
				strncpy(rects_array[k].name, currentLayer.label().c_str(), Layer::NAME_SIZE - 1);
				rects_array[k].name[Layer::NAME_SIZE - 1] = 0;
            }
        }
    }
    catch( Magick::Exception &error_ )
    {
        printf("Caught exception: %s file: %s", error_.what(), in_image_path );
        return 0;
    }

    return true;
}

}