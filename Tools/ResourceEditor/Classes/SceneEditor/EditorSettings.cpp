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



#include "EditorSettings.h"

#include "ControlsFactory.h"

#include "Render/Highlevel/Heightmap.h"


EditorSettings::EditorSettings()
{
    settings = new KeyedArchive();
    
    settings->Load("~doc:/ResourceEditorOptions.archive");
	ApplyOptions();
}
    
EditorSettings::~EditorSettings()
{
    SafeRelease(settings);
}


KeyedArchive *EditorSettings::GetSettings()
{
    return settings;
}

void EditorSettings::Save()
{
    settings->Save("~doc:/ResourceEditorOptions.archive");
}

void EditorSettings::ApplyOptions()
{
    if(RenderManager::Instance())
    {
        RenderManager::Instance()->GetOptions()->SetOption(RenderOptions::IMPOSTERS_ENABLE, settings->GetBool("enableImposters", true));
    }
}


void EditorSettings::SetDataSourcePath(const FilePath &datasourcePath)
{
    settings->SetString("3dDataSourcePath", datasourcePath.GetAbsolutePathname());
}

FilePath EditorSettings::GetDataSourcePath()
{
    return FilePath(settings->GetString("3dDataSourcePath", "/"));
}

void EditorSettings::SetProjectPath(const FilePath &projectPath)
{
    settings->SetString(String("ProjectPath"), projectPath.GetAbsolutePathname());
}

FilePath EditorSettings::GetProjectPath()
{
    return FilePath(settings->GetString(String("ProjectPath"), String("")));
}

FilePath EditorSettings::GetParticlesConfigsPath()
{
	return GetProjectPath() + "Data/Configs/Particles/";
}

float32 EditorSettings::GetCameraSpeed()
{
    int32 index = settings->GetInt32("CameraSpeedIndex", 0);
    return GetCameraSpeed(index);
}

void EditorSettings::SetCameraSpeedIndex(int32 camSpeedIndex)
{
    DVASSERT(camSpeedIndex >= 0 && camSpeedIndex < 4);

    settings->SetInt32("CameraSpeedIndex", camSpeedIndex);
    Save();
}

void EditorSettings::SetCameraSpeed(int32 camSpeedIndex, float32 speed)
{
    DVASSERT(camSpeedIndex >= 0 && camSpeedIndex < 4);
    settings->SetFloat(Format("CameraSpeedValue_%d", camSpeedIndex), speed);
}

float32 EditorSettings::GetCameraSpeed(int32 camSpeedIndex)
{
    DVASSERT(camSpeedIndex >= 0 && camSpeedIndex < 4);
    
    static const float32 speedConst[] = {35, 100, 250, 400};
    return settings->GetFloat(Format("CameraSpeedValue_%d", camSpeedIndex), speedConst[camSpeedIndex]);
}


int32 EditorSettings::GetScreenWidth()
{
    return settings->GetInt32("ScreenWidth", 1024);
}

void EditorSettings::SetScreenWidth(int32 width)
{
    settings->SetInt32("ScreenWidth", width);
}

int32 EditorSettings::GetScreenHeight()
{
    return settings->GetInt32("ScreenHeight", 690);
}

void EditorSettings::SetScreenHeight(int32 height)
{
    settings->SetInt32("ScreenHeight", height);
}

String EditorSettings::GetLanguage()
{
    return settings->GetString("Language", "en");
}

void EditorSettings::SetLanguage(const String &language)
{
    settings->SetString("Language", language);
}

bool EditorSettings::GetShowOutput()
{
    return settings->GetBool("ShowOutput", true);
}

void EditorSettings::SetShowOuput(bool showOutput)
{
    settings->SetBool("ShowOutput", showOutput);
}

int32 EditorSettings::GetLeftPanelWidth()
{
    return settings->GetInt32("LeftPanelWidth", ControlsFactory::LEFT_PANEL_WIDTH);
}

void EditorSettings::SetLeftPanelWidth(int32 width)
{
    settings->SetInt32("LeftPanelWidth", width);
}

int32 EditorSettings::GetRightPanelWidth()
{
    return settings->GetInt32("RightPanelWidth", ControlsFactory::RIGHT_PANEL_WIDTH);
}
void EditorSettings::SetRightPanelWidth(int32 width)
{
    settings->SetInt32("RightPanelWidth", width);
}

float32 EditorSettings::GetLodLayerDistance(int32 layerNum)
{
    DVASSERT(0 <= layerNum && layerNum < LodComponent::MAX_LOD_LAYERS);
    return settings->GetFloat(Format("LODLayer_%d", layerNum), LodComponent::GetDefaultDistance(layerNum));
}

