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


#include "ResourcesManageHelper.h"
//
#include "EditorSettings.h"
#include "StringUtils.h"
#include "FileSystem/FileSystem.h"
#include "StringConstants.h"
#include "DAVAEngine.h"

#include <QApplication>
#include <QString>
#include <QStringList>
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>
#include "UI/mainwindow.h"
#include "Project/Project.h"


/* 
	Project folders structure:
	
	$project - absolute path to project directory.
	$project/Data/UI/ui.uieditor - path to project file. Each project can have only one project file.
	$project/Data/UI/ - path to platform root folder. It this folders all available platfroms are stored.
	
	$project/DataSource/ - root directory for all unpacked resources.
	$project/DataSource/Gfx/ - path to unpacked sprites (usually in PSD format).
	$project/DataSource/Gfx/Fonts/ - path to unpacked graphics fonts sprites

	$project/Data/ - root directory for all project resources (fonts, sprites etc.)
	$project/Data/Gfx/ - path to packed sprites, which can be used in project.
	$project/Data/Fonts/ - path to true type fonts.
	$project/Data/Fontdef/ - path to graphic fonts definition files.
	$project/DAta/Gfx/Fonts/ - path to packed graphic fonts sprites.
*/

using namespace DAVA;

namespace
{
// True type fonts resource folder path
const String FONTS_RES_PATH("~res:/Fonts/");
// Graphics fonts definition resource folder path
const String GRAPHICS_FONTS_RES_PATH("~res:/Fonts/");
// Resource folder header
const QString RES_HEADER = "~res:";
// Button background image path
const QString BACKGROUND_IMAGE_PATH("~res:/Images/buttonBg.png");
// Documentation path.
const QString DOCUMENTATION_PATH = "~doc:/UIEditorHelp/";

// Additional text constants
const QString GFX = "/Gfx/";
const QString FONTS = "/Fonts/";
const QString UI = "/UI/";
// Project DATASOURCE folder
const QString PROJECT_DATASOURCE = "%1/DataSource";
// Project DATA folder
const QString PROJECT_DATA = "%1/Data";
// Platform directory path
const QString PROJECT_PLATFORM_PATH = PROJECT_DATA + "/UI/";
// Project file path
const QString PROJECT_FILE_PATH = "%1/ui.uieditor";
// Project GFX folder for sprites psd files
const QString PROJECT_DATASOURCE_GFX = PROJECT_DATASOURCE + GFX;
// Project GFX folder for graphics fonts sprites psd files
const QString PROJECT_DATASOURCE_GRAPHICS_FONTS = PROJECT_DATASOURCE_GFX + FONTS;
// Project converted sprites folder
const QString PROJECT_DATA_GFX = PROJECT_DATA + GFX;
// Project converted graphics fonts sprites folder
const QString PROJECT_DATA_GRAPHICS_FONTS = PROJECT_DATA_GFX + FONTS;
// Default project title
const QString PROJECT_TITLE = "QuickEd";

// Resource wrong location error message
const QString RES_WRONG_LOCATION_ERROR_MESSAGE = "Resource %1 is not located inside project 'Data' folder. It can't be linked with project or control!";

//Available fonts extensions
const QStringList FONTS_EXTENSIONS_FILTER = (QStringList() << "*.ttf" << "*.otf" << "*.fon" << "*.fnt" << "*.def" << "*.df");

}
QString ResourcesManageHelper::projectTitle;

QString ResourcesManageHelper::projectPath;

QString ResourcesManageHelper::GetFontAbsolutePath(const QString& resourceFileName, bool graphicsFont)
{	
    QString fontPath = graphicsFont ? QString::fromStdString(FilePath(GRAPHICS_FONTS_RES_PATH).GetAbsolutePathname())
									: QString::fromStdString(FilePath(FONTS_RES_PATH).GetAbsolutePathname());
    fontPath += resourceFileName;
	
    return fontPath;    
}

QString ResourcesManageHelper::GetFontRelativePath(const QString& resourceFileName, bool graphicsFont)
{
    QString fontPath = graphicsFont ? QString::fromStdString(FilePath(GRAPHICS_FONTS_RES_PATH).GetAbsolutePathname())
									: QString::fromStdString(FilePath(FONTS_RES_PATH).GetAbsolutePathname());
    fontPath += resourceFileName;
	
    return fontPath;
}

bool ResourcesManageHelper::ValidateResourcePath(const QString& resourcePath)
{
	const QString& resourceFolder = GetResourceRootDirectory();
	// Check if given resource is located inside resource folder
	return resourcePath.contains(resourceFolder);
}

QString ResourcesManageHelper::GetGraphicsFontPath(Font* font)
{
    if (font && (font->GetFontType() == Font::TYPE_GRAPHICAL))
    {
		GraphicsFont *gFont = dynamic_cast<GraphicsFont*>(font);
		// Get graphics font sprite if it's available
        Sprite *fontSprite = gFont->GetFontSprite();
        if (fontSprite)
        {
			// Save graphics font sprite path
        	return QString::fromStdString(fontSprite->GetRelativePathname().GetAbsolutePathname());
    	}
	}

	return QString();
}

QString ResourcesManageHelper::GetDefaultSpritesPath(const QString& currentSpritePath)
{
	// If sprite is already set - we should use its directory as default for file dialog
	if (!currentSpritePath.isEmpty() && currentSpritePath.compare(StringConstants::NO_SPRITE_IS_SET) != 0)
	{
		FilePath spriteAbsolutePath(currentSpritePath.toStdString() + ".txt");
		QFileInfo fileInfo(QString::fromStdString(spriteAbsolutePath.GetAbsolutePathname()));
		return fileInfo.absoluteDir().absolutePath();
	}

	return GetSpritesDirectory();
}

