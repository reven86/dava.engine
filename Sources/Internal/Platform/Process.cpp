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


#include "Platform/Process.h"
#include "FileSystem/FilePath.h"

#if defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_MACOS__)

#if defined(__DAVAENGINE_MACOS__)
#include <sys/wait.h>
#endif

static const int READ = 0;
static const int WRITE = 1;
static const int BUF_SIZE = 512;

namespace DAVA
{
Process::Process(const FilePath& path, const Vector<String>& args)
{
    pid = -1; //invalid pid
    output = "[Process::Process] The program has not been started yet!";
    executablePath = path;
    runArgs = args;
    running = false;
    exitCode = -1; //invalid

#if defined (__DAVAENGINE_WINDOWS__)
    childProcIn[0] = childProcIn[1] = 0;
    childProcOut[0] = childProcOut[1] = 0;

#else
    pipes[0] = pipes[1] = -1;
#endif
}

Process::~Process()
{
    CleanupHandles();
}

const String& Process::GetOutput() const
{
    return output;
}

int64 Process::GetPID() const
{
    return pid;
}

const FilePath& Process::GetPath() const
{
    return executablePath;
}

const Vector<String>& Process::GetArgs() const
{
    return runArgs;
}

int Process::GetExitCode() const
{
    return exitCode;
}

#if defined (__DAVAENGINE_WIN32__)

void Process::CleanupHandles()
{
    for (int i = 0; i < 2; ++i)
    {
        if (childProcIn[i])
        {
            ::CloseHandle(childProcIn[i]);
        }

        if (childProcOut[i])
        {
            ::CloseHandle(childProcOut[i]);
        }
    }

    childProcIn[0] = childProcIn[1] = 0;
    childProcOut[0] = childProcOut[1] = 0;

    if (pid != -1)
    {
        ::CloseHandle((HANDLE)pid);
        pid = -1;
    }
}

bool Process::Run(bool showWindow)
{
    //see http://msdn.microsoft.com/en-us/library/ms682499%28v=vs.85%29.aspx

    DVASSERT(!running);

    if (running)
        return false;

    bool result = showWindow;

    CleanupHandles();

    if (!showWindow)
    {
        SECURITY_ATTRIBUTES saAttr;
        saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
        saAttr.bInheritHandle = TRUE;
        saAttr.lpSecurityDescriptor = NULL;

        if (::CreatePipe(&childProcOut[READ], &childProcOut[WRITE], &saAttr, 0))
        {
            if (::CreatePipe(&childProcIn[READ], &childProcIn[WRITE], &saAttr, 0))
            {
                //::SetHandleInformation(childProcOut[WRITE], HANDLE_FLAG_INHERIT, 0);
                ::SetHandleInformation(childProcOut[READ], HANDLE_FLAG_INHERIT, 0);
                ::SetHandleInformation(childProcIn[WRITE], HANDLE_FLAG_INHERIT, 0);
                //::SetHandleInformation(childProcIn[READ], HANDLE_FLAG_INHERIT, 0);

                result = true;
            }
        }
    }

    if (result)
    {
        PROCESS_INFORMATION piProcInfo;
        STARTUPINFO siStartInfo;
        BOOL bSuccess = FALSE;

        // Set up members of the PROCESS_INFORMATION structure.

        ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

        // Set up members of the STARTUPINFO structure.
        // This structure specifies the STDIN and STDOUT handles for redirection.

        ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
        siStartInfo.cb = sizeof(STARTUPINFO);

        if (!showWindow)
        {
            siStartInfo.hStdError = childProcOut[WRITE];
            siStartInfo.hStdOutput = childProcOut[WRITE];
            siStartInfo.hStdInput = childProcIn[READ];
            siStartInfo.dwFlags |= STARTF_USESTDHANDLES;
            siStartInfo.dwFlags |= STARTF_USESHOWWINDOW;
            siStartInfo.wShowWindow = SW_HIDE;
        }

        // Create the child process.

        String runArgsFlat = "cmd.exe /c ";
        runArgsFlat += executablePath.GetAbsolutePathname();
        if (runArgs.size() > 0)
        {
            for (int i = 0; i < (int)runArgs.size(); ++i)
            {
                runArgsFlat += " ";
                runArgsFlat += runArgs[i];
            }
        }

#if defined(UNICODE)

        wchar_t* execPathW = NULL;
        size_t execPathWLength = 0;
        wchar_t* execArgsW = NULL;
        size_t execArgsWLength = 0;

        //VI: TODO: UNICODE: Use framework methods to convert to Unicode once it will be ready.
        ConvertToWideChar(runArgsFlat, &execArgsW, &execArgsWLength);

        bSuccess = CreateProcess(NULL,
                                 execArgsW, // command line
                                 NULL, // process security attributes
                                 NULL, // primary thread security attributes
                                 TRUE, // handles are inherited
                                 (showWindow) ? 0 : CREATE_NO_WINDOW, // creation flags
                                 NULL, // use parent's environment
                                 NULL, // use parent's current directory
                                 &siStartInfo, // STARTUPINFO pointer
                                 &piProcInfo); // receives PROCESS_INFORMATION

        SafeDeleteArray(execArgsW);

#else
        bSuccess = CreateProcess(NULL,
                                 runArgsFlat.c_str(), // command line
                                 NULL, // process security attributes
                                 NULL, // primary thread security attributes
                                 TRUE, // handles are inherited
                                 (showWindow) ? 0 : CREATE_NO_WINDOW, , // creation flags
                                 NULL, // use parent's environment
                                 NULL, // use parent's current directory
                                 &siStartInfo, // STARTUPINFO pointer
                                 &piProcInfo); // receives PROCESS_INFORMATION

#endif
        result = (TRUE == bSuccess);

        if (result)
        {
            pid = (int64)piProcInfo.hProcess;

            ::CloseHandle(childProcOut[WRITE]);
            childProcOut[WRITE] = 0;
        }
    }

    if (!result)
    {
        CleanupHandles();
    }

    running = result;
    return result;
}

void Process::Wait()
{
    DVASSERT(running);
    DVASSERT(pid != -1);

    if (!running || pid == -1)
        return;
    running = false;

    if (childProcOut[READ])
    {
        output = "";
        CHAR readBuf[BUF_SIZE];
        DWORD bytesRead = 0;
        BOOL readResult = FALSE;

        readResult = ReadFile(childProcOut[READ], readBuf, BUF_SIZE, &bytesRead, NULL);
        while (bytesRead > 0 && readResult != FALSE)
        {
            output.append(readBuf, bytesRead);
            readResult = ReadFile(childProcOut[READ], readBuf, BUF_SIZE, &bytesRead, NULL);
        }
    }

    ::WaitForSingleObject((HANDLE)pid, INFINITE);

    DWORD code;
    BOOL res = ::GetExitCodeProcess((HANDLE)pid, &code);
    exitCode = static_cast<int>(code);
    if (res == FALSE)
    {
        exitCode = -1;
        Logger::Error("[Process::Wait] Can't get exit code for process %s, error %d", executablePath.GetAbsolutePathname().c_str(), ::GetLastError());
    }

    CleanupHandles();
}

#if defined(UNICODE)

void Process::ConvertToWideChar(const String& str, wchar_t** outStr, size_t* outLength)
{
    *outStr = NULL;
    *outLength = 0;

    *outLength = mbstowcs(NULL, str.c_str(), str.size()) + 1;

    if (*outLength > 0)
    {
        *outStr = new wchar_t[*outLength];
        memset(*outStr, 0, sizeof(wchar_t) * (*outLength));
        mbstowcs(*outStr, str.c_str(), str.size());
    }
}

#endif

#else

bool Process::Run(bool showWindow)
{
    DVASSERT(!running);

    if (running)
        return false;

    running = false;
    bool result = false;

    if (pipe(pipes) != 0)
    {
        return result;
    }

    String execPath = executablePath.GetAbsolutePathname();
    Vector<char*> execArgs;

    execArgs.push_back(&execPath[0]);

    for (Vector<String>::iterator it = runArgs.begin();
         it != runArgs.end();
         ++it)
    {
        execArgs.push_back(&(*it)[0]);
    }

    execArgs.push_back(NULL);

    pid = fork();

    switch (pid)
    {
    case 0: //child process
        {
            close(STDERR_FILENO);
            close(STDOUT_FILENO);

            dup2(pipes[WRITE], STDERR_FILENO);
            dup2(pipes[WRITE], STDOUT_FILENO);

            close(pipes[READ]);
            pipes[READ] = -1;

            int execResult = execv(execPath.c_str(), &execArgs[0]);
            DVASSERT(execResult >= 0);
            _exit(0); //if we got here - there's a problem
            break;
        }

        case -1: //error
        {
            result = false;
            Logger::Error("[Process::Run] Failed to start process %s", executablePath.GetAbsolutePathname().c_str());
            break;
        }

        default: //parent process
        {
            close(pipes[WRITE]);
            pipes[WRITE] = -1;

            running = true;
            result = true;
        }
        };

        return result;
}

void Process::Wait()
{
    DVASSERT(running);
    DVASSERT(pid != -1);

    if (!running || pid == -1)
        return;
    running = false;

    int status = 0;
    //    waitpid(pid, &status, 0); // it is not working on OSX
    int64 pd = -1;
    do
    {
        status = 0;
        pd = wait(&status);
    } while (pd != pid);

    exitCode = WEXITSTATUS(status);
    if (WIFEXITED(status) == 0)
    {
        if (exitCode == 0)
        {
            exitCode = -1; //to say external code about problems
        }
        Logger::Error("[Process::Wait] The process %s exited abnormally! (%d)", executablePath.GetAbsolutePathname().c_str(), exitCode);
    }

    output = "";

    char readBuf[BUF_SIZE];
    int bytesRead = read(pipes[READ], readBuf, BUF_SIZE);
    while (bytesRead > 0)
    {
        output.append(readBuf, bytesRead);

        bytesRead = read(pipes[READ], readBuf, BUF_SIZE);
    }
    //}

    CleanupHandles();
}

void Process::CleanupHandles()
{
    for (int i = 0; i < 2; ++i)
    {
        if (pipes[i] != -1)
        {
            close(pipes[i]);
            pipes[i] = -1;
        }
    }
}

#endif
	
};

#endif
