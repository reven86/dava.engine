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



#ifndef __EDITOR_SETTINGS_H__
#define __EDITOR_SETTINGS_H__

#include "DAVAEngine.h"

using namespace DAVA;

class EditorSettings: public Singleton<EditorSettings>
{
    
public: 
    
    static const int32 RESENT_FILES_MAX_COUNT = 5;
    
public:
    EditorSettings();
    virtual ~EditorSettings();

    KeyedArchive *GetSettings();
    void Save();

	void ApplyOptions();

    void SetProjectPath(const FilePath &projectPath);
    FilePath GetProjectPath();
    
    void SetDataSourcePath(const FilePath &datasourcePath);
    FilePath GetDataSourcePath();
    
    float32 GetCameraSpeed();
    void SetCameraSpeedIndex(int32 camSpeedIndex);//0 - 4
    void SetCameraSpeed(int32 camSpeedIndex, float32 speed);
    float32 GetCameraSpeed(int32 camSpeedIndex);
    
    int32 GetScreenWidth();
    void SetScreenWidth(int32 width);
    
    int32 GetScreenHeight();
    void SetScreenHeight(int32 height);
    
    String GetLanguage();
    void SetLanguage(const String &language);
    
    bool GetShowOutput();
    void SetShowOuput(bool showOutput);
    
    int32 GetLeftPanelWidth();
    void SetLeftPanelWidth(int32 width);

    int32 GetRightPanelWidth();
    void SetRightPanelWidth(int32 width);

    float32 GetLodLayerDistance(int32 layerNum);
    void SetLodLayerDistance(int32 layerNum, float32 distance);

    int32 GetLastOpenedCount();
    String GetLastOpenedFile(int32 index);
    void AddLastOpenedFile(const FilePath & pathToFile);
    
    void SetDrawGrid(bool drawGrid);
    bool GetDrawGrid();

	void SetEnableImposters(bool enableImposters);
	bool GetEnableImposters();
    
    void SetTextureViewGPU(int32 gpu);

	String GetDesignerName();
	void SetDesignerName(const String &userName);
    
	void SetMaterialsColor(const Color &ambient, const Color &diffuse, const Color &specular);
	Color GetMaterialAmbientColor();
	Color GetMaterialDiffuseColor();
	Color GetMaterialSpecularColor();

	FilePath GetParticlesConfigsPath();
    
//     bool GetShowEditorCamerLight();
//     void SetShowEditorCamerLight(bool show);
    
    void SetPreviewDialogEnabled(bool enabled);
    bool GetPreviewDialogEnabled();
    
protected:

	Vector4 ToVector4(const Color &color);
	Color ToColor(const Vector4 &colorVector);

    KeyedArchive *settings;
};



#endif // __EDITOR_SETTINGS_H__