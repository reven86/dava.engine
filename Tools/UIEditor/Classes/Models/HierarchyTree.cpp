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




#include "HierarchyTree.h"
#include "HierarchyTreePlatformNode.h"
#include "HierarchyTreeScreenNode.h"
#include "HierarchyTreeControlNode.h"
#include "HierarchyTreeNode.h"
#include "HierarchyTreeController.h"

#include "MetadataFactory.h"
#include "ResourcesManageHelper.h"
#include "EditorFontManager.h"
#include "FileSystem/FileSystem.h"

#include <QFile>
#include <QDir>

#if defined (__DAVAENGINE_MACOS__)
#include <utime.h>
#endif

#define PLATFORMS_NODE "platforms"
#define LOCALIZATION_NODE "localization"
#define LOCALIZATION_PATH_NODE "LocalizationPath"
#define LOCALIZATION_LOCALE_NODE "Locale"
#define FONT_NODE "font"
#define DEFAULT_FONT_PATH_NODE "DefaultFontPath"
#define DEFAULT_FONTS_PATH_NODE "DefaultFontsPath"

HierarchyTree::HierarchyTree()
{
	projectCreated = false;
}

HierarchyTree::~HierarchyTree()
{
    Clear();
}

void HierarchyTree::Clear()
{
    rootNode.Clear();
    UnlockProjectFiles(true);
}

