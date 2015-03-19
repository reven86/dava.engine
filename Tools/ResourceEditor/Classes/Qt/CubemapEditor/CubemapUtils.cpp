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


#include "CUbemapEditor/CubemapUtils.h"
#include "Render/Texture.h"
#include "Qt/Settings/SettingsManager.h"
#include "Qt/Main/QtUtils.h"
#include "Project/ProjectManager.h"

#define CUBEMAPEDITOR_MAXFACES 6

static DAVA::String FACE_NAME_SUFFIX[] = {
	DAVA::String("_px"),
	DAVA::String("_nx"),
	DAVA::String("_py"),
	DAVA::String("_ny"),
	DAVA::String("_pz"),
	DAVA::String("_nz")
};

void CubemapUtils::GenerateFaceNames(const DAVA::String& baseName, DAVA::Vector<DAVA::String>& faceNames)
{
	faceNames.clear();
	
	DAVA::FilePath filePath(baseName);
	
	DAVA::String fileNameWithoutExtension = filePath.GetFilename();
	DAVA::String extension = filePath.GetExtension();
	fileNameWithoutExtension.replace(fileNameWithoutExtension.find(extension), extension.size(), "");

	for(int i = 0; i < CubemapUtils::GetMaxFaces(); ++i)
	{
		DAVA::FilePath faceFilePath = baseName;
		faceFilePath.ReplaceFilename(fileNameWithoutExtension +
									 CubemapUtils::GetFaceNameSuffix(CubemapUtils::MapUIToFrameworkFace(i)) +
									 CubemapUtils::GetDefaultFaceExtension());

		faceNames.push_back(faceFilePath.GetAbsolutePathname());
	}
}

int CubemapUtils::GetMaxFaces()
{
	return CUBEMAPEDITOR_MAXFACES;
}

const DAVA::String& CubemapUtils::GetFaceNameSuffix(int faceId)
{
	return FACE_NAME_SUFFIX[faceId];
}

const DAVA::String& CubemapUtils::GetDefaultFaceExtension()
{
    return DAVA::Texture::GetDefaultFaceExtension();
}

DAVA::FilePath CubemapUtils::GetDialogSavedPath(const DAVA::String& key, const DAVA::String& defaultValue)
{
    DAVA::VariantType settinsValue = SettingsManager::GetValue(key);
    DAVA::FilePath path = settinsValue.GetType() == DAVA::VariantType::TYPE_STRING ? settinsValue.AsString() : settinsValue.AsFilePath();
        
    DAVA::FilePath defaultPath(defaultValue);
    DAVA::FilePath projectPath = ProjectManager::Instance()->CurProjectPath();
    bool isInProject = DAVA::FilePath::ContainPath(path, projectPath);
    
	if(!path.Exists() || !isInProject)
	{
		path = defaultPath.Exists() ? defaultPath : projectPath;
		SettingsManager::SetValue(key, DAVA::VariantType(path));
	}

	return path;
}
