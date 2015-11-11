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

#ifndef __PROGRAM_OPTIONS_H__
#define __PROGRAM_OPTIONS_H__

#include "Base/BaseTypes.h"
#include "FileSystem/VariantType.h"

namespace DAVA
{
class ProgramOptions
{
public:
    ProgramOptions(const DAVA::String& _commandName);

    void AddOption(const DAVA::String& optionName, const DAVA::VariantType& defaultValue, const DAVA::String& description = nullptr, bool canBeMultiple = false);
    void AddArgument(const DAVA::String& argumentName, bool required = true);

    DAVA::uint32 GetOptionVaulesCount(const DAVA::String& optionName) const;
    DAVA::VariantType GetOption(const DAVA::String& optionName, size_t pos = 0) const;

    DAVA::String GetArgument(const DAVA::String& argumentName) const;
    const DAVA::String& GetCommand() const;

    bool Parse(int argc, char* argv[]);
    void PrintUsage() const;

private:
    bool ParseOption();

private:
    struct Option
    {
        void SetValue(const DAVA::VariantType& value);

        DAVA::String name;
        DAVA::String alias;
        DAVA::String descr;
        bool multipleValuesSuported = false;
        DAVA::VariantType defaultValue;
        DAVA::Vector<DAVA::VariantType> values;
    };

    struct Argument
    {
        bool required = false;
        bool set = false;
        DAVA::String name;
        DAVA::String value;
    };

    char** argValues = nullptr;
    size_t argCount = 0;
    size_t argIndex = 0;

    DAVA::Vector<Argument> arguments;
    DAVA::Vector<Option> options;

    DAVA::String commandName;
};

} //END of DAVA
#endif //__PROGRAM_OPTIONS_H__