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

#include "FileSystem/FileSystem.h"
#include "FileSystem/VariantType.h"

class ProgramOptions
{
public:
    
    ProgramOptions(const DAVA::String _commandName) : commandName(_commandName)
    {
    }
    
    void AddOption(const char *optionName, const DAVA::VariantType &defaultValue, const char *description = NULL)
    {
        Option op;
        op.name = optionName;
        op.value = defaultValue;
    
        if(NULL != description)
        {
            op.descr = description;
        }

        options.push_back(op);
    }

    void AddArgument(const char *argumentName, bool required = true)
    {
        Argument ar;
        ar.name = argumentName;
        ar.required = required;
        ar.set = false;
        arguments.push_back(ar);
    }

    bool Parse(int argc, char *argv[], size_t start = 1)
    {
        bool ret = true;
        size_t curParamPos = 0;

        argValues = argv;
        argCount = (size_t) argc;
        argIndex = start;
        
        while(ret && argIndex < argCount)
        {
            // search if there is options with such name
            if(!ParseOption())
            {
                // set required
                if(curParamPos < arguments.size())
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
        for(size_t i = 0; i < arguments.size(); ++i)
        {
            if(arguments[i].required && !arguments[i].set)
            {
                printf("Error - required argument not specified: %s\n", arguments[i].name.c_str());
                ret = false;
                break;
            }
        }

        return ret;
    }

    void PrintUsage()
    {
        printf("  %s ", commandName.c_str());

        if(options.size() > 0)
        {
            printf("[options] ");
        }

        for(size_t i = 0; i < arguments.size(); ++i)
        {
            if(arguments[i].required)
            {
                printf("<%s> ", arguments[i].name.c_str());
            }
            else
            {
                printf("[%s] ", arguments[i].name.c_str());
            }
        }
        printf("\n");

        for(size_t i = 0; i < options.size(); ++i)
        {
            printf("\t%s", options[i].name.c_str());

            int optionType = options[i].value.GetType();
            if(optionType != DAVA::VariantType::TYPE_BOOLEAN)
            {
                printf(" <value>\t");
            }
            else
            {
                printf("\t\t");
            }

            if(!options[i].descr.empty())
            {
                printf("- %s\n", options[i].descr.c_str());
            }
            else
            {
                printf("\n");
            }
        }
    }

    DAVA::VariantType GetOption(const char *optionName) const
    {
        DAVA::VariantType v;

        for(size_t i = 0; i < options.size(); ++i)
        {
            if(options[i].name == optionName)
            {
                v = options[i].value;
                break;
            }
        }

        return v;
    }

    DAVA::String GetArgument(const char *argumentName) const
    {
        DAVA::String ret;

        for(size_t i = 0; i < arguments.size(); ++i)
        {
            if(arguments[i].name == argumentName)
            {
                ret = arguments[i].value;
                break;
            }
        }

        return ret;
    }
    
    const DAVA::String & GetCommand() const
    {
        return commandName;
    }

protected:
    struct Option
    {
        DAVA::String name;
        DAVA::String alias;
        DAVA::String descr;
        DAVA::VariantType value;
    };

    struct Argument
    {
        bool required;
        bool set;
        DAVA::String name;
        DAVA::String value;
    };

    bool ParseOption()
    {
        bool ret = false;
        const char *str = argValues[argIndex];

        for(size_t i = 0; i < options.size(); ++i)
        {
            const char *optionName = options[i].name.c_str();
            size_t optionNameLen = options[i].name.length();
            size_t index = 0;
            
            for(index = 0; index < optionNameLen; ++index)
            {
                if(optionName[index] != str[index])
                {
                    break;
                }
            }

            // found
            if(index == optionNameLen)
            {
                if(optionNameLen == strlen(str))
                {
                    if(options[i].value.GetType() == DAVA::VariantType::TYPE_BOOLEAN)
                    {
                        // bool option don't need any arguments
                        options[i].value.SetBool(true);
                        ret = true;
                    }
                    else
                    {
                        argIndex++;
                        if(argIndex < argCount)
                        {
                            const char *valueStr = argValues[argIndex];

                            int optionType = options[i].value.GetType();
                            switch(optionType)
                            {
                                case DAVA::VariantType::TYPE_STRING:
                                case DAVA::VariantType::TYPE_NONE:
                                    options[i].value.SetString(valueStr);
                                    break;
                                case DAVA::VariantType::TYPE_INT32:
                                    {
                                        DAVA::int32 value = 0;
                                        if(1 == sscanf(valueStr, "%d", &value))
                                        {
                                            options[i].value.SetInt32(value);
                                        }
                                    }
                                    break;
                                case DAVA::VariantType::TYPE_UINT32:
                                    {
                                        DAVA::uint32 value = 0;
                                        if(1 == sscanf(valueStr, "%u", &value))
                                        {
                                            options[i].value.SetUInt32(value);
                                        }
                                    }
                                    break;
                                case DAVA::VariantType::TYPE_BOOLEAN:
                                    if(strcmp(valueStr, "true"))
                                    {
                                        options[i].value.SetBool(true);
                                    }
                                    else if(strcmp(valueStr, "false"))
                                    {
                                        options[i].value.SetBool(false);
                                    }
                                    break;
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

    char **argValues;
    size_t argCount;
    size_t argIndex;
    DAVA::Vector<Argument> arguments;
    DAVA::Vector<Option> options;
    
    DAVA::String commandName;
};


#endif //__PROGRAM_OPTIONS_H__