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


#ifndef __DAVAENGINE_IREADONLY_CONNECTION_H__
#define __DAVAENGINE_IREADONLY_CONNECTION_H__

#include <exception>
#include <memory>
#include <type_traits>

#include "Base/BaseTypes.h"
#include "Base/RefPtr.h"
#include "Base/TypeHolders.h"
#include "Network/Base/Endpoint.h"

namespace DAVA
{
namespace Net
{

//--------------------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------------------
struct IReadOnlyConnection : public RefCounter
{
    virtual ~IReadOnlyConnection() {}
    
    enum class ChannelState
    {
        Disconnected,
        Connected
    };
    virtual ChannelState GetChannelState() = 0;
    virtual const Endpoint& GetEndpoint() = 0;

    virtual size_t ReadSome(char* buffer, size_t bufSize) = 0;
    virtual bool ReadAll(char* buffer, size_t bufSize) = 0;

    virtual size_t ReadBytesCount() = 0;
    
    template <typename T>
    bool Read(T& value);

    template <typename Container>
    void Read(Container& container, size_t count);
};
using IReadOnlyConnectionPtr = RefPtr<IReadOnlyConnection>;

//--------------------------------------------------------------------------------------------------
//Implementation of template methods
//--------------------------------------------------------------------------------------------------
template <typename T>
bool IReadOnlyConnection::Read(T& value)
{
    return ReadAll(reinterpret_cast<char*>(&value), sizeof(T));
}

template <typename Container>
void IReadOnlyConnection::Read(Container& container, size_t count)
{
    using value_type = typename Container::value_type;
    static_assert(std::is_pod<value_type>::value, 
                  "Container::value_type must be POD-type");

    value_type val;

    for (size_t i = 0; i < count; ++i)
    {
        if (!Read(val))
        {
            throw std::logic_error("IReadOnlyChannel::read: unable to read data");
        }
        container.emplace_back(std::move(val));
    }
}
//--------------------------------------------------------------------------------------------------

}  // namespace Net
}  // namespace DAVA

#endif  // __DAVAENGINE_IREADONLY_CONNECTION_H__