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


#include "BeastCommandLineTool.h"
#include "CommandLine/SceneUtils/SceneUtils.h"
#include "CommandLine/CommandLineParser.h"
#include "Scene/SceneEditor2.h"
#include "Scene/SceneHelper.h"
#include "Commands2/BeastAction.h"
#include "Qt/Settings/SettingsManager.h"

using namespace DAVA;

#if defined (__DAVAENGINE_BEAST__)

BeastCommandLineTool::BeastCommandLineTool()
	:	CommandLineTool()
{
}

void BeastCommandLineTool::PrintUsage() const
{
    printf("\n");
    printf("-beast -file <file> -output <output_path>\n");
    printf("\twill beast scene file and place output to output_path\n");
    printf("\t-file - full pathname of scene for beasting\n");
    printf("\t-output - full path for output of beasting\n");
    printf("\n");
    printf("Samples:\n");
    printf("-beast -file /Projects/WOT/wot.blitz/DataSource/3d/Maps/karelia/karelia.sc2 -output /Projects/WOT/wot.blitz/DataSource/3d/Maps/karelia/lightmap\n");

}

DAVA::String BeastCommandLineTool::GetCommandLineKey() const
{
    return "-beast";
}

bool BeastCommandLineTool::InitializeFromCommandLine()
{
    scenePathname = CommandLineParser::GetCommandParam(String("-file"));
    outputPath = CommandLineParser::GetCommandParam( String( "-output" ) );
    if (scenePathname.IsEmpty() || outputPath.IsEmpty())
    {
        errors.insert(String("Incorrect params for beasting of the scene"));
        return false;
    }

    if(!scenePathname.IsEqualToExtension(".sc2"))
    {
        errors.insert(String("Wrong pathname. Need path ot *.sc2"));
        return false;
    }
    
    return true;
}

void BeastCommandLineTool::Process() 
{
	SceneEditor2 *scene = new SceneEditor2();
	if(scene->Load(scenePathname))
	{
        scene->Update(0.1f);
        scene->Exec(new BeastAction( scene, outputPath, BeastProxy::MODE_LIGHTMAPS, NULL ));
		scene->Save();
	}
	SafeRelease(scene);
    RenderObjectsFlusher::Flush();
}

const DAVA::FilePath & BeastCommandLineTool::GetScenePathname() const
{
    return scenePathname;
}

DAVA::FilePath BeastCommandLineTool::GetQualityConfigPath() const
{
    return CreateQualityConfigPath(scenePathname);
}


#endif //#if defined (__DAVAENGINE_BEAST__)

