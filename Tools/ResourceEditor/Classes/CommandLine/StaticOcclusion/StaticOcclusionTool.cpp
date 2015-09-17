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


#include "StaticOcclusionTool.h"
#include "CommandLine/SceneUtils/SceneUtils.h"
#include "TexturePacker/CommandLineParser.h"
#include "Scene/SceneEditor2.h"

using namespace DAVA;

void StaticOcclusionTool::PrintUsage() const
{
    printf("\n");
    printf("-staticocclusion -build [-processfile]\n");
    printf("\twill build static occlusion for scene file\n");
    printf("\t-build - will build static occlusion\n");
    printf("\t-processfile - pathname to scene file\n");

    printf("\n");
    printf("Samples:\n");
    printf("-staticocclusion -build -processfile /Users/User/Project/DataSource/3dMaps/level.sc2\n");
}

DAVA::String StaticOcclusionTool::GetCommandLineKey() const
{
    return "-staticocclusion";
}

bool StaticOcclusionTool::InitializeFromCommandLine()
{
    commandAction = ACTION_NONE;
    
    if(CommandLineParser::CommandIsFound(String("-build")))
    {
        commandAction = ACTION_BUILD;
        scenePathname = CommandLineParser::GetCommandParam(String("-processfile"));
        if(scenePathname.IsEmpty())
        {
            errors.insert("[StaticOcclusionTool] Filename is not set");
            return false;
        }
    }
    else
    {
        errors.insert("[StaticOcclusionTool] Incorrect action");
        return false;
    }
    
    return true;
}

void StaticOcclusionTool::DumpParams() const
{ 
    Logger::Info("StaticOcclusionTool started with params:\n\tFilename: %s", scenePathname.GetStringValue().c_str());
}

void StaticOcclusionTool::Process() 
{
    if (commandAction == ACTION_BUILD)
    {
        SceneEditor2 *scene = new SceneEditor2();
        if(scene->Load(scenePathname))
        {
            scene->staticOcclusionBuildSystem->Build();
            while(scene->staticOcclusionBuildSystem->IsInBuild())
            {
                scene->Update(0.1f);
            }

            scene->Save();
        }
        SafeRelease(scene);
    	RenderObjectsFlusher::Flush();
	}
}

DAVA::FilePath StaticOcclusionTool::GetQualityConfigPath() const
{
    return CreateQualityConfigPath(scenePathname);
}

