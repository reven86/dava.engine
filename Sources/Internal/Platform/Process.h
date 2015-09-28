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


#ifndef __DAVAENGINE_PROCESS_H__
#define __DAVAENGINE_PROCESS_H__

#include "Base/BaseTypes.h"
#include "Base/Message.h"
#include "Base/BaseObject.h"

#if defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_MACOS__)

namespace DAVA
{
class Process
{
public:
    Process(const FilePath& path, const Vector<String>& args);
    ~Process();

    bool Run(bool showWindow);
    void Wait();

    const String& GetOutput() const;
    int64 GetPID() const;
    const FilePath& GetPath() const;
    const Vector<String>& GetArgs() const;

    int GetExitCode() const;

private:
    void CleanupHandles();

#if defined (__DAVAENGINE_WIN32__) && defined(UNICODE)

    void ConvertToWideChar(const String& str, wchar_t** outStr, size_t* outLength);

#endif

private:
    int64 pid = -1;
    String output;
    FilePath executablePath;
    Vector<String> runArgs;
    bool running = false;

#if defined (__DAVAENGINE_WIN32__)
    HANDLE childProcIn[2];
    HANDLE childProcOut[2];
#else
    int pipes[2];
#endif

    int exitCode = -1; //invalid by default
};
};

#endif

#endif
