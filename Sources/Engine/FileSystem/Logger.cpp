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
#include <stdarg.h>

#include "Utils/Utils.h"

namespace DAVA 
{

#if defined(__DAVAENGINE_WIN32__)

#define vsnprintf _vsnprintf
#define vswprintf _vsnwprintf
#define snprintf _snprintf

#endif

void Logger::Logv(eLogLevel ll, const char8* text, va_list li)
{
    if(!text || text[0] == '\0') return;
    
	char tmp[4096] = {0};

	vsnprintf(tmp, sizeof(tmp) - 2, text, li);
	strcat(tmp, "\n");

	// always send log to custom subscribers
	CustomLog(ll, tmp);

	// print platform log or write log to file
	// only if log level is acceptable
	if (ll >= logLevel)
	{
        if(consoleModeEnabled)
        {
            ConsoleLog(ll, tmp);
        }
        else
        {
            PlatformLog(ll, tmp);
        }

		if(!logFilename.IsEmpty())
		{
			FileLog(ll, tmp);
		}
	}
}

void Logger::Logv(eLogLevel ll, const char16* text, va_list li)
{
    if(!text || text[0] == '\0') return;

	wchar_t tmp[4096] = {0};

	vswprintf(tmp, sizeof(tmp)/sizeof(wchar_t) - 2, text, li);
	wcscat(tmp, L"\n");

	// always send log to custom subscribers
	CustomLog(ll, tmp);

	// print platform log or write log to file
	// only if log level is acceptable
	if (ll >= logLevel)
	{
        if(consoleModeEnabled)
        {
            ConsoleLog(ll, tmp);
        }
        else
        {
            PlatformLog(ll, tmp);
        }

		if(!logFilename.IsEmpty())
		{
			FileLog(ll, tmp);
		}

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

void Logger::Log(eLogLevel ll, const char16* text, ...)
{
	if (ll < logLevel) return; 
	
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

void Logger::FrameworkDebug( const char16 * text, ... )
{
	va_list vl;
	va_start(vl, text);
	if (Logger::Instance())
		Logger::Instance()->Logv(LEVEL_FRAMEWORK, text, vl);
	va_end(vl);
}

void Logger::Debug(const char16 * text, ...)
{
	va_list vl;
	va_start(vl, text);
    if (Logger::Instance())
        Logger::Instance()->Logv(LEVEL_DEBUG, text, vl);
	va_end(vl);
}

void Logger::Info(const char16 * text, ...)
{
	va_list vl;
	va_start(vl, text);
    if (Logger::Instance())
        Logger::Instance()->Logv(LEVEL_INFO, text, vl);
	va_end(vl);
}	

void Logger::Warning(const char16 * text, ...)
{
	va_list vl;
	va_start(vl, text);
    if (Logger::Instance())
        Logger::Instance()->Logv(LEVEL_WARNING, text, vl);
	va_end(vl);
}

void Logger::Error(const char16 * text, ...)
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
            snprintf(prefix, 127, "[%s] ", GetLogLevelString(ll));
            file->Write(prefix, sizeof(char) * strlen(prefix));
            file->Write(text, sizeof(char) * strlen(text));
            file->Release();
		}
	}
}

void Logger::FileLog(eLogLevel ll, const char16* text)
{
	if(FileSystem::Instance())
	{
        File *file = File::Create(logFilename, File::APPEND | File::WRITE);
		if(file)
		{
            char16 prefix[128];
            swprintf(prefix, 127, L"[%s] ",  StringToWString(GetLogLevelString(ll)).c_str());

            file->Write(prefix, sizeof(wchar_t) * wcslen(prefix));
            file->Write(text, sizeof(wchar_t) * wcslen(text));
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

void Logger::CustomLog(eLogLevel ll, const char16* text)
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

void Logger::ConsoleLog(DAVA::Logger::eLogLevel ll, const char16 *text)
{
    wprintf(L"[%s] %s", StringToWString(GetLogLevelString(ll)).c_str(), text);
}


}



