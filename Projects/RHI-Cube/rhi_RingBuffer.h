/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#ifndef __RHI_RINGBUFFER_H__
#define __RHI_RINGBUFFER_H__

    #include "rhi_Type.h"


namespace rhi
{

class
RingBuffer
{
public:
                RingBuffer();

    void        initialize( unsigned sz );
    void        uninitialize();

    float*      alloc( unsigned cnt );
    void        reset();
    

private:

    unsigned    size;
    uint8*      data_ptr;
    uint8*      cur;

    unsigned    mem_used;
    unsigned    alloc_count;
};


//------------------------------------------------------------------------------

RingBuffer::RingBuffer()
  : size(0),
    data_ptr(0),
    cur(0),
    mem_used(0)
{
}


//------------------------------------------------------------------------------

void
RingBuffer::initialize( unsigned sz )
{
    size     = sz;
    data_ptr = (uint8*)malloc( sz );    

    cur         = data_ptr;
    mem_used    = 0;
    alloc_count = 0;
}


//------------------------------------------------------------------------------

void        
RingBuffer::uninitialize()
{
    if( data_ptr )
        free( data_ptr );

    size        = 0;
    data_ptr    = 0;
    cur         = 0;
}


//------------------------------------------------------------------------------

float*      
RingBuffer::alloc( unsigned cnt )
{
    DVASSERT(cur);

    unsigned    sz  = L_ALIGNED_SIZE(cnt*sizeof(float),16);
    uint8*      buf = cur + sz;
    uint8*      p   = cur;

    if( buf > data_ptr + size )
    {
        buf = data_ptr + sz;
        p   = data_ptr;
    }

    cur          = buf;
    mem_used    += sz;
    ++alloc_count;

    return (float*)p;
}


//------------------------------------------------------------------------------

void
RingBuffer::reset()
{
    mem_used    = 0;
    alloc_count = 0;
}


} // namespace rhi
#endif // __RHI_RINGBUFFER_H__
