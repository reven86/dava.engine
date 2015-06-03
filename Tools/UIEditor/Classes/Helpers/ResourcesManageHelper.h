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


#ifndef UIEditor_ResourcesManageHelper_h
#define UIEditor_ResourcesManageHelper_h

#include "Base/BaseTypes.h"
#include "Render/2D/FTFont.h"

class QString;
class QStringList;

namespace DAVA
{

class ResourcesManageHelper{

public:
    // Helper to work with resources
    static QString GetFontRelativePath(const QString& resourceFileName, bool graphicsFont = false);
    static QString GetFontAbsolutePath(const QString& resourceFileName, bool graphicsFont = false);
    static QString GetResourceRelativePath(const QString& resourceAbsolutePath, bool keepFileExtension = false);
    static QStringList GetFontsList();
	// Functions to work with help contents and button background image
	static QString GetButtonBackgroundImagePath();
	// Get path to documentation folder
	static QString GetDocumentationPath();
	// Get graphics font sprite file path
	static QString GetGraphicsFontPath(Font* font);
	
	// Working directory
	static QString GetDefaultDirectory();
	// Resource directory
	static QString GetResourceRootDirectory();
	// Get sprites data directory
	static QString GetSpritesDirectory();
	// Get font sprites data directory
	static QString GetFontSpritesDirectory();
	// Get sprites datasource directory
	static QString GetSpritesDatasourceDirectory();
	// Get font sprites datasource directory
	static QString GetFontSpritesDatasourceDirectory();
	// Get resource folder path
	static QString GetResourceFolder(const QString& resourcePath);
	// Validate if selected resource is located inside project 
	static bool ValidateResourcePath(const QString& resourcePath);
	
	// Get working directory path of last opened sprite
	static QString GetDefaultSpritesPath(const QString& currentSpritePath);
	// Get working directory path of last opened font sprite
	static QString GetDefaultFontSpritesPath(const QString& currentSpritePath);
	
	// Get project title string
	static QString GetProjectTitle();
	// Get project title string
	static QString GetProjectTitle(const QString& projectFilePath);
	// Get the folder wehere project is located
	static QString GetProjectPath();
	// Get folder with resource data for specified project path
	static QString GetDataPath(const QString& projectPath);
	// Path to palform specific files for specified project path
	static QString GetPlatformRootPath(const QString& projectPath);
	// Project file path for specified project path
	static QString GetProjectFilePath(const QString& projectPath);
	// Initialize project internal resources
	// Do not use this function elsewhere than during application init!!!
	static void InitInternalResources();
	// Show error message
	static void ShowErrorMessage(const QString& messageParam);
	// Reverse slash order in string
	static QString ConvertPathToUnixStyle(const QString& inputString);

private:
	static QString buttonBackgroundImagePath;
	static QString projectTitle;
};
};

#endif