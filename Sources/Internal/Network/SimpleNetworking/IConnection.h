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


#ifndef __DAVAENGINE_ICONNECTION_H__
#define __DAVAENGINE_ICONNECTION_H__

#include "Network/SimpleNetworking/IReadOnlyConnection.h"

namespace DAVA
{
namespace Net
{

//--------------------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------------------
struct IConnection : public IReadOnlyConnection
{
    virtual size_t Write(const char* buffer, size_t bufSize) = 0;
    
    template <typename T>
    bool Write(const T& value);
    
    template <typename CharT>
    bool Write(const BasicString<CharT>& string);
    
    template <typename T>
    bool Write(const Vector<T>& vect);
};
using IConnectionPtr = std::shared_ptr<IConnection>;

//--------------------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------------------
template <typename T>
inline bool WriteArrayInConnection(IConnection* connection, const T* array, size_t elements)
{
    const char* data = reinterpret_cast<const char*>(array);
    size_t len = elements * sizeof(T);
    size_r wrote = connection->Write(data, len);
    
    return wrote == len;
}

template <typename T>
bool IConnection::Write(const T& value)
{
    return WriteArrayInConnection(this, &value, 1);
}

template <typename CharT>
bool IConnection::Write(const BasicString<CharT>& string)
{
    //write string with null symbol
    return WriteArrayInConnection(this, string.c_str(), string.size() + 1);
}
    
template <typename T>
bool IConnection::Write(const Vector<T>& vect)
{
    static_assert(std::is_pod<T>::value, 
                  "Vector::value_type must be POD-type");
                  
    return WriteArrayInConnection(this, vect.data(), vect.size());
}

}  // namespace Net
}  // namespace DAVA

#endif  // __DAVAENGINE_ICONNECTION_H__