QString ResourcesManageHelper::GetDefaultFontSpritesPath(const QString& currentSpritePath)
{
	// If sprite is already set - we should use its directory as default for file dialog
	if (!currentSpritePath.isEmpty() && currentSpritePath.compare(StringConstants::NO_SPRITE_IS_SET) != 0)
	{
		FilePath spriteAbsolutePath(currentSpritePath.toStdString());
		QFileInfo fileInfo(QString::fromStdString(spriteAbsolutePath.GetAbsolutePathname()));
		return fileInfo.absoluteDir().absolutePath();
	}
	
	return GetFontSpritesDirectory();
}

QString ResourcesManageHelper::GetResourceRelativePath(const QString& resourceAbsolutePath, bool keepFileExtension)
{
    QFileInfo fileInfo(resourceAbsolutePath);
    QString processedResourcePath = resourceAbsolutePath;
    
    if (fileInfo.exists())
    {
        //We should keep file extension if needed
        if (keepFileExtension)
            processedResourcePath = resourceAbsolutePath;
        else
        {
            //Remove file extension
            QString resourceExtension = QString(".%1").arg(fileInfo.suffix());
            processedResourcePath = TruncateFileExtension(resourceAbsolutePath, resourceExtension);
        }
		//Convert absolute path to relative one, if possible.
        const QString& resourceFolder = GetResourceRootDirectory();
        if (processedResourcePath.startsWith(resourceFolder, Qt::CaseInsensitive))
        {
        	processedResourcePath.replace(resourceFolder, RES_HEADER, Qt::CaseInsensitive);
		}
    }

    return processedResourcePath;
}

QStringList ResourcesManageHelper::GetFontsList()
{
    QStringList filesNamesList;
	// Get true type fonts
    // Get absoulute path
    QString fontsPath = QString::fromStdString(FilePath(FONTS_RES_PATH).GetAbsolutePathname());
    QDir dir(fontsPath);
    // Get the list of files in fonts directory - both true type fonts and graphics fonts
    filesNamesList = dir.entryList(FONTS_EXTENSIONS_FILTER, QDir::Files);
	fontsPath.clear();
    return filesNamesList;
}

void ResourcesManageHelper::InitInternalResources()
{
	// Save project default title
    if(DAVA::Core::Instance())
    {
        DAVA::KeyedArchive *options = DAVA::Core::Instance()->GetOptions();
        if(options)
        {
			projectTitle = options->GetString("title", PROJECT_TITLE.toStdString()).c_str();
        }
    }
	// If project name wasn't set - create default name
	if (projectTitle.isNull() || projectTitle.isEmpty())
		projectTitle = PROJECT_TITLE;
}

QString ResourcesManageHelper::GetDocumentationPath()
{
	return DOCUMENTATION_PATH;
}

void ResourcesManageHelper::SetProjectPath(const QString &path)
{
    projectPath = path;
}

QString ResourcesManageHelper::GetProjectPath()
{
    return projectPath;
}

QString ResourcesManageHelper::GetProjectTitle()
{
	// Set default project title
	QString projectTitleString = projectTitle;
	// Get active project path
	QString projectPath = GetProjectPath();
	if (!projectPath.isNull() && !projectPath.isEmpty())
	{
		projectTitleString = GetProjectTitle(GetProjectFilePath(projectPath));
	}

	return projectTitleString;
}

QString ResourcesManageHelper::GetProjectTitle(const QString& projectFilePath)
{
	return QString("%1 | Project %2").arg(projectTitle).arg(projectFilePath);
}

QString ResourcesManageHelper::GetDefaultDirectory()
{
	QString defaultDir = QString::fromStdString(EditorSettings::Instance()->GetProjectPath());
	//If default directory path is not available in project settings - use current working path
	if ( defaultDir.isNull() || defaultDir.isEmpty() )
	{
  		defaultDir = QDir::currentPath();
	}
	return defaultDir;
}

QString ResourcesManageHelper::GetResourceRootDirectory()
{
	QString projectPath = GetProjectPath();
	if (projectPath.isNull() || projectPath.isEmpty())
	{
		return QString();
	}
	return GetDataPath(projectPath);
}

QString ResourcesManageHelper::GetResourceFolder(const QString& resourcePath)
{
	QString projectPath = GetProjectPath();
	if (projectPath.isNull() || projectPath.isEmpty())
	{
		return QString();
	}
	return QString(resourcePath).arg(projectPath);
}

QString ResourcesManageHelper::GetSpritesDirectory()
{
	return GetResourceFolder(PROJECT_DATA_GFX);
}

QString ResourcesManageHelper::GetFontSpritesDirectory()
{
	return GetResourceFolder(PROJECT_DATA_GRAPHICS_FONTS);
}

QString ResourcesManageHelper::GetFontSpritesDatasourceDirectory()
{
	return GetResourceFolder(PROJECT_DATASOURCE_GRAPHICS_FONTS);
}

QString ResourcesManageHelper::GetDataPath(const QString& projectPath)
{
	return QString(PROJECT_DATA).arg(projectPath);
}

QString ResourcesManageHelper::GetPlatformRootPath(const QString& projectPath)
{
	return QString(PROJECT_PLATFORM_PATH).arg(projectPath);
}
	
QString ResourcesManageHelper::GetProjectFilePath(const QString& projectPath)
{	
	return QString(PROJECT_FILE_PATH).arg(projectPath);
}

void ResourcesManageHelper::ShowErrorMessage(const QString& messageParam)
{
	QMessageBox messageBox;
	messageBox.setText(QString(RES_WRONG_LOCATION_ERROR_MESSAGE).arg(messageParam));
	messageBox.setStandardButtons(QMessageBox::Ok);
	messageBox.exec();
}