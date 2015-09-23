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

void ProgramOptions::Option::SetValue(const DAVA::VariantType& value)
{
    if (multipleValuesSuported || values.size() == 0)
    {
        values.push_back(value);
    }
    else
    {
        values[0] = value;
    }
}

ProgramOptions::ProgramOptions(const DAVA::String& _commandName)
    : commandName(_commandName)
{
}

void ProgramOptions::AddOption(const char* optionName, const DAVA::VariantType& defaultValue, const char* description, bool canBeMultiple)
{
    Option op;
    op.name = optionName;
    op.multipleValuesSuported = canBeMultiple;
    op.defaultValue = defaultValue;

    if (nullptr != description)
    {
        op.descr = description;
    }

    options.push_back(op);
}

void ProgramOptions::AddArgument(const char* argumentName, bool required)
{
    Argument ar;
    ar.name = argumentName;
    ar.required = required;
    ar.set = false;
    arguments.push_back(ar);
}

bool ProgramOptions::Parse(int argc, char* argv[], size_t start)
{
    bool ret = true;
    size_t curParamPos = 0;

    argValues = argv;
    argCount = (size_t)argc;
    argIndex = start;

    while (ret && argIndex < argCount)
    {
        // search if there is options with such name
        if (!ParseOption())
        {
            // set required
            if (curParamPos < arguments.size())
            {
                arguments[curParamPos].value = argValues[argIndex];
                arguments[curParamPos].set = true;
                curParamPos++;
            }
            else
            {
                printf("Error - unknown argument: %s\n", argValues[argIndex]);
                ret = false;
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
            ret = false;
        }
    }

    return ret;
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
        if (optionType != DAVA::VariantType::TYPE_BOOLEAN)
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

DAVA::uint32 ProgramOptions::GetOptionsCount(const char* optionName) const
{
    for (auto& opt : options)
    {
        if (opt.name == optionName)
        {
            DAVA::uint32 count = opt.values.size();
            return (count > 0) ? count : 1; //real arguments or default
        }
    }

    return 1; //default
}

DAVA::VariantType ProgramOptions::GetOption(const char* optionName, size_t pos) const
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

    return DAVA::VariantType();
}

DAVA::String ProgramOptions::GetArgument(const char* argumentName) const
{
    for (auto& arg : arguments)
    {
        if (arg.name == argumentName)
        {
            return arg.value;
        }
    }

    return DAVA::String();
}

const DAVA::String& ProgramOptions::GetCommand() const
{
    return commandName;
}

bool ProgramOptions::ParseOption()
{
    bool ret = false;
    const char* str = argValues[argIndex];

    for (size_t i = 0; i < options.size(); ++i)
    {
        const char* optionName = options[i].name.c_str();
        size_t optionNameLen = options[i].name.length();
        size_t index = 0;

        for (index = 0; index < optionNameLen; ++index)
        {
            if (optionName[index] != str[index])
            {
                break;
            }
        }

        // found
        if (index == optionNameLen)
        {
            if (optionNameLen == strlen(str))
            {
                if (options[i].defaultValue.GetType() == DAVA::VariantType::TYPE_BOOLEAN)
                {
                    // bool option don't need any arguments
                    options[i].SetValue(DAVA::VariantType(true));
                    ret = true;
                }
                else
                {
                    argIndex++;
                    if (argIndex < argCount)
                    {
                        const DAVA::String valueStr = DAVA::String(argValues[argIndex]);
                        DAVA::Vector<DAVA::String> tokens; //one or more params

                        if (options[i].multipleValuesSuported)
                        {
                            DAVA::Split(valueStr, ",", tokens, true, false);
                        }
                        else
                        {
                            tokens.push_back(valueStr);
                        }

                        int optionType = options[i].defaultValue.GetType();
                        switch (optionType)
                        {
                        case DAVA::VariantType::TYPE_STRING:
                        case DAVA::VariantType::TYPE_NONE:
                            {
                                for (auto& t : tokens)
                                {
                                    options[i].SetValue(DAVA::VariantType(t));
                                }
                                break;
                            }
                            case DAVA::VariantType::TYPE_INT32:
                                {
                                    for (auto& t : tokens)
                                    {
                                        DAVA::int32 value = 0;
                                        if (1 == sscanf(t.c_str(), "%d", &value))
                                        {
                                            options[i].SetValue(DAVA::VariantType(value));
                                        }
                                    }
                                }
                                break;
                                case DAVA::VariantType::TYPE_UINT32:
                                {
                                    for (auto& t : tokens)
                                    {
                                        DAVA::uint32 value = 0;
                                        if (1 == sscanf(t.c_str(), "%u", &value))
                                        {
                                            options[i].SetValue(DAVA::VariantType(value));
                                        }
                                    }
                                }
                                break;
                                case DAVA::VariantType::TYPE_UINT64:
                            {
                                for (auto& t : tokens)
                                {
                                    DAVA::uint64 value = 0;
                                    if (1 == sscanf(t.c_str(), "%llu", &value))
                                    {
                                        options[i].SetValue(DAVA::VariantType(value));
                                    }
                                }
                            }
                            break;
                            case DAVA::VariantType::TYPE_BOOLEAN:
                            {
                                for (auto& t : tokens)
                                {
                                    if (strcmp(t.c_str(), "true"))
                                    {
                                        options[i].SetValue(DAVA::VariantType(true));
                                    }
                                    else if (strcmp(t.c_str(), "false"))
                                    {
                                        options[i].SetValue(DAVA::VariantType(false));
                                    }
                                }
                                break;
                            }
                            default:
                                DVASSERT(0 && "Not implemented")
                                break;
                        }

                        ret = true;
                        break;
                    }
                }
            }
        }
    }

    return ret;
}


