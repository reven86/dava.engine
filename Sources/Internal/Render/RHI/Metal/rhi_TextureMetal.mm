
    #include "../Common/rhi_Private.h"
    #include "../Common/rhi_Pool.h"
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
                      : mappedData(nullptr),
                        uid(nil),
                        is_mapped(false),
                        is_renderable(false)
                    {}


    TextureFormat   format;
    uint32          width;
    uint32          height;
    void*           mappedData;
    uint32          mappedLevel;
    uint32          mappedSlice;
    id<MTLTexture>  uid;
    uint32          is_mapped:1;
    uint32          is_renderable:1;
    uint32          is_cubemap:1;
};

typedef Pool<TextureMetal_t,RESOURCE_TEXTURE>    TextureMetalPool;
RHI_IMPL_POOL(TextureMetal_t,RESOURCE_TEXTURE);


//------------------------------------------------------------------------------

static MTLPixelFormat
MetalTextureFormat( TextureFormat format )
{
    switch( format )
    {
//        case TEXTURE_FORMAT_R8G8B8      : return MTLPixelFormatBGRA8Unorm;
//        case TEXTURE_FORMAT_A8R8G8B8    : return MTLPixelFormatBGRA8Unorm;
        case TEXTURE_FORMAT_R8G8B8A8    : return MTLPixelFormatRGBA8Unorm;
//        TEXTURE_FORMAT_X8R8G8B8,

        case TEXTURE_FORMAT_R5G5B5A1    : return MTLPixelFormatA1BGR5Unorm;
        case TEXTURE_FORMAT_R5G6B5      : return MTLPixelFormatB5G6R5Unorm;

        case TEXTURE_FORMAT_R4G4B4A4    : return MTLPixelFormatABGR4Unorm;

//        TEXTURE_FORMAT_A16R16G16B16,
//        TEXTURE_FORMAT_A32R32G32B32,

//        TEXTURE_FORMAT_R8,
//        TEXTURE_FORMAT_R16,

        case TEXTURE_FORMAT_PVRTC2_4BPP_RGB     : return MTLPixelFormatPVRTC_RGB_4BPP;
        case TEXTURE_FORMAT_PVRTC2_4BPP_RGBA    : return MTLPixelFormatPVRTC_RGBA_4BPP;
        case TEXTURE_FORMAT_PVRTC2_2BPP_RGB     : return MTLPixelFormatPVRTC_RGB_2BPP;
        case TEXTURE_FORMAT_PVRTC2_2BPP_RGBA    : return MTLPixelFormatPVRTC_RGBA_2BPP;

//        TEXTURE_FORMAT_ATC_RGB,
//        TEXTURE_FORMAT_ATC_RGBA_EXPLICIT,
//        TEXTURE_FORMAT_ATC_RGBA_INTERPOLATED,

//        TEXTURE_FORMAT_ETC1,
        case TEXTURE_FORMAT_ETC2_R8G8B8         : return MTLPixelFormatETC2_RGB8;
//        case TEXTURE_FORMAT_ETC2_R8G8B8A8       : pf = MTLPixelFormatETC2_RGBA8; break;
        case TEXTURE_FORMAT_ETC2_R8G8B8A1       : return MTLPixelFormatETC2_RGB8A1;

        case TEXTURE_FORMAT_EAC_R11_UNSIGNED    : return MTLPixelFormatEAC_R11Unorm;
        case TEXTURE_FORMAT_EAC_R11_SIGNED      : return MTLPixelFormatEAC_R11Snorm;
//        case TEXTURE_FORMAT_EAC_R11G11_UNSIGNED : pf = MTLPixelFormatEAC_R11G11Unorm; break;
//        case TEXTURE_FORMAT_EAC_R11G11_SIGNED   : pf = MTLPixelFormatEAC_R11G11Snorm; break;

        case TEXTURE_FORMAT_D16                 : return MTLPixelFormatR16Unorm;
//        TEXTURE_FORMAT_D24S8

        default:
            return MTLPixelFormatInvalid;
    }
}


