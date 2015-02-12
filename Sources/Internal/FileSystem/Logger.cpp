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

#if defined(__DAVAENGINE_WIN32__)

#define vsnprintf _vsnprintf
#define vswprintf _vsnwprintf
#define snprintf _snprintf

#endif


String ConvertCFormatListToString(const char8* format, va_list pargs)
{
    // Allocate a buffer on the stack that's big enough for us almost
    // all the time.  Be prepared to allocate dynamically if it doesn't fit.
    String dynamicbuf;
    dynamicbuf.resize(4096 * 2);

    while (true)
    {
        va_list copy;
        va_copy(copy, pargs);
        int needed = vsnprintf(&dynamicbuf[0], dynamicbuf.size(), format, copy);
        va_end(copy);
        // NB. C99 (which modern Linux and OS X follow) says vsnprintf
        // failure returns the length it would have needed.  But older
        // glibc and current Windows return -1 for failure, i.e., not
        // telling us how much was needed.
        if (needed < static_cast<int>(dynamicbuf.size()) && needed >= 0)
        {
            // It fit fine so we're done.
            return dynamicbuf;
        }
        // you you want to print 1Mb with one call may be you format
        // string incorrect?
        DVASSERT(dynamicbuf.size() < 1024 * 1024);
        // vsnprintf reported that it wanted to write more characters
        // than we allocated.  So try again using a dynamic buffer.  This
        // doesn't happen very often if we chose our initial size well.
        dynamicbuf.resize(dynamicbuf.size() * 2);
    }
    DVASSERT(false);
    return String("never happen! ");
}

void Logger::Logv(eLogLevel ll, const char8* text, va_list li)
{
    if (!text || text[0] == '\0') return;

    // try use stack first
    const size_t size = 4096;
    std::array<char8, size> stackbuf;

    va_list copy;
    va_copy(copy, li);
    int needMoreBuff = vsnprintf(&stackbuf[0], size, text, copy);
    va_end(copy);

    if (needMoreBuff < 0 || needMoreBuff > static_cast<int>(size))
    {
        String formatedMessage = ConvertCFormatListToString(text, li);
        // always send log to custom subscribers
        Output(ll, formatedMessage.c_str());
    }
    else
    {
        Output(ll, &stackbuf[0]);
    }
}

static const char8 * logLevelString[5] =
{	
	"framwork",
	"debug",
	"info",
	"warning",
	"error" 
};
	
Logger::Logger()
{
	logLevel = LEVEL_FRAMEWORK;
	SetLogFilename(String());
    
    consoleModeEnabled = false;
}

Logger::~Logger()
{
	for(size_t i = 0; i < customOutputs.size(); ++i)
	{
		delete customOutputs[i];
	}
}
	
Logger::eLogLevel Logger::GetLogLevel()
{
	return logLevel;
}
	
const char8 * Logger::GetLogLevelString(eLogLevel ll)
{
	return logLevelString[ll];
}
	
void Logger::SetLogLevel(eLogLevel ll)
{
	logLevel = ll;
}
	
void Logger::Log(eLogLevel ll, const char8* text, ...)
{
	if (ll < logLevel)return; 
	
	va_list vl;
	va_start(vl, text);
	Logv(ll, text, vl);
	va_end(vl);
}	

void Logger::FrameworkDebug( const char8 * text, ... )
{
	va_list vl;
	va_start(vl, text);
	if (Logger::Instance())
		Logger::Instance()->Logv(LEVEL_FRAMEWORK, text, vl);
	va_end(vl);
}

void Logger::Debug(const char8 * text, ...)
{
	va_list vl;
	va_start(vl, text);
    if (Logger::Instance())
        Logger::Instance()->Logv(LEVEL_DEBUG, text, vl);
	va_end(vl);
}
	
void Logger::Info(const char8 * text, ...)
{
	va_list vl;
	va_start(vl, text);
    if (Logger::Instance())
        Logger::Instance()->Logv(LEVEL_INFO, text, vl);
	va_end(vl);
}	
	
void Logger::Warning(const char8 * text, ...)
{
	va_list vl;
	va_start(vl, text);
    if (Logger::Instance())
        Logger::Instance()->Logv(LEVEL_WARNING, text, vl);
	va_end(vl);
}
	
void Logger::Error(const char8 * text, ...)
{
	va_list vl;
	va_start(vl, text);
    if (Logger::Instance())
        Logger::Instance()->Logv(LEVEL_ERROR, text, vl);
	va_end(vl);
}

void Logger::AddCustomOutput(DAVA::LoggerOutput *lo)
{
	if(Logger::Instance() && lo)
		Logger::Instance()->customOutputs.push_back(lo);
}

void Logger::RemoveCustomOutput(DAVA::LoggerOutput *lo)
{
	if(Logger::Instance() && lo)
	{
		Vector<LoggerOutput *>::const_iterator endIt = Logger::Instance()->customOutputs.end();
		for(Vector<LoggerOutput *>::iterator it = Logger::Instance()->customOutputs.begin(); it != endIt; ++it)
		{
			if((*it) == lo)
			{
				Logger::Instance()->customOutputs.erase(it);
				break;
			}
		}
	}
}

void Logger::SetLogFilename(const String & filename)
{
	if(filename.empty())
	{
        logFilename = FilePath();
	}
	else
	{
		logFilename = FileSystem::Instance()->GetCurrentDocumentsDirectory() + filename;
	}
}

void Logger::SetLogPathname(const FilePath & filepath)
{
	logFilename = filepath;
}


void Logger::FileLog(eLogLevel ll, const char8* text)
{
	if(FileSystem::Instance())
	{
        File *file = File::Create(logFilename, File::APPEND | File::WRITE);
		if(file)
		{
            char8 prefix[128];
            snprintf(prefix, 128, "[%s] ", GetLogLevelString(ll));
            file->Write(prefix, sizeof(char) * strlen(prefix));
            file->Write(text, sizeof(char) * strlen(text));
            file->Release();
		}
	}
}

void Logger::CustomLog(eLogLevel ll, const char8* text)
{
	for(size_t i = 0; i < customOutputs.size(); ++i)
	{
		customOutputs[i]->Output(ll, text);
	}
}

void Logger::EnableConsoleMode()
{
    consoleModeEnabled = true;
}

    
void Logger::ConsoleLog(DAVA::Logger::eLogLevel ll, const char8 *text)
{
    printf("[%s] %s", GetLogLevelString(ll), text);
}

void Logger::Output(eLogLevel ll, const char8* formadedMsg)
{
    CustomLog(ll, formadedMsg);

    // print platform log or write log to file
    // only if log level is acceptable
    if (ll >= logLevel)
    {
        if (consoleModeEnabled)
        {
            ConsoleLog(ll, formadedMsg);
        }
        else
        {
            PlatformLog(ll, formadedMsg);
        }

        if (!logFilename.IsEmpty())
        {
            FileLog(ll, formadedMsg);
        }
    }
}

} // end namespace DAVA



