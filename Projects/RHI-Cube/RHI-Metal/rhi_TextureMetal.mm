
    #include "../rhi_Base.h"
    #include "../RHI/rhi_Pool.h"
    #include "rhi_Metal.h"

    #include "Debug/DVAssert.h"
    #include "FileSystem/Logger.h"
    using DAVA::Logger;

    #include "_metal.h"


namespace rhi
{
//==============================================================================

struct
TextureMetal_t
{
public:
                    TextureMetal_t()
                      : data(0),
                        uid(nil),
                        is_mapped(false)
                    {}


    void*           data;
    id<MTLTexture>  uid;
    uint32          is_mapped:1;
};

typedef Pool<TextureMetal_t>    TextureMetalPool;


namespace Texture
{

//------------------------------------------------------------------------------

Handle  
Create( unsigned width, unsigned height, TextureFormat format, uint32 options )
{
    Handle                  handle;
    MTLTextureDescriptor*   desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatBGRA8Unorm width:width height:height mipmapped:NO];
    id<MTLTexture>          uid  = [_Metal_Device newTextureWithDescriptor:desc];

    if( uid != nil )
    {
        handle = TextureMetalPool::Alloc();
        
        TextureMetal_t* tex = TextureMetalPool::Get( handle );

        tex->uid    = uid;
//        tex->data   = [uid contents];
        tex->data   = ::malloc( width*height*sizeof(uint32) );
    }

    return handle;
}


//------------------------------------------------------------------------------

void
Delete( Handle tex )
{
    TextureMetal_t* self = TextureMetalPool::Get( tex );

    if( self )
    {
        ::free( self->data );
        self->data = 0;

        TextureMetalPool::Free( tex );
    }
}


//------------------------------------------------------------------------------

void*
Map( Handle tex, unsigned level )
{
    TextureMetal_t* self   = TextureMetalPool::Get( tex );
    unsigned        stride = [self->uid width] * sizeof(uint32);
    MTLRegion       rgn;

    DVASSERT(!self->is_mapped);

    rgn.origin.x    = 0;
    rgn.origin.y    = 0;
    rgn.origin.z    = 0;
    rgn.size.width  = [self->uid width];
    rgn.size.height = [self->uid height];
    rgn.size.depth  = 1;
    
    [self->uid getBytes:self->data bytesPerRow:stride fromRegion:rgn mipmapLevel:0];
    self->is_mapped = true;
    
    return self->data;
}


//------------------------------------------------------------------------------

void
Unmap( Handle tex )
{
    TextureMetal_t* self    = TextureMetalPool::Get( tex );
    MTLRegion       rgn;
    unsigned        stride  = [self->uid width] * sizeof(uint32);

    DVASSERT(self->is_mapped);

    rgn.origin.x    = 0;
    rgn.origin.y    = 0;
    rgn.origin.z    = 0;
    rgn.size.width  = [self->uid width];
    rgn.size.height = [self->uid height];
    rgn.size.depth  = 1;

    [self->uid replaceRegion:rgn mipmapLevel:0 withBytes:self->data bytesPerRow:stride];
}


//==============================================================================
} // namespace Texture


//------------------------------------------------------------------------------

namespace TextureMetal
{

void
SetToRHI( Handle tex, unsigned unitIndex, id<MTLRenderCommandEncoder> ce )
{
    TextureMetal_t* self = TextureMetalPool::Get( tex );

    [ce setFragmentTexture:self->uid atIndex:unitIndex];
}

} // namespace TextureMetal

//==============================================================================
} // namespace rhi

