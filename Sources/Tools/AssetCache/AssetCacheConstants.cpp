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

#include "AssetCache/AssetCacheConstants.h"
#include "Base/GlobalEnum.h"
#include "Debug/DVAssert.h"

namespace DAVA
{
namespace AssetCache
{
const String LOCALHOST = "127.0.0.1";

String ErrorToString(Error error)
{
    static const Vector<std::pair<Error, String>> errorStrings = 
    {{
        {Error::NO_ERRORS, "OK"},
        {Error::CODE_NOT_INITIALIZED, "CODE_NOT_INITIALIZED"},
        {Error::WRONG_COMMAND_LINE, "WRONG_COMMAND_LINE"},
        {Error::WRONG_IP, "WRONG_IP"},
        {Error::OPERATION_TIMEOUT, "OPERATION_TIMEOUT"},
        {Error::CANNOT_CONNECT, "CANNOT_CONNECT"},
        {Error::SERVER_ERROR, "SERVER_ERROR"},
        {Error::NOT_FOUND_ON_SERVER, "NOT_FOUND_ON_SERVER"},
        {Error::READ_FILES, "READ_FILES"},
        {Error::ADDRESS_RESOLVER_FAILED, "ADDRESS_RESOLVER_FAILED"},
        {Error::CANNOT_SEND_REQUEST_ADD, "CANNOT_SEND_REQUEST_ADD"},
        {Error::CANNOT_SEND_REQUEST_GET, "CANNOT_SEND_REQUEST_GET"},
        {Error::CORRUPTED_DATA, "CORRUPTED_DATA"}
    }};

    DVASSERT(static_cast<uint32>(Error::ERRORS_COUNT) == errorStrings.size());

    const auto & errorDetails = errorStrings[static_cast<uint32>(error)];

    DVASSERT(errorDetails.first == error);
    return errorDetails.second;
}


} // end of namespace AssetCache
} // end of namespace DAVA