//------------------------------------------------------------------------------

static Handle
metal_Texture_Create( const Texture::Descriptor& texDesc )
{
    DVASSERT(texDesc.levelCount);

    Handle                  handle = InvalidHandle;
    MTLPixelFormat          pf     = MetalTextureFormat( texDesc.format );
    MTLTextureDescriptor*   desc   = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:pf width:texDesc.width height:texDesc.height mipmapped:NO];
    
    desc.textureType      = (texDesc.type == TEXTURE_TYPE_CUBE)  ? MTLTextureTypeCube : MTLTextureType2D;
    desc.mipmapLevelCount = texDesc.levelCount;
    
    id<MTLTexture>  uid = [_Metal_Device newTextureWithDescriptor:desc];

    if( uid != nil )
    {
        handle = TextureMetalPool::Alloc();
        
        TextureMetal_t* tex = TextureMetalPool::Get( handle );

        tex->format         = texDesc.format;
        tex->width          = texDesc.width;
        tex->height         = texDesc.height;
        tex->uid            = uid;
        tex->is_mapped      = false;
        tex->is_renderable  = texDesc.isRenderTarget;
        tex->is_cubemap     = texDesc.type == TEXTURE_TYPE_CUBE;
        
        if( !tex->is_renderable )
            tex->mappedData = ::malloc( TextureSize( texDesc.format, texDesc.width, texDesc.height, 0 ) );
    }

    return handle;
}


//------------------------------------------------------------------------------

static void
metal_Texture_Delete( Handle tex )
{
    TextureMetal_t* self = TextureMetalPool::Get( tex );

    if( self )
    {
        if( self->mappedData )
        {
            ::free( self->mappedData );
            self->mappedData = 0;
        }

        TextureMetalPool::Free( tex );
    }
}


//------------------------------------------------------------------------------

static void*
metal_Texture_Map( Handle tex, unsigned level, TextureFace face )
{
    TextureMetal_t* self   = TextureMetalPool::Get( tex );
    MTLRegion       rgn;
    uint32          stride = TextureStride( self->format, Size2i([self->uid width],[self->uid height]), level );
    Size2i          ext    = TextureExtents( Size2i([self->uid width],[self->uid height]), level );

    DVASSERT(!self->is_renderable);
    DVASSERT(!self->is_mapped);

    rgn.origin.x    = 0;
    rgn.origin.y    = 0;
    rgn.origin.z    = 0;
    rgn.size.width  = ext.dx;
    rgn.size.height = ext.dy;
    rgn.size.depth  = 1;
    
    if( self->is_cubemap )
    {
        NSUInteger  slice = 0;
        unsigned    sz    = TextureSize( self->format, [self->uid width], [self->uid height], level );
        
        switch( face )
        {
            case TEXTURE_FACE_LEFT   : slice = 0; break;
            case TEXTURE_FACE_RIGHT  : slice = 1; break;
            case TEXTURE_FACE_FRONT  : slice = 2; break;
            case TEXTURE_FACE_BACK   : slice = 3; break;
            case TEXTURE_FACE_TOP    : slice = 4; break;
            case TEXTURE_FACE_BOTTOM : slice = 5; break;
        }
        
        [self->uid getBytes:self->mappedData bytesPerRow:stride bytesPerImage:sz fromRegion:rgn mipmapLevel:level slice:slice];
        self->mappedSlice = slice;
    }
    else
    {
        [self->uid getBytes:self->mappedData bytesPerRow:stride fromRegion:rgn mipmapLevel:level];
    }
    
    self->is_mapped   = true;
    self->mappedLevel = level;
    
    return self->mappedData;
}


//------------------------------------------------------------------------------

