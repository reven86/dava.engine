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

#include "Infrastructure/TextureUtils.h"

using namespace DAVA;

DAVA_TESTCLASS (LoggerFileTest)
{
    DAVA_TEST (TestFunction)
    {
        const uint32 logCutSize = 80;
        const String filename("TestLogFile.txt");
        const FilePath logFilePath(Logger::GetLogPathForFilename(filename));

        {
            ScopedPtr<File> log(File::Create(logFilePath, File::CREATE | File::WRITE));
        }

        Logger::Instance()->SetMaxFileSize(logCutSize);

        for (uint32 i = 0; i < 10; ++i)
        {
            if (i > 0)
            {
                ScopedPtr<File> log(File::Create(logFilePath, File::OPEN | File::READ));
                TEST_VERIFY(log);
                uint32 size = log->GetSize();
                // log could have any size from last session
                TEST_VERIFY(logCutSize < size);
            }

            // choult to cut file to logCutSize
            Logger::Instance()->SetLogFilename(filename);

            {
                ScopedPtr<File> log(File::Create(logFilePath, File::OPEN | File::READ));
                TEST_VERIFY(log);
                uint32 size = log->GetSize();
                // current session should start from last logCutSize bytes of prev session.
                TEST_VERIFY(logCutSize >= size);
            }

            // fill log
            Logger::Debug(Format("%d ==", i).c_str());
            for (uint32 j = 0; j < logCutSize; ++j)
            {
                Logger::Debug(Format("%d++", j).c_str());
            }
        }
    }
}
;
