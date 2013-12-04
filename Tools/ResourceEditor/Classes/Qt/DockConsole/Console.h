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


#ifndef __QT_CONSOLE_H__
#define __QT_CONSOLE_H__

#include <QObject>

#include "Base/StaticSingleton.h"
#include "FileSystem/Logger.h"

class LoggerToConsole : public DAVA::LoggerOutput
{
public:
	LoggerToConsole();
	~LoggerToConsole();

	virtual void Output(DAVA::Logger::eLogLevel ll, const DAVA::char8* text) const;
	virtual void Output(DAVA::Logger::eLogLevel ll, const DAVA::char16* text) const;
};

class Console : public QObject, public DAVA::Singleton<Console>
{
	Q_OBJECT

public:
	Console();
	~Console();

	void Clear();
	void Echo(const DAVA::String &text);

	size_t GetLineCount() const;
	const DAVA::String& GetLine(size_t index) const;

signals:
	void OnClear();
	void OnEcho();

protected:
	DAVA::Vector<DAVA::String> log;

	LoggerToConsole *output;
};

#endif // __QT_CONSOLE_H__
