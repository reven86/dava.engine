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

    #include "rhi_RingBufferMetal.h"

    #include "_metal.h"

namespace rhi
{

//------------------------------------------------------------------------------

void
RingBufferMetal::Initialize( unsigned sz )
{
    uid = [_Metal_Device newBufferWithLength:sz options:MTLResourceOptionCPUCacheModeDefault];
//    uid = [_Metal_Device newBufferWithLength:sz options:MTLCPUCacheModeWriteCombined];

    buf.Initialize( uid.contents, uid.length );
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