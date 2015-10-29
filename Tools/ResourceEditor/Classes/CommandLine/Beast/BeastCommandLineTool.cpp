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

#include "CommandLine/Beast/BeastCommandLineTool.h"

#include "Scene/SceneEditor2.h"
#include "Commands2/BeastAction.h"

using namespace DAVA;

#if defined (__DAVAENGINE_BEAST__)

namespace OptionName
{
static const String File = "-file";
static const String Output = "-output";
}

BeastCommandLineTool::BeastCommandLineTool()
    : CommandLineTool("-beast")
{
    options.AddOption(OptionName::File, VariantType(String("")), "Full pathname of scene for beasting");
    options.AddOption(OptionName::Output, VariantType(String("")), "Full path for output folder for beasting");
}

void BeastCommandLineTool::ConvertOptionsToParamsInternal()
{
    scenePathname = options.GetOption(OptionName::File).AsString();
    outputPath = options.GetOption(OptionName::Output).AsString();
}

bool BeastCommandLineTool::InitializeInternal()
{
    if (scenePathname.IsEmpty() || !scenePathname.IsEqualToExtension(".sc2"))
    {
        AddError("Scene was not selected");
        return false;
    }

    if (outputPath.IsEmpty())
    {
        AddError("Out folder was not selected");
        return false;
    }
    outputPath.MakeDirectoryPathname();

    return true;
}

void BeastCommandLineTool::ProcessInternal()
{
    ScopedPtr<SceneEditor2> scene(new SceneEditor2());
    if (scene->Load(scenePathname))
    {
        scene->Update(0.1f);
        scene->Exec(new BeastAction(scene, outputPath, BeastProxy::MODE_LIGHTMAPS, nullptr));
        scene->Save();
    }
    RenderObjectsFlusher::Flush();
}

DAVA::FilePath BeastCommandLineTool::GetQualityConfigPath() const
{
    return CreateQualityConfigPath(scenePathname);
}


#endif //#if defined (__DAVAENGINE_BEAST__)

