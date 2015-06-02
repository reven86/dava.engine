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
#include "ResourcesManageHelper.h"
#include <QString>

static const Color DEFAULT_BACKGROUND_FRAME_COLOR(0.2f, 0.2f, 0.2f, 1.0f);

EditorSettings::EditorSettings()
{
    settings = new KeyedArchive();    
    settings->Load("~doc:/UIEditorOptions.archive");
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
    settings->Save("~doc:/UIEditorOptions.archive");
}

void EditorSettings::SetProjectPath(const String &projectPath)
{
	settings->SetString(String("ProjectPath"), projectPath);
	Save();
}

String EditorSettings::GetProjectPath()
{
	return settings->GetString(String("ProjectPath"), String(""));
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

void EditorSettings::AddLastOpenedFile(const String & pathToFile)
{
    Vector<String> filesList;
    
	// Put all slash symbols to Unix style
	QString normalizedPath = ResourcesManageHelper::ConvertPathToUnixStyle(QString::fromStdString(pathToFile));
	String _pathToFile = normalizedPath.toStdString();

    int32 count = GetLastOpenedCount();

    for(int32 i = 0; i < count; ++i)
    {
        String path = settings->GetString(Format("LastOpenedFile_%d", i), "");
        if(path == _pathToFile)
        {
            return;
        }
        filesList.push_back(path);
    }
    
    filesList.insert(filesList.begin(), _pathToFile);
    count = 0;
    for(;(count < (int32)filesList.size()) && (count < RECENT_FILES_COUNT); ++count)
    {
        settings->SetString(Format("LastOpenedFile_%d", count), filesList[count]);
    }
    settings->SetInt32("LastOpenedFilesCount", count);
    
    Save();
}

void EditorSettings::SetUIEditorVersion(const String& editorVersion)
{
	settings->SetString("editor.version", editorVersion);
	Save();
}

String EditorSettings::GetUIEditorVersion()
{
	return settings->GetString("editor.version");
}

void EditorSettings::SetPixelized(bool value)
{
    settings->SetBool("editor.pixelized", value);
	Save();
}

bool EditorSettings::IsPixelized()
{
    return settings->GetBool("editor.pixelized", true);
}

Color EditorSettings::GetCurrentBackgroundFrameColor() const
{
    return GetColor("editor.currentBackgroundFrameColor", DEFAULT_BACKGROUND_FRAME_COLOR);
}

void EditorSettings::SetCurrentBackgroundFrameColor(const Color& color)
{
    SetColor("editor.currentBackgroundFrameColor", color);
}

Color EditorSettings::GetCustomBackgroundFrameColor() const
{
    return GetColor("editor.customBackgroundFrameColor", DEFAULT_BACKGROUND_FRAME_COLOR);
}

void EditorSettings::SetCustomBackgroundFrameColor(const Color& color)
{
    SetColor("editor.customBackgroundFrameColor", color);
}

Color EditorSettings::GetColor(const String& colorName, const Color& defaultColor) const
{
    Vector4 defaultValue(defaultColor.r, defaultColor.g, defaultColor.b, defaultColor.a);
    Vector4 colorValues = settings->GetVector4(colorName, defaultValue);
    return Color(colorValues.x, colorValues.y, colorValues.z, colorValues.w);
}

void EditorSettings::SetColor(const String& colorName, const Color& color)
{
    Vector4 colorValues(color.r, color.g, color.b, color.a);
    settings->SetVector4(colorName, colorValues);
    Save();
}