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


#include "DockConsole/Console.h"

Console::Console()
{
	output = new LoggerToConsole();
	DAVA::Logger::AddCustomOutput(output);
	log.reserve(5000);
}

Console::~Console()
{
	DAVA::Logger::RemoveCustomOutput(output);
}

void Console::Clear()
{
	if(log.size() > 0)
	{
		log.clear();
		emit OnClear();
	}
}

void Console::Echo(const DAVA::String &text)
{
	bool echo = false;

	size_t begin = 0;
	size_t end = text.find('\n', 0);

	while(end != DAVA::String::npos)
	{
		log.push_back(text.substr(begin, end - begin));
		echo = true;

		begin = end + 1;
		end = text.find('\n', begin);
	}

	if(begin < text.size())
	{
		log.push_back(text.substr(begin));
		echo = true;
	}

	if(echo)
	{
		emit OnEcho();
	}
}

size_t Console::GetLineCount() const
{
	return log.size();
}

const DAVA::String& Console::GetLine(size_t index) const
{
	static DAVA::String empty = "";
	
	if(index < log.size())
	{
		return log[index];
	}
	
	return empty;
}

LoggerToConsole::LoggerToConsole()
{ }

LoggerToConsole::~LoggerToConsole()
{ }

void LoggerToConsole::Output(DAVA::Logger::eLogLevel ll, const DAVA::char8* text) const
{
	Console::Instance()->Echo(text);
}

void LoggerToConsole::Output(DAVA::Logger::eLogLevel ll, const DAVA::char16* text) const
{

}
