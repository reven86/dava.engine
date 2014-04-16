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


#ifndef __DAVAENGINE_LOGGER_H__
#define __DAVAENGINE_LOGGER_H__

#include "Base/BaseTypes.h"
#include "Base/Singleton.h"
#include "Platform/Mutex.h"

namespace DAVA 
{

enum eLogLevel
{
    LEVEL_ALL = 0,
	LEVEL_FRAMEWORK = LEVEL_ALL,
	LEVEL_DEBUG,
	LEVEL_INFO,
	LEVEL_WARNING,
	LEVEL_ERROR,

    LEVEL_USER,

	LEVEL_DISABLE = 0xFFFF
};

struct LoggerRecord
{
    const char* text;
};

class LoggerOutput
{
public:
	LoggerOutput() : level (LEVEL_ALL) {}
	virtual ~LoggerOutput() {}

    eLogLevel GetLogLevel() const { return level; };
    void SetLogLevel(eLogLevel ll) { level = ll; };

	virtual void Output(const LoggerRecord &record) const = 0;

private:
    eLogLevel level;
};

class Logger : public Singleton<Logger>
{
public:
	Logger();
	virtual ~Logger();

	static eLogLevel GetLogLevel();
	static void SetLogLevel(eLogLevel ll);

	static void Log(eLogLevel ll, const char* text, ...);
    //static void LogVariable();
    //static void Backtrace();

	static void Framework(const char * text, ...);
	static void Debug(const char * text, ...);
	static void Info(const char * text, ...);
	static void Warning(const char * text, ...);
	static void Error(const char * text, ...);

	static void AddCustomOutput(LoggerOutput *lo);
	static void RemoveCustomOutput(LoggerOutput *lo);

protected:
    Mutex logMutex;
	eLogLevel logLevel;
	Vector<LoggerOutput *> logOutputs;

    static const int logMaxTextLength = 4096;
    static char logTextBuffer[logMaxTextLength];

	void Logv(eLogLevel ll, const char* text, va_list li);
};

};

#endif // __DAVAENGINE_LOGGER_H__