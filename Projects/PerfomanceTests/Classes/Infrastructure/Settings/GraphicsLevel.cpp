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

#include "GraphicsLevel.h"
#include "Platform/DeviceInfo.h"

using namespace DAVA;

#define GET_BOOL(prop) ReadBool(prop, #prop, archive, node) 
#define GET_STRING(prop) ReadString(prop, #prop, archive, node) 

GraphicsLevel::GraphicsLevel(const DAVA::String & fileName, YamlNode * node)
    :   archive(new DAVA::KeyedArchive)
{
    vegetationAnimation = false;
    stencilShadows = true;

    ReadSettings(fileName, node);
}

GraphicsLevel::~GraphicsLevel()
{
}

void ReadString(String & var, const String & key, KeyedArchive * arch, DAVA::YamlNode * node)
{
    var = arch->GetString(key, var);
    if (!node) return;
    const auto & map = node->AsMap();
    auto fit = map.find(key);
    if (fit != map.end() && fit->second->GetType() == YamlNode::TYPE_MAP)
    {
        const auto & typeMap = fit->second->AsMap();
        auto fit2 = typeMap.find("string");
        if (fit2 != typeMap.end() && fit2->second->GetType() == YamlNode::TYPE_STRING)
        {
            var = fit2->second->AsString();
        }
    }
}

void ReadBool(bool & var, const String & key, KeyedArchive * arch, DAVA::YamlNode * node)
{
    var = arch->GetBool(key, var);
    if (!node) return;
    const auto & map = node->AsMap();
    auto fit = map.find(key);
    if (fit != map.end() && fit->second->GetType() == YamlNode::TYPE_MAP)
    {
        const auto & typeMap = fit->second->AsMap();
        auto fit2 = typeMap.find("bool");
        if (fit2 != typeMap.end() && fit2->second->GetType() == YamlNode::TYPE_STRING)
        {
            var = fit2->second->AsBool();
        }
    }
}

void GraphicsLevel::ReadSettings(const DAVA::String & fileName, DAVA::YamlNode * node)
{
    archive->DeleteAllKeys();

    String path = "~res:/GraphicSettings/" + fileName;
    archive->LoadFromYamlFile(path);
    
    GET_STRING(water);
    GET_STRING(vegetation);
    GET_STRING(tree_lighting);
    GET_STRING(landscape);
    GET_STRING(static_object);
    
    GET_BOOL(vegetationAnimation);
    GET_BOOL(stencilShadows);
}


void GraphicsLevel::Activate(void)
{
    QualitySettingsSystem * qs = QualitySettingsSystem::Instance();
    qs->SetCurMaterialQuality(FastName("Water"), FastName(water));
    qs->SetCurMaterialQuality(FastName("Vegetation"), FastName(vegetation));
    qs->SetCurMaterialQuality(FastName("Spherical Harmonics"), FastName(tree_lighting));
    qs->SetCurMaterialQuality(FastName("Landscape"), FastName(landscape));
    qs->SetCurMaterialQuality(FastName("Static object"), FastName(static_object));

    qs->EnableOption(QualitySettingsSystem::QUALITY_OPTION_VEGETATION_ANIMATION, vegetationAnimation);
    qs->EnableOption(QualitySettingsSystem::QUALITY_OPTION_STENCIL_SHADOW, stencilShadows);
    qs->SetCurTextureQuality(FastName(archive->GetString("textureQuality")));
}