void EditorSettings::SetLodLayerDistance(int32 layerNum, float32 distance)
{
    DVASSERT(0 <= layerNum && layerNum < LodComponent::MAX_LOD_LAYERS);
    return settings->SetFloat(Format("LODLayer_%d", layerNum), distance);
}

int32 EditorSettings::GetLastOpenedCount()
{
    return settings->GetInt32("LastOpenedFilesCount", 0);
}

String EditorSettings::GetLastOpenedFile(int32 index)
{
    int32 count = GetLastOpenedCount();
    DVASSERT((0 <= index) && (index < count));
    
    return settings->GetString(Format("LastOpenedFile_%d", index), "");
}

void EditorSettings::AddLastOpenedFile(const FilePath & pathToFile)
{
    Vector<String> filesList;
    
    int32 count = GetLastOpenedCount();
    for(int32 i = 0; i < count; ++i)
    {
        String path = settings->GetString(Format("LastOpenedFile_%d", i), "");
        if(path != pathToFile.GetAbsolutePathname())
        {
            filesList.push_back(path);
        }
    }

    filesList.insert(filesList.begin(), pathToFile.GetAbsolutePathname());
    count = 0;
    for(;(count < (int32)filesList.size()) && (count < RESENT_FILES_MAX_COUNT); ++count)
    {
        settings->SetString(Format("LastOpenedFile_%d", count), filesList[count]);
    }
    settings->SetInt32("LastOpenedFilesCount", count);
    
    Save();
}

void EditorSettings::SetDrawGrid(bool drawGrid)
{
    settings->SetBool("DrawGrid", drawGrid);
}

bool EditorSettings::GetDrawGrid()
{
    return settings->GetBool("DrawGrid", true);
}

void EditorSettings::SetEnableImposters(bool enableImposters)
{
	RenderManager::Instance()->GetOptions()->SetOption(RenderOptions::IMPOSTERS_ENABLE, enableImposters);
	settings->SetBool("enableImposters", enableImposters);
}

bool EditorSettings::GetEnableImposters()
{
	return settings->GetBool("enableImposters", true);
}

eGPUFamily EditorSettings::GetTextureViewGPU()
{
    return (eGPUFamily)settings->GetInt32(String("TextureViewGPU"), GPU_UNKNOWN);
}
void EditorSettings::SetTextureViewGPU(int32 gpu)
{
    settings->SetInt32(String("TextureViewGPU"), gpu);
	Save();
}


void EditorSettings::SetMaterialsColor(const Color &ambient, const Color &diffuse, const Color &specular)
{
    Vector4 ambientVector = ToVector4(ambient);
    Vector4 diffuseVector = ToVector4(diffuse);
    Vector4 specularVector = ToVector4(specular);
	settings->SetVector4(String("materials.ambient"), ambientVector);
	settings->SetVector4(String("materials.diffuse"), diffuseVector);
	settings->SetVector4(String("materials.specular"), specularVector);
}

Color EditorSettings::GetMaterialAmbientColor()
{
	Vector4 colorVect = settings->GetVector4(String("materials.ambient"), ToVector4(Color::White));
	return ToColor(colorVect);
}
Color EditorSettings::GetMaterialDiffuseColor()
{
	Vector4 colorVect = settings->GetVector4(String("materials.diffuse"), ToVector4(Color::White));
	return ToColor(colorVect);
}

Color EditorSettings::GetMaterialSpecularColor()
{
	Vector4 colorVect = settings->GetVector4(String("materials.specular"), ToVector4(Color::White));
	return ToColor(colorVect);
}

Vector4 EditorSettings::ToVector4(const Color &color)
{
	Vector4 vect(color.r, color.g, color.b, color.a);
	return vect;
}

Color EditorSettings::ToColor(const Vector4 &colorVector)
{
	Color color(colorVector.x, colorVector.y, colorVector.z, colorVector.w);
	return color;
}

String EditorSettings::GetDesignerName()
{
    return settings->GetString("DesignerName", "nobody");
}

void EditorSettings::SetDesignerName(const String &userName)
{
    settings->SetString("DesignerName", userName);
}

void EditorSettings::SetPreviewDialogEnabled(bool enabled)
{
    settings->SetBool("PreviewDialogEnabled", enabled);
	Save();
}

bool EditorSettings::GetPreviewDialogEnabled()
{
    return settings->GetBool("PreviewDialogEnabled", false);
}