bool HierarchyTree::Load(const QString& projectPath)
{
	CreateProject();

	// Attempt to create a project
	YamlParser* project = YamlParser::Create(projectPath.toStdString());
	if (!project)
		return false;
	// Set current project file path
	rootNode.SetProjectFilePath(projectPath);
	
	YamlNode* projectRoot = project->GetRootNode();
	if (!projectRoot)
	{
		SafeRelease(project);
		return false;
	}

    // Build the list of file names to be locked.
    List<QString> fileNames;
    fileNames.push_back(projectPath);

    // NO Localization Data should exist at this point, otherwise automatic
    // LocalizedStrings obtaining will interfere with the loading process!
    LocalizationSystem::Instance()->Cleanup();

    Map<HierarchyTreePlatformNode*, const YamlNode*> loadedPlatforms;

	bool result = true;
    
    // add project path to res folders (to allow loading fonts before everything else)
    FilePath bundleName(rootNode.GetProjectDir().toStdString());
    bundleName.MakeDirectoryPathname();
    
    List<FilePath> resFolders = FilePath::GetResourcesFolders();
    List<FilePath>::const_iterator searchIt = find(resFolders.begin(), resFolders.end(), bundleName);
    
    if(searchIt == resFolders.end())
    {
        FilePath::AddResourcesFolder(bundleName);
    }
    EditorFontManager::Instance()->SetProjectDataPath(bundleName.GetAbsolutePathname() + "Data/");
    
    const YamlNode *font = projectRoot->Get(FONT_NODE);
    
    // Get font node
	if (font)
	{
		// Get default font node
		const YamlNode *fontPath = font->Get(DEFAULT_FONT_PATH_NODE);
		if (fontPath)
		{
			// Get font values into array
			const Vector<YamlNode*> &fontPathArray = fontPath->AsVector();
			EditorFontManager::DefaultFontPath defaultFontPath("", "");
			// True type font
			if (fontPathArray.size() == 1)
			{
				defaultFontPath.fontPath = FilePath(fontPathArray[0]->AsString());
			}
			else if (fontPathArray.size() == 2) // Graphics font
			{
				defaultFontPath.fontPath = FilePath(fontPathArray[0]->AsString());
				defaultFontPath.fontSpritePath = FilePath(fontPathArray[1]->AsString());
			}
			EditorFontManager::Instance()->InitDefaultFontFromPath(defaultFontPath);
		}
        
        const YamlNode *localizationFontsPathNode = font->Get(DEFAULT_FONTS_PATH_NODE);
        if(localizationFontsPathNode)
        {
            FilePath localizationFontsPath(localizationFontsPathNode->AsString());
            if(localizationFontsPath.Exists())
            {
                EditorFontManager::Instance()->SetDefaultFontsPath(localizationFontsPath.GetAbsolutePathname());
            }
            else
            {
                EditorFontManager::Instance()->SetDefaultFontsPath(bundleName.GetAbsolutePathname() + "Data" + localizationFontsPath.GetAbsolutePathname().substr(5));
            }
        }
    }
    
    if(EditorFontManager::Instance()->GetDefaultFontsPath().IsEmpty())
    {
        EditorFontManager::Instance()->SetDefaultFontsPath(FilePath(bundleName.GetAbsolutePathname() + "Data/UI/Fonts/fonts.yaml"));
    }
    EditorFontManager::Instance()->LoadLocalizedFonts();
    
	const YamlNode* platforms = projectRoot->Get(PLATFORMS_NODE);
	for (uint32 i = 0; i < platforms->GetCount(); i++)
	{
		const String &platformName = platforms->GetItemKeyName(i);
        if (platformName.empty())
            continue;

		HierarchyTreePlatformNode* platformNode = new HierarchyTreePlatformNode(&rootNode, QString::fromStdString(platformName));
        const YamlNode* platform = platforms->Get(i);
		result &= platformNode->Load(platform, fileNames);
		rootNode.AddTreeNode(platformNode);
        
        // Remember the platform to load its localization later.
        loadedPlatforms.insert(std::make_pair(platformNode, platform));
	}

    // After the project is loaded and tree is build, update the Tree Extradata with the texts from buttons just loaded.
    // Do this for all platforms and screens. The update direction is FROM Control TO Extra Data.
    UpdateExtraData(BaseMetadata::UPDATE_EXTRADATA_FROM_CONTROL);

    // Now we can load the Localization for each Platform.
    for (Map<HierarchyTreePlatformNode*, const YamlNode*>::iterator iter = loadedPlatforms.begin();
         iter != loadedPlatforms.end(); iter ++)
    {
        iter->first->LoadLocalization(iter->second);
    }

    // Preview Modes are also stored in the project file - update them too.
    PreviewController::Instance()->LoadPreviewSettings(projectRoot);

    // All the data needed is loaded.
    SafeRelease(project);
    
	HierarchyTreePlatformNode* platformNode = NULL;
	HierarchyTreeScreenNode* screenNode = NULL;
	
	if (!rootNode.GetChildNodes().empty())
	{
		platformNode = dynamic_cast<HierarchyTreePlatformNode*>((*rootNode.GetChildNodes().begin()));
	}
	
	if (platformNode && !platformNode->GetChildNodes().empty())
	{
		screenNode = dynamic_cast<HierarchyTreeScreenNode*>((*platformNode->GetChildNodes().begin()));
	}
    
    // Update aggregators for all the platform we loaded.
    for (Map<HierarchyTreePlatformNode*, const YamlNode*>::iterator iter = loadedPlatforms.begin();
         iter != loadedPlatforms.end(); iter ++)
    {
        HierarchyTreeController::Instance()->UpdateAggregators(iter->first);
    }
    
    // After the project is loaded and tree is build, update the Tree Extradata with the texts from buttons just loaded.
    // Do this for all platforms and screens.
	HierarchyTreeController::Instance()->UpdateSelection(platformNode, screenNode);

    // Can lock the project files here.
    LockProjectFiles(fileNames);
    
	return result;
}

void HierarchyTree::CreateProject()
{
	projectCreated = true;
}

