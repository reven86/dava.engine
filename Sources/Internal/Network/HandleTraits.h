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

#ifndef __DAVAENGINE_HANDLETRAITS_H__
#define __DAVAENGINE_HANDLETRAITS_H__

#include <type_traits>

#include <libuv/uv.h>

namespace DAVA
{

/*
 IsHandleType<T> checks whether type T is supported libuv's handle type
*/
template<typename T>
struct IsHandleType : public std::false_type {};

template<> struct IsHandleType<uv_tcp_t>    : public std::true_type {};
template<> struct IsHandleType<uv_udp_t>    : public std::true_type {};
template<> struct IsHandleType<uv_timer_t>  : public std::true_type {};
template<> struct IsHandleType<uv_async_t>  : public std::true_type {};

/*
 IsHandleConvertible<To,From> checks whether pointer of type From can be converted
 to pointer of type To, where From and To should be libuv handle types
*/
template<typename To, typename From>
struct IsHandleConvertible : public std::false_type {};

template<> struct IsHandleConvertible<uv_handle_t, uv_tcp_t>    : public std::true_type {};
template<> struct IsHandleConvertible<uv_stream_t, uv_tcp_t>    : public std::true_type {};
template<> struct IsHandleConvertible<uv_handle_t, uv_stream_t> : public std::true_type {};
template<> struct IsHandleConvertible<uv_handle_t, uv_udp_t>    : public std::true_type {};
template<> struct IsHandleConvertible<uv_handle_t, uv_timer_t > : public std::true_type {};
template<> struct IsHandleConvertible<uv_handle_t, uv_async_t > : public std::true_type {};

// Explicit const specializations
// I hope I will not need volatile and const volatile specializations
template<> struct IsHandleConvertible<const uv_handle_t, const uv_tcp_t>    : public std::true_type {};
template<> struct IsHandleConvertible<const uv_stream_t, const uv_tcp_t>    : public std::true_type {};
template<> struct IsHandleConvertible<const uv_handle_t, const uv_stream_t> : public std::true_type {};
template<> struct IsHandleConvertible<const uv_handle_t, const uv_udp_t>    : public std::true_type {};
template<> struct IsHandleConvertible<const uv_handle_t, const uv_timer_t > : public std::true_type {};
template<> struct IsHandleConvertible<const uv_handle_t, const uv_async_t > : public std::true_type {};

/*
 Function CastHandleTo<To, From> performs allowable casting from pointer to type From to pointer to type To.
 If casting is not allowed then compiler would complain.
*/
template<typename To, typename From>
typename std::enable_if<std::is_same<To, From>::value || IsHandleConvertible<To, From>::value, To>::type* CastHandleTo(From* ptr)
{
    return reinterpret_cast<To*>(ptr);
}

// CastHandleTo<To, From> - alternative implementation
//template<typename To, typename From>
//To* CastHandleTo(From* ptr)
//{
//    static_assert(std::is_same<To, From>::value || IsConvertible<To, From>::value, "Handle types are not compatible");
//    return reinterpret_cast<To*>(ptr);
//}

}   // namespace DAVA

#endif  // __DAVAENGINE_HANDLETRAITS_H__
