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

#include "CommandLine/ProgramOptions.h"

#include "FileSystem/FileSystem.h"

#include "Utils/Utils.h"

namespace DAVA
{
void ProgramOptions::Option::SetValue(const VariantType& value)
{
    if (multipleValuesSuported || values.empty())
    {
        values.push_back(value);
    }
    else
    {
        values[0] = value;
    }
}

ProgramOptions::ProgramOptions(const String& _commandName)
    : commandName(_commandName)
{
}

void ProgramOptions::AddOption(const String& optionName, const VariantType& defaultValue, const String& description, bool canBeMultiple)
{
    Option op;
    op.name = optionName;
    op.multipleValuesSuported = canBeMultiple;
    op.defaultValue = defaultValue;
    op.descr = description;

    options.push_back(op);
}

void ProgramOptions::AddArgument(const String& argumentName, bool required)
{
    Argument ar;
    ar.name = argumentName;
    ar.required = required;
    ar.set = false;
    arguments.push_back(ar);
}

bool ProgramOptions::Parse(uint32 argc, char* argv[])
{
    Vector<String> commandLine(argv, argv + argc);
    return Parse(commandLine);
}
bool ProgramOptions::Parse(const Vector<String>& commandLine)
{
    uint32 argc = commandLine.size();
    const Vector<String>& argv = commandLine;

    // if first argument equal command name we should skip it else we should stop parsing
    uint32 argIndex = 1; //skip executable pathname in params
    if (argIndex < argc && commandName == String(argv[argIndex]))
    {
        argIndex++;
    }
    else
    {
        return false;
    }

    uint32 curParamPos = 0;
    while (argIndex < argc)
    {
        // search if there is options with such name
        if (!ParseOption(argIndex, commandLine))
        {
            // set required
            if (curParamPos < arguments.size())
            {
                arguments[curParamPos].value = argv[argIndex];
                arguments[curParamPos].set = true;
                curParamPos++;
            }
            else if (argIndex < argc)
            {
                printf("Error - unknown argument: [%d] %s\n", argIndex, argv[argIndex]);
                return false;
            }
            else
            {
                printf("Error of parsing command line\n");
                return false;
            }
        }

        argIndex++;
    }

    // check if there is all required parameters
    for (auto& arg : arguments)
    {
        if (arg.required && !arg.set)
        {
            printf("Error - required argument not specified: %s\n", arg.name.c_str());
            return false;
        }
    }

    return true;
}

bool ProgramOptions::ParseOption(uint32& argIndex, const Vector<String>& commandLine)
{
    uint32 argc = commandLine.size();
    const Vector<String>& argv = commandLine;

    const String argString = argv[argIndex];
    for (auto& opt : options)
    {
        if (opt.name == argString)
        {
            if (opt.defaultValue.GetType() == VariantType::TYPE_BOOLEAN)
            {
                // bool option don't need any arguments
                opt.SetValue(VariantType(true));
                return true;
            }
            else
            {
                argIndex++;
                if (argIndex < argc)
                {
                    const String valueStr = argv[argIndex];
                    Vector<String> tokens; //one or more params

                    if (opt.multipleValuesSuported)
                    {
                        Split(valueStr, ",", tokens, false, false);
                    }
                    else
                    {
                        tokens.push_back(valueStr);
                    }

                    const VariantType::eVariantType optionType = opt.defaultValue.GetType();
                    switch (optionType)
                    {
                    case VariantType::TYPE_STRING:
                    case VariantType::TYPE_NONE:
                    {
                        for (auto& t : tokens)
                        {
                            opt.SetValue(VariantType(t));
                        }
                        break;
                    }
                    case VariantType::TYPE_INT32:
                    {
                        for (auto& t : tokens)
                        {
                            int32 value = 0;
                            if (1 == sscanf(t.c_str(), "%d", &value))
                            {
                                opt.SetValue(VariantType(value));
                            }
                        }
                        break;
                    }
                    case VariantType::TYPE_UINT32:
                    {
                        for (auto& t : tokens)
                        {
                            uint32 value = 0;
                            if (1 == sscanf(t.c_str(), "%u", &value))
                            {
                                opt.SetValue(VariantType(value));
                            }
                        }
                        break;
                    }
                    case VariantType::TYPE_UINT64:
                    {
                        for (auto& t : tokens)
                        {
                            uint64 value = 0;
                            if (1 == sscanf(t.c_str(), "%llu", &value))
                            {
                                opt.SetValue(VariantType(value));
                            }
                        }
                        break;
                    }
                    case VariantType::TYPE_BOOLEAN:
                    {
                        for (auto& t : tokens)
                        {
                            if (strcmp(t.c_str(), "true"))
                            {
                                opt.SetValue(VariantType(true));
                            }
                            else if (strcmp(t.c_str(), "false"))
                            {
                                opt.SetValue(VariantType(false));
                            }
                        }
                        break;
                    }
                    default:
                        DVASSERT(0 && "Not implemented");
                        break;
                    }

                    return true;
                }
                break;
            }
        }
    }

    return false;
}

void ProgramOptions::PrintUsage() const
{
    printf("  %s ", commandName.c_str());

    if (options.size() > 0)
    {
        printf("[options] ");
    }

    for (auto& arg : arguments)
    {
        if (arg.required)
        {
            printf("<%s> ", arg.name.c_str());
        }
        else
        {
            printf("[%s] ", arg.name.c_str());
        }
    }

    printf("\n");

    for (auto& opt : options)
    {
        printf("\t%s", opt.name.c_str());

        int optionType = opt.defaultValue.GetType();
        if (optionType != VariantType::TYPE_BOOLEAN)
        {
            printf(" <value>");
            if (opt.multipleValuesSuported)
            {
                printf(",[additional values...]");
            }
            printf("\t");
        }
        else
        {
            printf("\t\t");
        }

        if (!opt.descr.empty())
        {
            printf("- %s", opt.descr.c_str());
        }

        printf("\n");
    }
}

uint32 ProgramOptions::GetOptionValuesCount(const String& optionName) const
{
    for (auto& opt : options)
    {
        if (opt.name == optionName)
        {
            uint32 count = static_cast<uint32>(opt.values.size());
            return (count > 0) ? count : 1; //real arguments or default
        }
    }

    return 1; //default
}

VariantType ProgramOptions::GetOption(const String& optionName, uint32 pos) const
{
    for (auto& opt : options)
    {
        if (opt.name == optionName)
        {
            const auto count = opt.values.size();
            if (count > 0)
            {
                DVASSERT(pos < opt.values.size());
                if (pos < opt.values.size())
                {
                    return opt.values[pos];
                }
            }

            return opt.defaultValue;
        }
    }

    return VariantType();
}

String ProgramOptions::GetArgument(const String& argumentName) const
{
    for (auto& arg : arguments)
    {
        if (arg.name == argumentName)
        {
            return arg.value;
        }
    }

    return String();
}

const String& ProgramOptions::GetCommand() const
{
    return commandName;
}

} //END of DAVA