void HierarchyTree::CloseProject()
{
	projectCreated = false;
	// Remove closed project resource folder
	FilePath bundleName(rootNode.GetProjectDir().toStdString());
	if (!bundleName.IsEmpty())
	{
		bundleName.MakeDirectoryPathname();
		// DF-1805 - Do not remove bundleName which is not in resources
        List<FilePath> resFolders = FilePath::GetResourcesFolders();
        List<FilePath>::const_iterator searchIt = find(resFolders.begin(), resFolders.end(), bundleName);
        
        if(searchIt != resFolders.end())
        {
			FilePath::RemoveResourcesFolder(bundleName);
		}
	}  
	// Reset project path
	rootNode.SetProjectFilePath(QString());
	rootNode.ResetUnsavedChanges();
	// Reset default font
	EditorFontManager::Instance()->ResetDefaultFont();
    // TODO: reset localized fonts path (unload localized fonts)
    EditorFontManager::Instance()->Reset();
	Clear();
}

HierarchyTreePlatformNode* HierarchyTree::AddPlatform(const QString& name, const Vector2& size)
{
    HierarchyTreePlatformNode* platformNode = new HierarchyTreePlatformNode(&rootNode, name);
	platformNode->SetSize(size.dx, size.dy);
	rootNode.AddTreeNode(platformNode);
	
	return platformNode;
}

HierarchyTreeScreenNode* HierarchyTree::AddScreen(const QString& name, HierarchyTreeNode::HIERARCHYTREENODEID platformId)
{
	HierarchyTreeNode* baseNode = FindNode(&rootNode, platformId);
	HierarchyTreePlatformNode* platformNode = dynamic_cast<HierarchyTreePlatformNode*>(baseNode);
	if (!platformNode)
	{
		return NULL;
	}
	
	HierarchyTreeScreenNode* screenNode = new HierarchyTreeScreenNode(platformNode, name);
	InsertScreenNode(platformNode, screenNode);

	return screenNode;
}

void HierarchyTree::InsertScreenNode(HierarchyTreePlatformNode* platformNode, HierarchyTreeScreenNode* screenNode)
{
	// Insert the new Screen Node right at the end of the screens list,
	// but before the Aggregators. See pls DF-2011 for details.
	HierarchyTreeNode* nodeToInsertAfter = NULL;

	HierarchyTreeNode::HIERARCHYTREENODESLIST::const_iterator iter;
	for (iter = platformNode->GetChildNodes().begin(); iter != platformNode->GetChildNodes().end(); ++iter)
    {
		HierarchyTreeNode* node = (*iter);
		if (dynamic_cast<HierarchyTreeAggregatorNode*>(node))
		{
			// Stop here.
			break;
		}

		if (dynamic_cast<HierarchyTreeScreenNode*>(node))
		{
			nodeToInsertAfter = node;
			continue;
		}
	}

	platformNode->AddTreeNode(screenNode, nodeToInsertAfter);
}

HierarchyTreeAggregatorNode* HierarchyTree::AddAggregator(const QString& name, HierarchyTreeNode::HIERARCHYTREENODEID platformId, const Rect& rect)
{
	HierarchyTreeNode* baseNode = FindNode(&rootNode, platformId);
	HierarchyTreePlatformNode* platformNode = dynamic_cast<HierarchyTreePlatformNode*>(baseNode);
	if (!platformNode)
	{
		return NULL;
	}
	
	HierarchyTreeAggregatorNode* aggregatorNode = new HierarchyTreeAggregatorNode(platformNode, name, rect);
	platformNode->AddTreeNode(aggregatorNode);
	
	return aggregatorNode;
}

HierarchyTreeNode* HierarchyTree::GetNode(HierarchyTreeNode::HIERARCHYTREENODEID id) const
{
	if (rootNode.GetId() == id)
		return (HierarchyTreeNode*)&rootNode;
		
	return FindNode(&rootNode, id);
}

HierarchyTreeNode* HierarchyTree::FindNode(const HierarchyTreeNode *parent, HierarchyTreeNode::HIERARCHYTREENODEID id) const
{
	if (!parent)
		return NULL;
	
	HierarchyTreeNode::HIERARCHYTREENODESLIST::const_iterator iter;
	for (iter = parent->GetChildNodes().begin(); iter != parent->GetChildNodes().end(); ++iter)
    {
		HierarchyTreeNode* node = (*iter);
		
		if (node->GetId() == id)
			return node;
		
		node = FindNode(node, id);
		if (node)
			return node;
	}
	
	return NULL;
}

