
    #include "rhi_RingBufferMetal.h"

    #include "_metal.h"

namespace rhi
{

//------------------------------------------------------------------------------

void
RingBufferMetal::Initialize( unsigned sz )
{
    uid = [_Metal_Device newBufferWithLength:sz options:MTLResourceOptionCPUCacheModeDefault];

    buf.Initialize( uid.contents, sz );
}


//------------------------------------------------------------------------------

void
RingBufferMetal::Uninitialize()
{
}


//------------------------------------------------------------------------------

float*
RingBufferMetal::Alloc( unsigned cnt, unsigned* offset )
{
    float*      ptr = buf.Alloc( cnt, 256 ); // since MTL-buf offset must be aligned to 256
    unsigned    off = (uint8*)ptr - (uint8*)(uid.contents);

    if( offset )
        *offset = off;

    return ptr;
}


//------------------------------------------------------------------------------

id<MTLBuffer>
RingBufferMetal::BufferUID() const
{
    return uid;
}


//------------------------------------------------------------------------------

unsigned
RingBufferMetal::Offset( void* ptr ) const
{
    return (uint8*)ptr - (uint8*)(uid.contents);
}




} // namespace rhi 