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


#include "CleanFolderTool.h"

#include "TexturePacker/CommandLineParser.h"

using namespace DAVA;

void CleanFolderTool::PrintUsage() const
{
    printf("\n");
    printf("-cleanfolder [-folder [directory]]\n");
    printf("\twill delete folder with files \n");
    printf("\t-folder - path for /Users/User/Project/Data/3d/ folder \n");

    printf("\n");
    printf("Sample:\n");
    printf("-cleanfolder -folder /Users/User/Project/Data/3d\n");
}

DAVA::String CleanFolderTool::GetCommandLineKey() const
{
    return "-cleanfolder";
}

bool CleanFolderTool::InitializeFromCommandLine()
{
    foldername = CommandLineParser::GetCommandParam(String("-folder"));
    if(foldername.IsEmpty())
    {
        errors.insert(String("Incorrect params for cleaning folder"));
        return false;
    }

    foldername.MakeDirectoryPathname();
    
    return true;
}

void CleanFolderTool::Process(CommandLineTool::EngineHelperCallback cb) 
{
    bool ret = FileSystem::Instance()->DeleteDirectory(foldername);
    if(!ret)
    {
        bool folderExists = FileSystem::Instance()->IsDirectory(foldername);
        if(folderExists)
        {
            errors.insert(String(Format("[CleanFolder] ret = %d, folder = %s", ret, foldername.GetAbsolutePathname().c_str())));
        }
    }
}