HierarchyTreeNode* HierarchyTree::GetNode(const UIControl* control) const
{
	return FindNode(&rootNode, control);
}

HierarchyTreeNode* HierarchyTree::FindNode(const HierarchyTreeNode* parent, const UIControl* control) const
{
	if (!parent)
		return NULL;
	
	HierarchyTreeNode::HIERARCHYTREENODESLIST::const_iterator iter;
	for (iter = parent->GetChildNodes().begin(); iter != parent->GetChildNodes().end(); ++iter) {
		HierarchyTreeNode* node = (*iter);
		
		HierarchyTreeControlNode* controlNode = dynamic_cast<HierarchyTreeControlNode*>(node);
		if (controlNode && controlNode->GetUIObject() == control)
			return node;
		
		node = FindNode(node, control);
		if (node)
			return node;
	}
	
	return NULL;
}

const HierarchyTreeNode::HIERARCHYTREENODESLIST& HierarchyTree::GetPlatforms() const
{
	return rootNode.GetChildNodes();
}

HierarchyTreeNode::HIERARCHYTREENODESLIST HierarchyTree::GetNodes() const
{
    HierarchyTreeNode::HIERARCHYTREENODESLIST resultList;
    if (GetPlatforms().empty())
    {
        return  resultList;
    }

    GetNodesRecursive(&rootNode, resultList);

    return resultList;
}

void HierarchyTree::GetNodesRecursive(const HierarchyTreeNode* rootNode, HierarchyTreeNode::HIERARCHYTREENODESLIST& resultList) const
{
    for (HierarchyTreeNode::HIERARCHYTREENODESLIST::const_iterator iter = rootNode->GetChildNodes().begin(); iter != rootNode->GetChildNodes().end(); ++iter)
	{
		HierarchyTreeNode* childNode = (*iter);
		if (!childNode)
		{
			continue;
		}
        
        resultList.push_back(childNode);
        GetNodesRecursive(childNode, resultList);
    }
}

void HierarchyTree::DeleteNodes(const HierarchyTreeNode::HIERARCHYTREENODESLIST& nodes, bool deleteNodeFromMemory, bool deleteNodeFromScene)
{
	//copy id for safe delete
	Set<HierarchyTreeControlNode::HIERARCHYTREENODEID> ids;
	Set<HierarchyTreeControlNode::HIERARCHYTREENODEID>::iterator idIter;
	HierarchyTreeNode::HIERARCHYTREENODESLIST::const_iterator nodeIter;
	for (nodeIter = nodes.begin(); nodeIter != nodes.end(); ++nodeIter)
	{
		ids.insert((*nodeIter)->GetId());
	}
	
	for (idIter = ids.begin(); idIter != ids.end(); ++idIter)
	{
		HierarchyTreeControlNode::HIERARCHYTREENODEID id = (*idIter);
		HierarchyTreeNode* node = FindNode(&rootNode, id);
		if (!node)
			continue;
		
		HierarchyTreeControlNode* controlNode = dynamic_cast<HierarchyTreeControlNode*>(node);
		if (controlNode)
		{
			controlNode->GetParent()->RemoveTreeNode(controlNode, deleteNodeFromMemory, deleteNodeFromScene);
			continue;
		}
		
		HierarchyTreeScreenNode* screenNode = dynamic_cast<HierarchyTreeScreenNode*>(node);
		if (screenNode)
		{
			screenNode->GetPlatform()->RemoveTreeNode(screenNode, deleteNodeFromMemory, deleteNodeFromScene);
			continue;
		}
		
		HierarchyTreePlatformNode* platformNode = dynamic_cast<HierarchyTreePlatformNode*>(node);
		if (platformNode)
		{
			rootNode.RemoveTreeNode(platformNode, deleteNodeFromMemory, deleteNodeFromScene);
			continue;
		}
	}
}

