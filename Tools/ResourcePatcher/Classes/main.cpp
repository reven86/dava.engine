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

#include "DLC/Patcher/PatchFile.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/VariantType.h"

class ProgramOptions
{
public:
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

    void PrintUsage(const char *executable)
    {
        printf("%s ", executable);

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

    DAVA::VariantType GetOption(const char *optionName)
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

    DAVA::String GetArgument(const char *argumentName)
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
};

void PrintError(DAVA::PatchFileReader::PatchError error)
{
    switch(error)
    {
        case DAVA::PatchFileReader::ERROR_NO:
            break;
        case DAVA::PatchFileReader::ERROR_EMPTY_PATCH:
            printf("ERROR_EMPTY_PATCH\n");
            break;
        case DAVA::PatchFileReader::ERROR_ORIG_READ:
            printf("ERROR_ORIG_PATH\n");
            break;
        case DAVA::PatchFileReader::ERROR_ORIG_CRC:
            printf("ERROR_ORIG_CRC\n");
            break;
        case DAVA::PatchFileReader::ERROR_NEW_WRITE:
            printf("ERROR_NEW_PATH\n");
            break;
        case DAVA::PatchFileReader::ERROR_NEW_CRC:
            printf("ERROR_NEW_CRC\n");
            break;
        case DAVA::PatchFileReader::ERROR_CANT_READ:
            printf("ERROR_CANT_READ\n");
            break;
        case DAVA::PatchFileReader::ERROR_CORRUPTED:
            printf("ERROR_CORRUPTED\n");
            break;
        case DAVA::PatchFileReader::ERROR_UNKNOWN:
        default:
            printf("ERROR_UNKNOWN\n");
            break;
    }
}

int DoPatch(DAVA::PatchFileReader *reader, const DAVA::FilePath &origBase, const DAVA::FilePath &origPath, const DAVA::FilePath &newBase, const DAVA::FilePath &newPath)
{
    int ret = 0;

    if(!reader->Apply(origBase, origPath, newBase, newPath))
    {
        PrintError(reader->GetLastError());
        ret = 1;
    }

    return ret;
}

int main(int argc, char *argv[])
{
    int ret = 0;

    ProgramOptions writeOptions;
    ProgramOptions listOptions;
    ProgramOptions applyOptions;
    ProgramOptions applyAllOptions;

    writeOptions.AddOption("-a", DAVA::VariantType(false), "Append patch to existing file.");
    writeOptions.AddOption("-nc", DAVA::VariantType(false), "Generate uncompressed patch.");
    writeOptions.AddOption("-v", DAVA::VariantType(false), "Verbose output.");
    writeOptions.AddOption("-bo", DAVA::VariantType(DAVA::String("")), "Original file base dir.");
    writeOptions.AddOption("-bn", DAVA::VariantType(DAVA::String("")), "New file base dir.");
    writeOptions.AddArgument("OriginalFile");
    writeOptions.AddArgument("NewFile");
    writeOptions.AddArgument("PatchFile");

    listOptions.AddArgument("PatchFile");
    listOptions.AddOption("-v", DAVA::VariantType(false), "Verbose output.");

    applyOptions.AddOption("-i", DAVA::VariantType(DAVA::String("")), "Input original file.");
    applyOptions.AddOption("-o", DAVA::VariantType(DAVA::String("")), "Output file or directory.");
    applyOptions.AddOption("-bo", DAVA::VariantType(DAVA::String("")), "Original file base dir.");
    applyOptions.AddOption("-bn", DAVA::VariantType(DAVA::String("")), "New file base dir.");
    applyOptions.AddOption("-v", DAVA::VariantType(false), "Verbose output.");
    applyOptions.AddArgument("PatchIndex");
    applyOptions.AddArgument("PatchFile");

    applyAllOptions.AddArgument("PatchFile");
    applyAllOptions.AddOption("-bo", DAVA::VariantType(DAVA::String("")), "Original file base dir.");
    applyAllOptions.AddOption("-bn", DAVA::VariantType(DAVA::String("")), "New file base dir.");
    applyAllOptions.AddOption("-t", DAVA::VariantType(false), "Truncate patch file, when applying it.");
    applyAllOptions.AddOption("-v", DAVA::VariantType(false), "Verbose output.");

    new DAVA::FileSystem;

    DAVA::FileSystem::Instance()->SetDefaultDocumentsDirectory();
    DAVA::FileSystem::Instance()->CreateDirectory(DAVA::FileSystem::Instance()->GetCurrentDocumentsDirectory(), true);

    bool paramsOk = false;
    if(argc > 1)
    {
        //DAVA::String command = DAVA::CommandLineParser::Instance()->GetCommand(0);

        const char *command = argv[1];
        if(0 == strcmp(command, "write"))
        {
            paramsOk = writeOptions.Parse(argc, argv, 2);
            if(paramsOk)
            {
                DAVA::PatchFileWriter::WriterMode writeMode = DAVA::PatchFileWriter::WRITE;
                BSType bsType = BS_ZLIB;

                if(writeOptions.GetOption("-a").AsBool())
                {
                    writeMode = DAVA::PatchFileWriter::APPEND;
                }

                if(writeOptions.GetOption("-nc").AsBool())
                {
                    bsType = BS_PLAIN;
                }

                DAVA::FilePath origPath = writeOptions.GetArgument("OriginalFile");
                DAVA::FilePath newPath = writeOptions.GetArgument("NewFile");
                DAVA::FilePath patchPath = writeOptions.GetArgument("PatchFile");

                DAVA::FilePath origBasePath = writeOptions.GetOption("-bo").AsString();
                DAVA::FilePath newBasePath = writeOptions.GetOption("-bn").AsString();
                bool verbose = writeOptions.GetOption("-v").AsBool();

                if(!origBasePath.IsEmpty() && !origBasePath.IsDirectoryPathname())
                {
                    printf("Bad original base dir\n");
                    ret = 1;
                }

                if(!newBasePath.IsEmpty() && !newBasePath.IsDirectoryPathname())
                {
                    printf("Bad new base dir\n");
                    ret = 1;
                }

                if(0 == ret)
                {
                    DAVA::PatchFileWriter patchWriter(patchPath, writeMode, bsType, verbose);
                    if(!patchWriter.Write(origBasePath, origPath, newBasePath, newPath))
                    {
                        printf("Error, while creating patch [%s] -> [%s].\n", origPath.GetRelativePathname().c_str(), newPath.GetRelativePathname().c_str());
                        ret = 1;
                    }
                }
            }
        }
        else if(0 == strcmp(command, "list"))
        {
            paramsOk = listOptions.Parse(argc, argv, 2);
            if(paramsOk)
            {
                DAVA::uint32 index = 0;
                DAVA::FilePath patchPath = listOptions.GetArgument("PatchFile");
                bool verbose = listOptions.GetOption("-v").AsBool();

                if(!patchPath.Exists())
                {
                    printf("No such file %s\n", patchPath.GetAbsolutePathname().c_str());
                    ret = 1;
                }

                if(0 == ret)
                {
                    DAVA::PatchFileReader patchReader(patchPath);
                    patchReader.ReadFirst();

                    const DAVA::PatchInfo *patchInfo = patchReader.GetCurInfo();
                    while(NULL != patchInfo)
                    {
                        DAVA::String origStr = patchInfo->origPath;
                        DAVA::String newStr = patchInfo->newPath;

                        if(origStr.empty()) origStr = "[]";
                        if(newStr.empty()) newStr = "[]";

                        printf("  %4u: %s --> %s\n", index, origStr.c_str(), newStr.c_str());
                        if(verbose)
                        {
                            printf("     OrigSize: %u byte; OrigCRC: 0x%X\n", patchInfo->origSize, patchInfo->origCRC);
                            printf("     NewSize: %u byte; NewCRC: 0x%X\n\n", patchInfo->newSize, patchInfo->newCRC);
                        }

                        patchReader.ReadNext();
                        patchInfo = patchReader.GetCurInfo();
                        index++;
                    }
                    
                    PrintError(patchReader.GetLastError());
                }
            }
        }
        else if(0 == strcmp(command, "apply"))
        {
            paramsOk = applyOptions.Parse(argc, argv, 2);
            if(paramsOk)
            {
                DAVA::uint32 indexToApply = 0;
                DAVA::FilePath patchPath = applyOptions.GetArgument("PatchFile");
                DAVA::String patchIndex = applyOptions.GetArgument("PatchIndex");
                DAVA::FilePath origPath = applyOptions.GetOption("-i").AsString();
                DAVA::FilePath newPath = applyOptions.GetOption("-o").AsString();
                DAVA::FilePath origBasePath = applyOptions.GetOption("-bo").AsString();
                DAVA::FilePath newBasePath = applyOptions.GetOption("-bn").AsString();
                bool verbose = applyOptions.GetOption("-v").AsBool();

                if(!patchPath.Exists())
                {
                    printf("No such file %s\n", patchPath.GetAbsolutePathname().c_str());
                    ret = 1;
                }

                if(!origBasePath.IsEmpty() && !origBasePath.IsDirectoryPathname())
                {
                    printf("Bad original base dir\n");
                    ret = 1;
                }

                if(!newBasePath.IsEmpty() && !newBasePath.IsDirectoryPathname())
                {
                    printf("Bad new base dir\n");
                    ret = 1;
                }

                if(0 == ret)
                {
                    if(sscanf(patchIndex.c_str(), "%u", &indexToApply) > 0)
                    {
                        DAVA::uint32 index = 0;
                        DAVA::PatchFileReader patchReader(patchPath, verbose);
                        bool indexFound = false;

                        patchReader.ReadFirst();
                        const DAVA::PatchInfo *patchInfo = patchReader.GetCurInfo();
                        while(NULL != patchInfo)
                        {
                            if(index == indexToApply)
                            {
                                if(origPath.IsEmpty())
                                {
                                    origPath = patchInfo->origPath;
                                }

                                if(newPath.IsEmpty())
                                {
                                    newPath = patchInfo->newPath;
                                }
                                else if(newPath.IsDirectoryPathname())
                                {
                                    newPath += patchInfo->newPath;
                                }

                                indexFound = true;
                                ret = DoPatch(&patchReader, origBasePath, origPath, newBasePath, newPath);
                                break;
                            }

                            patchReader.ReadNext();
                            patchInfo = patchReader.GetCurInfo();
                            index++;
                        }

                        if(!indexFound)
                        {
                            printf("No such index - %u\n", indexToApply);
                            ret = 1;
                        }
                    }
                }
            }
        }
        else if(0 == strcmp(command, "apply-all"))
        {
            paramsOk = applyAllOptions.Parse(argc, argv, 2);
            if(paramsOk)
            {
                DAVA::FilePath patchPath = applyAllOptions.GetArgument("PatchFile");
                DAVA::FilePath origBasePath = applyAllOptions.GetOption("-bo").AsString();
                DAVA::FilePath newBasePath = applyAllOptions.GetOption("-bn").AsString();
                bool verbose = applyAllOptions.GetOption("-v").AsBool();
                bool truncate = applyAllOptions.GetOption("-t").AsBool();

                if(!patchPath.Exists())
                {
                    printf("No such file %s\n", patchPath.GetAbsolutePathname().c_str());
                    ret = 1;
                }

                if(!origBasePath.IsEmpty() && !origBasePath.IsDirectoryPathname())
                {
                    printf("Bad original base dir\n");
                    ret = 1;
                }

                if(!newBasePath.IsEmpty() && !newBasePath.IsDirectoryPathname())
                {
                    printf("Bad new base dir\n");
                    ret = 1;
                }

                if(0 == ret)
                {
                    DAVA::PatchFileReader patchReader(patchPath, verbose);

                    // go from last patch to the first one and truncate applied patch
                    if(truncate)
                    {
                        patchReader.ReadLast();
                        const DAVA::PatchInfo *patchInfo = patchReader.GetCurInfo();
                        while(NULL != patchInfo && 0 == ret)
                        {
                            ret = DoPatch(&patchReader, origBasePath, DAVA::FilePath(), newBasePath, DAVA::FilePath());

                            patchReader.Truncate();
                            patchReader.ReadPrev();
                            patchInfo = patchReader.GetCurInfo();
                        }
                    }
                    // go from first to the last once and apply patch
                    else
                    {
                        patchReader.ReadFirst();
                        const DAVA::PatchInfo *patchInfo = patchReader.GetCurInfo();
                        while(NULL != patchInfo && 0 == ret)
                        {
                            ret = DoPatch(&patchReader, origBasePath, DAVA::FilePath(), newBasePath, DAVA::FilePath());

                            patchReader.ReadNext();
                            patchInfo = patchReader.GetCurInfo();
                        }
                    }

                    PrintError(patchReader.GetLastError());
                }
            }
        }
    }

    if(!paramsOk)
    {
        printf("Usage: ResourcePatcher <command>\n");
        printf("\n Commands: write, list, apply, apply-all\n\n");
        writeOptions.PrintUsage("  write"); printf("\n");
        listOptions.PrintUsage("  list"); printf("\n");
        applyOptions.PrintUsage("  apply"); printf("\n");
        applyAllOptions.PrintUsage("  apply-all"); printf("\n");
    }

    DAVA::FileSystem::Instance()->Release();
    return ret;
}
