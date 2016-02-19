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


#include "FileSystem/Logger.h"
#include "FileSystem/FileSystem.h"
#include "Debug/DVAssert.h"
#include <cstdarg>
#include <array>

#include "Utils/Utils.h"

namespace DAVA
{

#if defined(__DAVAENGINE_WINDOWS__)

#define vsnprintf _vsnprintf
#define snprintf _snprintf

#endif

namespace
{
const size_t defaultBufferSize{ 4096 };
}

String ConvertCFormatListToString(const char8* format, va_list pargs)
{
    String dynamicbuf;
    dynamicbuf.resize(defaultBufferSize * 2);

    while (true)
    {
        va_list copy;
        va_copy(copy, pargs);
        int32 charactersWritten = vsnprintf(&dynamicbuf[0], dynamicbuf.size(), format, copy);
        va_end(copy);
        // NB. C99 (which modern Linux and OS X follow) says vsnprintf
        // failure returns the length it would have needed.  But older
        // glibc and current Windows return -1 for failure, i.e., not
        // telling us how much was needed.
        if (charactersWritten < static_cast<int32>(dynamicbuf.size()) && charactersWritten >= 0)
        {
            dynamicbuf.resize(charactersWritten);
            return dynamicbuf;
        }
        // do you really want to print 1Mb with one call may be your format
        // string incorrect?
        DVASSERT_MSG(dynamicbuf.size() < 1024 * 1024,
                     DAVA::Format("format: {%s}", format).c_str());

        dynamicbuf.resize(dynamicbuf.size() * 2);
    }
    DVASSERT(false);
    return String("never happen! ");
}

void Logger::Logv(eLogLevel ll, const char8* text, va_list li) const
{
    if (!text || text[0] == '\0')
        return;

    // try use stack first
    Array<char8, defaultBufferSize> stackbuf;

    va_list copy;
    va_copy(copy, li);
    int32 charactersWritten = vsnprintf(&stackbuf[0], defaultBufferSize - 1, text, copy);
    va_end(copy);

    if (charactersWritten < 0 || charactersWritten >= static_cast<int32>(defaultBufferSize - 1))
    {
        String formatedMessage = ConvertCFormatListToString(text, li);
        formatedMessage += '\n';

        Output(ll, formatedMessage.c_str());
    }
    else
    {
        stackbuf[charactersWritten] = '\n';
        stackbuf[charactersWritten + 1] = '\0';

        Output(ll, &stackbuf[0]);
    }
}

static const Array<const char8*, 5> logLevelString
{
  {
  "framwork",
  "debug",
  "info",
  "warning",
  "error"
  }
};

Logger::Logger()
    :
    logLevel{ LEVEL_FRAMEWORK }
    ,
    consoleModeEnabled{ false }
{
    SetLogFilename(String());
}

Logger::~Logger()
{
    for (auto logOutput : customOutputs)
    {
        delete logOutput;
    }
}

Logger::eLogLevel Logger::GetLogLevel() const
{
    return logLevel;
}

const char8* Logger::GetLogLevelString(eLogLevel ll) const
{
#ifndef __DAVAENGINE_WINDOWS__
    static_assert(logLevelString.size() == LEVEL__DISABLE,
                  "please update strings values");
#endif
    return logLevelString[ll];
}

Logger::eLogLevel Logger::GetLogLevelFromString(const char8* ll) const
{
    for (size_t i = 0; i < logLevelString.size(); ++i)
    {
        if (strcmp(ll, logLevelString[i]) == 0)
        {
            return static_cast<eLogLevel>(i);
        }
    }
    return LEVEL__DISABLE;
}

void Logger::SetLogLevel(eLogLevel ll)
{
    logLevel = ll;
}

void Logger::Log(eLogLevel ll, const char8* text, ...) const
{
    if (ll < logLevel)
        return;

    va_list vl;
    va_start(vl, text);
    Logv(ll, text, vl);
    va_end(vl);
}

void Logger::FrameworkDebug(const char8* text, ...)
{
    va_list vl;
    va_start(vl, text);
    Logger* log = Logger::Instance();
    if (nullptr != log)
        log->Logv(LEVEL_FRAMEWORK, text, vl);
    va_end(vl);
}

void Logger::Debug(const char8* text, ...)
{
    va_list vl;
    va_start(vl, text);
    Logger* log = Logger::Instance();
    if (nullptr != log)
        log->Logv(LEVEL_DEBUG, text, vl);
    va_end(vl);
}

void Logger::Info(const char8* text, ...)
{
    va_list vl;
    va_start(vl, text);
    Logger* log = Logger::Instance();
    if (nullptr != log)
        log->Logv(LEVEL_INFO, text, vl);
    va_end(vl);
}

void Logger::Warning(const char8* text, ...)
{
    va_list vl;
    va_start(vl, text);
    Logger* log = Logger::Instance();
    if (nullptr != log)
        log->Logv(LEVEL_WARNING, text, vl);
    va_end(vl);
}

void Logger::Error(const char8* text, ...)
{
    va_list vl;
    va_start(vl, text);
    Logger* log = Logger::Instance();
    if (nullptr != log)
        log->Logv(LEVEL_ERROR, text, vl);
    va_end(vl);
}

void Logger::AddCustomOutput(DAVA::LoggerOutput* lo)
{
    Logger* log = Logger::Instance();
    if (nullptr != log && nullptr != lo)
        log->customOutputs.push_back(lo);
}

void Logger::RemoveCustomOutput(DAVA::LoggerOutput* lo)
{
    Logger* log = Logger::Instance();
    if (nullptr != log && nullptr != lo)
    {
        auto& outputs = log->customOutputs;

        outputs.erase(std::remove(outputs.begin(), outputs.end(), lo));
    }
}

void Logger::SetLogFilename(const String& filename)
{
    if (filename.empty())
    {
        logFilename = FilePath();
    }
    else
    {
        SetLogPathname(GetLogPathForFilename(filename));
    }
}

void Logger::SetLogPathname(const FilePath& filepath)
{
    const bool canWorkWithFile = CutOldLogFileIfExist(filepath);
    DVASSERT(canWorkWithFile);

    logFilename = filepath;
}

FilePath Logger::GetLogPathForFilename(const String& filename)
{
    FilePath logFilePath(FileSystem::Instance()->GetCurrentDocumentsDirectory() + filename);
    return logFilePath;
}

void Logger::SetMaxFileSize(uint32 size)
{
    cutLogSize = size;
}

void Logger::ResetMaxFileSize()
{
    cutLogSize = defaultCutLogSize;
}

bool Logger::CutOldLogFileIfExist(const FilePath& logFile)
{
    if (!logFile.Exists())
    {
        return true; // ok. No file - no questions;
    }

    // take the tail of the log file and put it to the start of the file. Cut file to size of taken tail.

    File* log = File::Create(logFile, File::OPEN | File::READ | File::WRITE);
    if (nullptr == log)
    {
        return false; // cannot open file;
    }

    SCOPE_EXIT
    {
        SafeRelease(log);
    };

    static const uint32 sizeToCut = cutLogSize;

    const uint32 fileSize = log->GetSize();
    if (sizeToCut >= fileSize)
    {
        return true; // ok! Have less data than we should to cut.
    }

    Vector<uint8> buff(sizeToCut);
    const bool seekSuccess = log->Seek(static_cast<int32>(-sizeToCut), File::SEEK_FROM_END);
    if (!seekSuccess)
    {
        return false; // have enought data but seek error
    }

    uint32 dataReaden = log->Read(buff.data(), sizeToCut);
    if (dataReaden != sizeToCut)
    {
        return false; // nave enought data but cannot to read
    }

    SafeRelease(log);

    File* truncatedLog = File::Create(logFile, File::CREATE | File::WRITE);
    if (nullptr == truncatedLog)
    {
        return false;
    }

    SCOPE_EXIT
    {
        SafeRelease(truncatedLog);
    };

    const uint32 dataWritten = truncatedLog->Write(buff.data(), sizeToCut);
    if (dataWritten != sizeToCut)
    {
        return false; // have correct file and data size but cannot write to file.
    }

    return true; // correct;
}

void Logger::FileLog(eLogLevel ll, const char8* text) const
{
    if (nullptr != FileSystem::Instance())
    {
        File* file = File::Create(logFilename, File::APPEND | File::WRITE);
        if (nullptr != file)
        {
            Array<char8, 128> prefix;


#if defined(__DAVAENGINE_WIN_UAP__)
            SYSTEMTIME st;
            GetSystemTime(&st);
            // then convert st to your precision needs
            snprintf(&prefix[0], prefix.size(), "- %d:%d:%d %d", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
            file->Write(prefix.data(), static_cast<uint32>(strlen(prefix.data())));
#endif
            snprintf(&prefix[0], prefix.size(), "[%s] ", GetLogLevelString(ll));
            file->Write(prefix.data(), static_cast<uint32>(strlen(prefix.data())));
            file->Write(text, static_cast<uint32>(strlen(text)));
            file->Release();
        }
    }
}

void Logger::CustomLog(eLogLevel ll, const char8* text) const
{
    for (auto output : customOutputs)
    {
        output->Output(ll, text);
    }
}

void Logger::EnableConsoleMode()
{
    consoleModeEnabled = true;
}

void Logger::ConsoleLog(DAVA::Logger::eLogLevel ll, const char8* text) const
{
    printf("[%s] %s", GetLogLevelString(ll), text);
}

void Logger::Output(eLogLevel ll, const char8* formatedMsg) const
{
    CustomLog(ll, formatedMsg);
    // print platform log or write log to file
    // only if log level is acceptable
    if (ll >= logLevel)
    {
        if (consoleModeEnabled)
        {
            ConsoleLog(ll, formatedMsg);
        }
        else
        {
            PlatformLog(ll, formatedMsg);
        }

        if (!logFilename.IsEmpty())
        {
            FileLog(ll, formatedMsg);
        }
    }
}

} // end namespace DAVA