bool HierarchyTree::SaveOnlyChangedScreens(const QString& projectPath)
{
	return DoSave(projectPath, false);
}

bool HierarchyTree::SaveAll(const QString& projectPath)
{
	return DoSave(projectPath, true);
}

bool HierarchyTree::DoSave(const QString& projectPath, bool saveAll)
{
	bool result = true;
	ScopedPtr<YamlNode> root( new YamlNode(YamlNode::TYPE_MAP) );
	
	// Get paths for default font
	const EditorFontManager::DefaultFontPath& defaultFontPath = EditorFontManager::Instance()->GetDefaultFontPath();
	FilePath fontPath = defaultFontPath.fontPath;
	FilePath fontSpritePath = defaultFontPath.fontSpritePath;
	// Check if default font path exist
	if (!fontPath.IsEmpty() || !EditorFontManager::Instance()->GetDefaultFontsPath().IsEmpty())
	{
		// Create font node
		YamlNode* fontNode = new YamlNode(YamlNode::TYPE_MAP);
		root->SetNodeToMap( FONT_NODE, fontNode );
        
        if(!fontPath.IsEmpty())
        {
            // Create fonts array
            YamlNode* fontPathNode = new YamlNode(YamlNode::TYPE_ARRAY);
            
            // Put font path
            fontPathNode->Add(fontPath.GetFrameworkPath());
            // Put font sprite path if it available
            if (!fontSpritePath.IsEmpty())
            {
                fontPathNode->Add(fontSpritePath.GetFrameworkPath());
            }
            // Insert array into node
            fontNode->AddNodeToMap(DEFAULT_FONT_PATH_NODE, fontPathNode);
        }
        
        if(!EditorFontManager::Instance()->GetDefaultFontsPath().IsEmpty())
        {

            fontNode->Add(DEFAULT_FONTS_PATH_NODE, EditorFontManager::Instance()->GetDefaultFontsFrameworkPath());
        }
    }

	YamlNode* platforms = new YamlNode(YamlNode::TYPE_MAP);
	root->SetNodeToMap( PLATFORMS_NODE, platforms );

    // Prior to Save we need to put the Localization Keys FROM the ExtraData TO the
    // appropriate text controls to save the localization keys, and not values.
    UpdateExtraData(BaseMetadata::UPDATE_CONTROL_FROM_EXTRADATA_RAW);

	QString oldPath = rootNode.GetProjectFilePath();
	
	// Get project file
	QString projectFile = ResourcesManageHelper::GetProjectFilePath(projectPath);
	
	rootNode.SetProjectFilePath(projectFile);
    
    if(!EditorFontManager::Instance()->GetDefaultFontsPath().IsEmpty())
    {
        EditorFontManager::Instance()->SaveLocalizedFonts();
    }

    // Unlock project files and delete unused items from disk before saving.
    UnlockProjectFiles(false);
    HierarchyTreeController::Instance()->DeleteUnusedItemsFromDisk(projectPath);

    // Do the save itself.
	List<QString> fileNames;
	for (HierarchyTreeNode::HIERARCHYTREENODESLIST::const_iterator iter = rootNode.GetChildNodes().begin();
		 iter != rootNode.GetChildNodes().end();
		 ++iter)
	{
		HierarchyTreePlatformNode* platformNode = dynamic_cast<HierarchyTreePlatformNode*>(*iter);
		if (!platformNode)
			continue;
		// In case we have new platform (copy or newly created) - we should force save for screens sequence
        FilePath platformFolder = platformNode->GetPlatformFolder();
		bool forceSave = !QDir(QString::fromStdString(platformFolder.GetAbsolutePathname())).exists();
        
		bool res = platformNode->Save(platforms, (forceSave || saveAll), fileNames);
		if (res)
		{
			platformNode->ResetUnsavedChanges();
		}

		result &= res;
	}

    PreviewController::Instance()->SavePreviewSettings(root);

	// Create project sub-directories
	QDir().mkpath(ResourcesManageHelper::GetPlatformRootPath(projectPath));
	// Update Data directory last modified datetime - set currrent time
	UpdateModificationDate(ResourcesManageHelper::GetDataPath(projectPath));

	// Save project file.
	result &= YamlEmitter::SaveToYamlFile(projectFile.toStdString(), root);
	fileNames.push_back(projectFile);

    // Return the Localized Values.
    UpdateExtraData(BaseMetadata::UPDATE_CONTROL_FROM_EXTRADATA_LOCALIZED);

	if (!result)
	{
		//restore project path
		rootNode.SetProjectFilePath(oldPath);
	}
	else
	{
		rootNode.ResetUnsavedChanges();
	}

	// List of files to be locked might be changed after save (if new platforms/screens are added).
	projectLockedFiles.clear();
	LockProjectFiles(fileNames);

	return result;
}

