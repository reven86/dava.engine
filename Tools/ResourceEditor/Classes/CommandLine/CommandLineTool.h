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


#ifndef __COMMAND_LINE_TOOL_H__
#define __COMMAND_LINE_TOOL_H__

#include "DAVAEngine.h"

class CommandLineTool
{    
public:
	using EngineHelperCallback = DAVA::Function<void()>;

public:
    
	CommandLineTool();
    virtual ~CommandLineTool() {};

	virtual DAVA::String GetCommandLineKey() const = 0;
    
    virtual bool InitializeFromCommandLine() = 0;
  
    virtual void Process(EngineHelperCallback) = 0;

	virtual void PrintUsage() const = 0;
    
    virtual DAVA::FilePath GetQualityConfigPath() const {return DAVA::FilePath(); };
    
	virtual void DumpParams() const {};

    inline const DAVA::Set<DAVA::String> & GetErrorList() const;
    
	inline bool IsOneFrameCommand() const;

    static DAVA::FilePath CreateProjectPathFromPath(const DAVA::FilePath & pathname);

    
protected:
    
    DAVA::FilePath CreateQualityConfigPath(const DAVA::FilePath & path) const;
    
    
    DAVA::Set<DAVA::String> errors;
	bool oneFrameCommand;
};


inline const DAVA::Set<DAVA::String> & CommandLineTool::GetErrorList() const
{
    return errors;
}

inline bool CommandLineTool::IsOneFrameCommand() const
{
	return oneFrameCommand;
}


#endif // __COMMAND_LINE_TOOL_H__


