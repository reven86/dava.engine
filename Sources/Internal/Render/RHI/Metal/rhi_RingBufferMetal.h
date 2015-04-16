#if !defined __RHI_RINGBUFFERMETAL_H__
#define __RHI_RINGBUFFERMETAL_H__

    #include "../Common/rhi_RingBuffer.h"
    #include "_metal.h"

namespace rhi
{

class
RingBufferMetal
{
public:

    void            Initialize( unsigned sz );
    void            Uninitialize();

    float*          Alloc( unsigned cnt, unsigned* offset=0 );

    id<MTLBuffer>   BufferUID() const;
    unsigned        Offset( void* ptr ) const;  


private:

    RingBuffer      buf;
    __unsafe_unretained id<MTLBuffer>   uid;
};


} // namespace rhi

#endif // __RHI_RINGBUFFERMETAL_H__