void HierarchyTree::UpdateExtraData(BaseMetadata::eExtraDataUpdateStyle updateStyle)
{
    for (HierarchyTreeNode::HIERARCHYTREENODESCONSTITER platformNodesIter = rootNode.GetChildNodes().begin();
         platformNodesIter != rootNode.GetChildNodes().end(); platformNodesIter ++)
    {
        HierarchyTreePlatformNode* platformNode = dynamic_cast<HierarchyTreePlatformNode*>(*platformNodesIter);
        DVASSERT(platformNode);
        if (!platformNode)
        {
            continue;
        }
        
        for (HierarchyTreeNode::HIERARCHYTREENODESCONSTITER screenNodesIter = platformNode->GetChildNodes().begin();
             screenNodesIter != platformNode->GetChildNodes().end(); screenNodesIter ++)
        {
            HierarchyTreeScreenNode* screenNode = dynamic_cast<HierarchyTreeScreenNode*>(*screenNodesIter);
            DVASSERT(screenNode);
            if (!screenNode)
            {
                continue;
            }

            UpdateExtraData(screenNode, updateStyle);
        }
    }
}

void HierarchyTree::UpdateExtraData(const HierarchyTreeScreenNode* screenNode,BaseMetadata::eExtraDataUpdateStyle updateStyle)
{
    // Update extra data from controls in a recursive way.
    for (HierarchyTreeNode::HIERARCHYTREENODESCONSTITER controlNodesIter = screenNode->GetChildNodes().begin();
         controlNodesIter != screenNode->GetChildNodes().end(); controlNodesIter ++)
    {
        HierarchyTreeControlNode* controlNode = dynamic_cast<HierarchyTreeControlNode*>(*controlNodesIter);
        UpdateExtraDataRecursive(controlNode, updateStyle);
    }
}

void HierarchyTree::UpdateExtraDataRecursive(HierarchyTreeControlNode* node, BaseMetadata::eExtraDataUpdateStyle updateStyle)
{
    if (!node)
    {
        DVASSERT(false);
        return;
    }

    // Create the Metadata, initialize with params.
    BaseMetadata* metadata = MetadataFactory::Instance()->GetMetadataForTreeNode(node);
    if (metadata)
    {
        METADATAPARAMSVECT params;
        params.push_back(BaseMetadataParams(node->GetId(), node->GetUIObject()));
        metadata->SetupParams(params);
        metadata->SetActiveParamID(0);

        metadata->UpdateExtraData(node->GetExtraData(), updateStyle);
        delete metadata;
    }

    // Repeat the same for all inner children.
    for (HierarchyTreeNode::HIERARCHYTREENODESCONSTITER iter = node->GetChildNodes().begin();
         iter != node->GetChildNodes().end(); iter ++)
    {
        HierarchyTreeControlNode* childNode = dynamic_cast<HierarchyTreeControlNode*>(*iter);
        DVASSERT(childNode);
        if (!childNode)
        {
            continue;
        }
        
        UpdateExtraDataRecursive(childNode, updateStyle);
    }
}

