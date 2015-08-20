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


#include "Debug/DVAssert.h"
#include "FileSystem/File.h"
#include "Utils/StringFormat.h"

#include "Network/Base/Endpoint.h"

#include "LogConsumer.h"

namespace DAVA
{
namespace Net
{

LogConsumer::LogConsumer(const Options& options) 
    : writeToConsole(options.writeToConsole)
    , rawOutput(options.rawOutput)
{
    if (options.filename.IsSet())
    {
        FilePath path = options.filename.Get();
        file = RefPtr<File>(File::Create(path, File::WRITE | File::APPEND));
    }
}

LogConsumer::~LogConsumer()
{
}

void LogConsumer::OnPacketReceived(IChannel* channel, const void* buffer, size_t length)
{
    String output(static_cast<const char8*>(buffer), length);

    if (!rawOutput)
    {
        String endp(channel->RemoteEndpoint().ToString());
        output = Format("[%s] %s", endp.c_str(), output.c_str());
    }

    if (file)
    {
        file->WriteLine(output);
    }

    if (writeToConsole)
    {
        printf("%s\n", output.c_str());
    }
}

}   // namespace Net
}   // namespace DAVA