static void
metal_Texture_Unmap( Handle tex )
{
    TextureMetal_t* self    = TextureMetalPool::Get( tex );
    MTLRegion       rgn;
    uint32          stride = TextureStride( self->format, Size2i([self->uid width],[self->uid height]), self->mappedLevel );
    Size2i          ext    = TextureExtents( Size2i([self->uid width],[self->uid height]), self->mappedLevel );

    DVASSERT(self->is_mapped);

    rgn.origin.x    = 0;
    rgn.origin.y    = 0;
    rgn.origin.z    = 0;
    rgn.size.width  = ext.dx;
    rgn.size.height = ext.dy;
    rgn.size.depth  = 1;
    
    if( self->is_cubemap )
    {
        unsigned    sz = TextureSize( self->format, [self->uid width], [self->uid height], self->mappedLevel );
        
        [self->uid replaceRegion:rgn mipmapLevel:self->mappedLevel slice:self->mappedSlice withBytes:self->mappedData bytesPerRow:stride bytesPerImage:sz];
    }
    else
    {
        [self->uid replaceRegion:rgn mipmapLevel:self->mappedLevel withBytes:self->mappedData bytesPerRow:stride];
    }
}


//------------------------------------------------------------------------------

void
metal_Texture_Update( Handle tex, const void* data, uint32 level, TextureFace face )
{
    TextureMetal_t* self   = TextureMetalPool::Get( tex );
    Size2i          ext    = TextureExtents( Size2i(self->width,self->height), level );
    uint32          sz     = TextureSize( self->format, self->width, self->height, level );
    uint32          stride = TextureStride( self->format, Size2i(self->width,self->height), level );
    MTLRegion       rgn;
    NSUInteger      slice  = 0;
    
    switch( face )
    {
        case TEXTURE_FACE_LEFT   : slice = 0; break;
        case TEXTURE_FACE_RIGHT  : slice = 1; break;
        case TEXTURE_FACE_FRONT  : slice = 2; break;
        case TEXTURE_FACE_BACK   : slice = 3; break;
        case TEXTURE_FACE_TOP    : slice = 4; break;
        case TEXTURE_FACE_BOTTOM : slice = 5; break;
    }
    
    rgn.origin.x    = 0;
    rgn.origin.y    = 0;
    rgn.origin.z    = 0;
    rgn.size.width  = ext.dx;
    rgn.size.height = ext.dy;
    rgn.size.depth  = 1;

    if( self->is_cubemap )
    {
        [self->uid replaceRegion:rgn mipmapLevel:level slice:slice withBytes:data bytesPerRow:stride bytesPerImage:sz];
    }
    else
    {
        [self->uid replaceRegion:rgn mipmapLevel:level withBytes:data bytesPerRow:stride];
    }
}


//------------------------------------------------------------------------------

namespace TextureMetal
{

void
SetupDispatch( Dispatch* dispatch )
{
    dispatch->impl_Texture_Create = &metal_Texture_Create;
    dispatch->impl_Texture_Delete = &metal_Texture_Delete;
    dispatch->impl_Texture_Map    = &metal_Texture_Map;
    dispatch->impl_Texture_Unmap  = &metal_Texture_Unmap;
    dispatch->impl_Texture_Update = &metal_Texture_Update;
}
    
void
SetToRHIFragment( Handle tex, unsigned unitIndex, id<MTLRenderCommandEncoder> ce )
{
    TextureMetal_t* self = TextureMetalPool::Get( tex );

    [ce setFragmentTexture:self->uid atIndex:unitIndex];
}

void
SetToRHIVertex( Handle tex, unsigned unitIndex, id<MTLRenderCommandEncoder> ce )
{
    TextureMetal_t* self = TextureMetalPool::Get( tex );

    [ce setVertexTexture:self->uid atIndex:unitIndex];
}

void
SetAsRenderTarget( Handle tex, MTLRenderPassDescriptor* desc )
{
    TextureMetal_t* self = TextureMetalPool::Get( tex );

    desc.colorAttachments[0].texture = self->uid;
}

} // namespace TextureMetal

//==============================================================================
} // namespace rhi

