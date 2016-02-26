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

#include "MaterialConfigCommands.h"

MaterialConfigModify::MaterialConfigModify(DAVA::NMaterial* material_, int id, const DAVA::String& text)
    : Command2(id, text)
    , material(material_)
{
    DVASSERT(material);
}

MaterialChangeCurrentConfig::MaterialChangeCurrentConfig(DAVA::NMaterial* material, DAVA::uint32 newCurrentConfigIndex)
    : MaterialConfigModify(material, CMDID_MATERIAL_CHANGE_CURRENT_CONFIG, "Change current material config")
    , newCurrentConfig(newCurrentConfigIndex)
{
    oldCurrentConfig = material->GetCurrConfig();
}

void MaterialChangeCurrentConfig::Undo()
{
    material->SetCurrConfig(oldCurrentConfig);
}

void MaterialChangeCurrentConfig::Redo()
{
    material->SetCurrConfig(newCurrentConfig);
}

MaterialRemoveConfig::MaterialRemoveConfig(DAVA::NMaterial* material, DAVA::uint32 configIndex_)
    : MaterialConfigModify(material, CMDID_MATERIAL_REMOVE_CONFIG, "Remove material config")
    , configIndex(configIndex_)
{
    config = material->GetConfig(configIndex);

    DAVA::uint32 configCount = material->GetConfigCount();
    DAVA::uint32 newCurrConfig = material->GetCurrConfig();
    DVASSERT(configCount > 1);
    DVASSERT(configIndex < configCount);
    if ((configIndex == newCurrConfig && configIndex == configCount - 1) ||
        configIndex < newCurrConfig)
    {
        --newCurrConfig;
    }

    changeCurrentConfigCommand.reset(new MaterialChangeCurrentConfig(material, newCurrConfig));
}

void MaterialRemoveConfig::Undo()
{
    material->InsertConfig(configIndex, config);
    changeCurrentConfigCommand->Undo();
}

void MaterialRemoveConfig::Redo()
{
    material->RemoveConfig(configIndex);
    changeCurrentConfigCommand->Redo();
}

MaterialCreateConfig::MaterialCreateConfig(DAVA::NMaterial* material, const DAVA::MaterialConfig& config_)
    : MaterialConfigModify(material, CMDID_MATERIAL_CREATE_CONFIG, "Create material config")
    , config(config_)
{
    changeCurrentConfigCommand.reset(new MaterialChangeCurrentConfig(material, material->GetConfigCount()));
}

void MaterialCreateConfig::Undo()
{
    changeCurrentConfigCommand->Undo();
    DVASSERT(configIndex != -1);
    material->RemoveConfig(configIndex);
    configIndex = -1;
}

void MaterialCreateConfig::Redo()
{
    DVASSERT(configIndex == -1);
    configIndex = material->GetConfigCount();
    material->InsertConfig(configIndex, config);
    changeCurrentConfigCommand->Redo();
}
