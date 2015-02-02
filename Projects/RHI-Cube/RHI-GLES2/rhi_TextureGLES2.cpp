
    #include "../rhi_Base.h"
    #include "../RHI/rhi_Pool.h"
    #include "rhi_GLES2.h"

    #include "Debug/DVAssert.h"
    #include "FileSystem/Logger.h"
    using DAVA::Logger;

    #include "_gl.h"


namespace rhi
{
//==============================================================================

class
TextureGLES2_t
{
public:

                    TextureGLES2_t();


    unsigned        _uid;

    unsigned        _width;
    unsigned        _height;
    void*           _data;
    unsigned        _is_mapped:1;
};


TextureGLES2_t::TextureGLES2_t()
  : _width(0),
    _height(0),
    _data(0),
    _is_mapped(false)
{
}

typedef Pool<TextureGLES2_t,RESOURCE_TEXTURE>   TextureGLES2Pool;
RHI_IMPL_POOL(TextureGLES2_t,RESOURCE_TEXTURE);


namespace Texture
{
//------------------------------------------------------------------------------

void
Delete( Handle tex )
{
    if( tex != InvalidHandle )
    {
        TextureGLES2_t* self = TextureGLES2Pool::Get( tex );

        DVASSERT(!self->_is_mapped);
        
        if( self->_data )
        {
            ::free( self->_data );
            
            self->_data     = 0;
            self->_width    = 0;
            self->_height   = 0;
        }

        TextureGLES2Pool::Free( tex );
    }
}


//------------------------------------------------------------------------------

Handle
Create( unsigned width, unsigned height, TextureFormat format, uint32 options )
{
    Handle  handle = InvalidHandle;
    GLuint  uid    = InvalidIndex;
    
    glGenTextures( 1, &uid );
    GL_CALL(glBindTexture( GL_TEXTURE_2D, uid ));
//    GL_CALL(glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR ));
// force single-MIP
glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0 );
glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0 );
    GL_CALL(glBindTexture( GL_TEXTURE_2D, 0 ));

    if( uid != InvalidIndex )
    {
//        void*   mem = VidMem()->alloc_aligned( width*height*4, 16 );
        void*   mem = ::malloc( width*height*4 );

        if( mem )
        {
            handle = TextureGLES2Pool::Alloc();
            
            TextureGLES2_t* tex = TextureGLES2Pool::Get( handle );

            tex->_uid       = uid;
            tex->_data      = mem;
            tex->_width     = width;
            tex->_height    = width;
            tex->_is_mapped = false;
        }
    }

    return handle;
}


//------------------------------------------------------------------------------

void*
Map( Handle tex, unsigned level )
{
    TextureGLES2_t* self = TextureGLES2Pool::Get( tex );
    void*           mem  = 0;

    DVASSERT(!self->_is_mapped);
    if( self->_data )
    {
        mem              = self->_data;
        self->_is_mapped = true;
    }

    return mem;
}


//------------------------------------------------------------------------------

void
Unmap( Handle tex )
{
    TextureGLES2_t* self = TextureGLES2Pool::Get( tex );

    DVASSERT(self->_is_mapped);

    GL_CALL(glBindTexture( GL_TEXTURE_2D, self->_uid ));
    GL_CALL(glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, self->_width, self->_height, 0, GL_BGRA, GL_UNSIGNED_BYTE, self->_data ));
    GL_CALL(glBindTexture( GL_TEXTURE_2D, 0 ));

    self->_is_mapped = false;
}

//==============================================================================
} // namespace Texture




//------------------------------------------------------------------------------

namespace TextureGLES2
{ 

void
SetToRHI( Handle tex, unsigned unit_i )
{
    TextureGLES2_t* self = TextureGLES2Pool::Get( tex );

    GL_CALL(glActiveTexture( GL_TEXTURE0+unit_i ));
    GL_CALL(glBindTexture( GL_TEXTURE_2D, self->_uid ));
}

} // namespace TextureGLES2

//==============================================================================
} // namespace rhi

