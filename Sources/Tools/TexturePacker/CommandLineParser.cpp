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


#include <stdlib.h>
#include "CommandLineParser.h"
#include "Core/Core.h"

#include <cerrno>

namespace DAVA
{
    
CommandLineParser::CommandLineParser()
    : isVerbose(false)
    , isExtendedOutput(false)
    , useTeamcityOutput(false)
{
}

void CommandLineParser::SetArguments(const Vector<String> &arguments)
{
    Clear();
    
    bool prevIsFlag = false;
	for (size_t i = 0; i < arguments.size(); ++i)
	{
		String arg = arguments[i];
        String::size_type argLen = arg.length();
        
		if ((argLen >= 1) && (arg[0] == '-'))
        {
            if(prevIsFlag)
                params.push_back(String());
            
			flags.push_back(arg);
            prevIsFlag = true;
        }
		else
        {
            params.push_back(arg);
            prevIsFlag = false;
        }
	}
    
    if(prevIsFlag)
        params.push_back(String());
}

void CommandLineParser::AddArgument(const String & arg)
{
    flags.push_back(arg);
    params.push_back(String());
}
    
void CommandLineParser::SetArguments(int argc, char * argv[])
{
    Clear();

    bool prevIsFlag = false;
	for (int i = 0; i < argc; ++i)
	{
		char * arg = argv[i];
		size_t argLen = strlen(arg);
        
		if ((argLen >= 1) && (arg[0] == '-'))
        {
            if(prevIsFlag)
                params.push_back(String());
            
			flags.push_back(String(arg));
            prevIsFlag = true;
        }
		else
        {
            params.push_back(String(arg));
            prevIsFlag = false;
        }
	}
    
    if(prevIsFlag)
        params.push_back(String());
}

void CommandLineParser::Clear()
{	
	flags.clear();
    params.clear();
}


void CommandLineParser::SetVerbose(bool _isVerbose)
{
	isVerbose = _isVerbose;
}

bool CommandLineParser::GetVerbose() const
{
	return isVerbose;
}

void CommandLineParser::SetExtendedOutput(bool isExO)
{
	isExtendedOutput = isExO;
}

bool CommandLineParser::IsExtendedOutput() const
{
	return isExtendedOutput;
}

void CommandLineParser::SetUseTeamcityOutput(bool use)
{
    useTeamcityOutput = use;
}

bool CommandLineParser::UseTeamcityOutput() const
{
    return useTeamcityOutput;
}


CommandLineParser::~CommandLineParser()
{
}

bool CommandLineParser::IsFlagSet(const String & s)
{
	for (uint32 k = 0; k < flags.size(); ++k)
		if (flags[k] == s)return true;
	return false;
}

    
String	CommandLineParser::GetParamForFlag(const String & flag)
{
    DVASSERT(flags.size() == params.size());
    
	for (uint32 k = 0; k < flags.size(); ++k)
    {
		if (flags[k] == flag)
        {
            return params[k];
        }
    }

	return String();
}
    

bool CommandLineParser::CommandIsFound(const DAVA::String &command)
{
    return (INVALID_POSITION != GetCommandPosition(command));
}

DAVA::String CommandLineParser::GetCommand(DAVA::uint32 commandPosition)
{
    const Vector<String>& commandLine = Core::Instance()->GetCommandLine();
	if(commandPosition < commandLine.size())
	{
		return commandLine[commandPosition];
	}
    
	return String();
}


DAVA::int32 CommandLineParser::GetCommandPosition(const DAVA::String &command)
{
    int32 position = INVALID_POSITION;
    
    const Vector<String> & commandLine = Core::Instance()->GetCommandLine();
    for(int32 i = 0; i < (int32)commandLine.size(); ++i)
    {
        if(command == commandLine[i])
        {
            position = i;
            break;
        }
    }
    
    return position;
}

DAVA::String CommandLineParser::GetCommandParam(const DAVA::String &command)
{
    int32 commandPosition = GetCommandPosition(command);
    if(CheckPosition(commandPosition))
    {
        return GetCommand(commandPosition + 1);
    }
    
    return String();
}

String CommandLineParser::GetCommandParamAdditional(const String &command, const int32 paramIndex)	//TODO: remove this method after fix of DF-1584
{
	int32 commandPosition = GetCommandPosition(command);
	int32 firstParamPosition = commandPosition + 1;
	return GetCommand(firstParamPosition + paramIndex);
}


int32 CommandLineParser::GetCommandsCount()
{
    const Vector<String> & commandLine = Core::Instance()->GetCommandLine();
    return (int32)commandLine.size();
}

bool CommandLineParser::CheckPosition(int32 commandPosition)
{
    if(     (INVALID_POSITION == commandPosition)
       ||   (GetCommandsCount() < commandPosition + 2))
    {
        return false;
    }
    
    return true;
}


};
