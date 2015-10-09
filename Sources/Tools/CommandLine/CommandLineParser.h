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

#ifndef __DAVAENGINE_COMMANDLINEPARSER_H__
#define __DAVAENGINE_COMMANDLINEPARSER_H__

#include "Base/BaseTypes.h"
#include "Base/StaticSingleton.h"

namespace DAVA
{
class CommandLineParser : public StaticSingleton<CommandLineParser>
{
public:
    CommandLineParser();
    virtual ~CommandLineParser();

    void SetVerbose(bool isVerbose);
    bool GetVerbose() const;

    void SetExtendedOutput(bool isExO);
    bool IsExtendedOutput() const;

    void SetUseTeamcityOutput(bool use);
    bool UseTeamcityOutput() const;

    void SetFlags(const Vector<String>& arguments);
    void ClearFlags();

    bool IsFlagSet(const String& s) const;
    String GetParamForFlag(const String& flag);
    Vector<String> GetParamsForFlag(const String& flag);

    static int32 GetCommandsCount();

    static String GetCommand(uint32 commandPosition);
    static String GetCommandParam(const String& command);

    DAVA_DEPRECATED(static String GetCommandParamAdditional(const String& command, const int32 paramIndex)); //TODO: remove this method after fix of DF-1584

    static bool CommandIsFound(const String& command);

private:
    static int32 GetCommandPosition(const DAVA::String& command);

    struct Flag
    {
        explicit Flag(const String& f)
            : name(f)
        {
        }
        String name;
        Vector<String> params;
    };

    Vector<Flag> flags;
    bool isVerbose;
    bool isExtendedOutput;
    bool useTeamcityOutput;
};
}

#endif // __DAVAENGINE_COMMANDLINEPARSER_H__