void HierarchyTree::UpdateControlsData(const HierarchyTreeScreenNode* screenNode)
{
	UpdateExtraData(screenNode, BaseMetadata::UPDATE_EXTRADATA_FROM_CONTROL);
}

void HierarchyTree::UpdateControlsData()
{
	UpdateExtraData(BaseMetadata::UPDATE_EXTRADATA_FROM_CONTROL);
}

void HierarchyTree::UpdateLocalization()
{
    UpdateExtraData(BaseMetadata::UPDATE_CONTROL_FROM_EXTRADATA_LOCALIZED);
}

void HierarchyTree::UpdateLocalization(const HierarchyTreeScreenNode* screenNode)
{
	UpdateExtraData(screenNode, BaseMetadata::UPDATE_CONTROL_FROM_EXTRADATA_LOCALIZED);
}

List<HierarchyTreeScreenNode*> HierarchyTree::GetUnsavedScreens()
{
	List<HierarchyTreeScreenNode*> resultList;
	
	if (GetPlatforms().empty())
	{
		return resultList;
	}

	for (HierarchyTreeNode::HIERARCHYTREENODESLIST::const_iterator platformIter = GetPlatforms().begin();
		 platformIter != GetPlatforms().end(); ++platformIter)
	{
		HierarchyTreePlatformNode* platformNode = dynamic_cast<HierarchyTreePlatformNode*>(*platformIter);
		if (!platformNode)
		{
			continue;
		}

		for (HierarchyTreeNode::HIERARCHYTREENODESLIST::const_iterator screenIter = platformNode->GetChildNodes().begin();
			 screenIter != platformNode->GetChildNodes().end(); ++screenIter)
		{
			HierarchyTreeScreenNode* screenNode = dynamic_cast<HierarchyTreeScreenNode*>(*screenIter);
			if (!screenNode)
			{
				continue;
			}

			// Does this screen contain unsaved changes?
			if (screenNode->GetUnsavedChanges() != 0)
			{
				resultList.push_back(screenNode);
			}
		}
	}

	return resultList;
}

bool HierarchyTree::IsPlatformNamePresent(const QString& name) const
{
	for (HierarchyTreeNode::HIERARCHYTREENODESLIST::const_iterator platformIter = GetPlatforms().begin();
	 platformIter != GetPlatforms().end(); ++platformIter)
	{
		HierarchyTreePlatformNode* platformNode = dynamic_cast<HierarchyTreePlatformNode*>(*platformIter);
		if (!platformNode)
		{
			continue;
		}
		if(name.compare(platformNode->GetName(), Qt::CaseInsensitive) == 0)
		{
			return true;
		}
	}

	return false;
}

void HierarchyTree::UpdateModificationDate(const QString &path)
{
#if defined (__DAVAENGINE_MACOS__)
	// Update last modification datetime of file or folder with current time
	// 02/05/2014 - Request only for MACOS
	utime(path.toStdString().c_str(), NULL);
#endif
}

void HierarchyTree::LockProjectFiles(const List<QString>& fileNames)
{
    for (List<QString>::const_iterator iter = fileNames.begin(); iter != fileNames.end(); iter ++)
    {
        const String& fileName = (*iter).toStdString();
        if (FileSystem::Instance()->LockFile(fileName, true))
        {
            projectLockedFiles.push_back(fileName);
        }
        else
        {
            Logger::Warning("Unable to lock file %s", fileName.c_str());
        }
    }
}

void HierarchyTree::UnlockProjectFiles(bool needCleanup)
{
    for (List<String>::iterator iter = projectLockedFiles.begin(); iter != projectLockedFiles.end(); iter ++)
    {
        const String& fileName = (*iter);
        FileSystem::Instance()->LockFile(fileName, false);
    }

    if (needCleanup)
    {
        projectLockedFiles.clear();
    }
}
