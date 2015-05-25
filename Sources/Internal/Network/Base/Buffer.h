/*==================================================================================
    Copyright(c) 2008, binaryzebra
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

#ifndef __DAVAENGINE_BUFFER_H__
#define __DAVAENGINE_BUFFER_H__

#include "Base/BaseTypes.h"
#include <libuv/uv.h>

namespace DAVA
{
namespace Net
{

typedef uv_buf_t Buffer;

template<typename T>
Buffer CreateBuffer(T* buffer, std::size_t count = 1)
{
#if defined(__DAVAENGINE_WIN_UAP__)
    __DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__
    return Buffer();
#else
    return uv_buf_init(static_cast<char8*>(static_cast<void*>(buffer)), static_cast<uint32>(sizeof(T) * count));
#endif
}

inline Buffer CreateBuffer(void* rawBuffer, std::size_t size)
{
#if defined(__DAVAENGINE_WIN_UAP__)
    __DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__
    return Buffer();
#else
    return uv_buf_init(static_cast<char8*>(rawBuffer), static_cast<uint32>(size));
#endif
}

/*
 Overloads that take pointer to const buffer
*/
template<typename T>
Buffer CreateBuffer(const T* buffer, std::size_t count = 1)
{
#if defined(__DAVAENGINE_WIN_UAP__)
    __DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__
        return Buffer();
#else
    return uv_buf_init(static_cast<char8*>(static_cast<void*>(const_cast<T*>(buffer))), static_cast<uint32>(sizeof(T) * count));
#endif
}

inline Buffer CreateBuffer(const void* rawBuffer, std::size_t size)
{
#if defined(__DAVAENGINE_WIN_UAP__)
    __DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__
    return Buffer();
#else
    return uv_buf_init(static_cast<char8*>(const_cast<void*>(rawBuffer)), static_cast<uint32>(size));
#endif
}

}   // namespace Net
}   // namespace DAVA

#endif  // __DAVAENGINE_BUFFER_H__
