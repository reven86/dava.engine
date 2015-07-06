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

#include "DAVAEngine.h"
#include "UnitTests/UnitTests.h"

using namespace DAVA;

namespace
{
    const uint32 bufSize{ 4100 }; // more then 4096
    const String messageEnd{ "test" };
    const WideString wideMessageEnd{ L"TEST" };

    String errorMessage;
}

class TestLoggerOutput : public LoggerOutput
{
public:
    TestLoggerOutput() = default;

    void Output(Logger::eLogLevel ll, const char8* text) override
    {
        std::ostringstream ostr;
        String msgFromLogger{text};

        if (currentMessageRawSize + 1 != msgFromLogger.length())
        {
            ostr << "size of buffer do not match! bufSize == " << bufSize
                << " msgFromLogger.length == " << msgFromLogger.length() << "\n";
        }
        const String lastNChars = msgFromLogger.substr(
            msgFromLogger.size() - (messageEnd.size() + 1), messageEnd.size());

        if (lastNChars != messageEnd)
        {
            ostr << messageEnd + " != " + lastNChars + "\n";
        }

        if (msgFromLogger.back() != '\n')
        {
            ostr << "last char should always be \\n\n";
        }

        errorMessage += ostr.str();
    }

    size_t currentMessageRawSize = 0;
};

DAVA_TESTCLASS(UnlimitedLogOutputTest)
{
    DAVA_TEST(TestFunction)
    {
        TestLoggerOutput testOutput;
        Logger::AddCustomOutput(&testOutput);

        for (auto bufSizeLocal : {10, static_cast<int32>(bufSize), 4095, 4096, 4097})
        {
            String str(bufSizeLocal, 'a');
            size_t startIndex = bufSizeLocal - messageEnd.size();

            for (auto c : messageEnd)
            {
                str[startIndex++] = c;
            }

            testOutput.currentMessageRawSize = bufSizeLocal;

            Logger::Instance()->Info("%s", str.c_str());
        }
        Logger::RemoveCustomOutput(&testOutput);

        if (!errorMessage.empty())
        {
            Logger::Error("Error: %s", errorMessage.c_str());
        }

        TEST_VERIFY(errorMessage.empty());
    }
};
