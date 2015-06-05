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


#include "HierarchyTreePlatformNode.h"
#include "HierarchyTreeScreenNode.h"
#include "HierarchyTreeAggregatorNode.h"
#include "ResourcesManageHelper.h"
#include <QDir>

#define WIDTH_NODE "width"
#define HEIGHT_NODE "height"
#define SCREENS_NODE "screens"
#define AGGREGATORS_NODE "aggregators"

#define LOCALIZATION_PATH_NODE "LocalizationPath"
#define LOCALIZATION_LOCALE_NODE "Locale"

#define SCREEN_PATH "%1/%2.yaml"

HierarchyTreePlatformNode::HierarchyTreePlatformNode(HierarchyTreeRootNode* rootNode, const QString& name) :
	HierarchyTreeNode(name),
    isPreview(false)
{
	this->rootNode = rootNode;
	width = height = 0;
}

HierarchyTreePlatformNode::HierarchyTreePlatformNode(HierarchyTreeRootNode* rootNode, const HierarchyTreePlatformNode* base) :
	HierarchyTreeNode(base),
    isPreview(false)
{
	this->rootNode = rootNode;
	this->width = base->GetWidth();
	this->height = base->GetHeight();
	
	const HierarchyTreeNode::HIERARCHYTREENODESLIST& chilren = base->GetChildNodes();
	for (HierarchyTreeNode::HIERARCHYTREENODESLIST::const_iterator iter = chilren.begin();
		 iter != chilren.end();
		 ++iter)
	{
		const HierarchyTreeScreenNode* screen = dynamic_cast<const HierarchyTreeScreenNode* >((*iter));
		const HierarchyTreeAggregatorNode* aggregator = dynamic_cast<const HierarchyTreeAggregatorNode* >((*iter));

		if (!screen && !aggregator)
			continue;

		HierarchyTreeNode* control;
		if (aggregator)
		{
			control = new HierarchyTreeAggregatorNode(this, aggregator);
		}
		else
		{
			control = new HierarchyTreeScreenNode(this, screen);
		}
		AddTreeNode(control);
	}
}

HierarchyTreePlatformNode::~HierarchyTreePlatformNode()
{
	// Remove screens before removing aggregators
	HierarchyTreeNode::HIERARCHYTREENODESITER it;
	for (it = childNodes.begin(); it != childNodes.end();)
	{
		if (!dynamic_cast<HierarchyTreeAggregatorNode*>(*it))
		{
			delete (*it);
			it = childNodes.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void HierarchyTreePlatformNode::SetSize(int width, int height)
{
	this->width = width;
	this->height = height;
	
	for (HIERARCHYTREENODESLIST::iterator iter = childNodes.begin(); iter != childNodes.end(); ++iter)
	{
		HierarchyTreeNode* node = (*iter);
		
		HierarchyTreeAggregatorNode* aggregator = dynamic_cast<HierarchyTreeAggregatorNode*>(node);
		if (aggregator)
			continue;
		
		HierarchyTreeScreenNode* screenNode = dynamic_cast<HierarchyTreeScreenNode*>(node);
		if (screenNode)
		{
			screenNode->GetScreen()->SetRect(Rect(0, 0, width, height));
		}
	}
}
	
int HierarchyTreePlatformNode::GetWidth(bool forceOriginal) const
{
    return forceOriginal || !isPreview ? width : previewWidth;
}

int HierarchyTreePlatformNode::GetHeight(bool forceOriginal) const
{
    return forceOriginal || !isPreview ? height : previewHeight;
}

Vector2 HierarchyTreePlatformNode::GetSize(bool forceOriginal) const
{
    return Vector2(GetWidth(forceOriginal), GetHeight(forceOriginal));
}

HierarchyTreeNode* HierarchyTreePlatformNode::GetParent()
{
	// Root Node is the parent for the Platform.
	return this->rootNode;
}

FilePath HierarchyTreePlatformNode::GetPlatformFolder() const
{
	QString path;
	if (rootNode)
	{
		path = ResourcesManageHelper::GetPlatformRootPath(rootNode->GetProjectDir());
	}
	path += GetName();

    FilePath folder(path.toStdString());
    folder.MakeDirectoryPathname();
    
	return folder;
}

QString HierarchyTreePlatformNode::GetScreenPath(QString screenName) const
{
	return QString(SCREEN_PATH).arg(QString::fromStdString(GetPlatformFolder().GetAbsolutePathname())).arg(screenName);
}

QString HierarchyTreePlatformNode::GetScreenPath(String screenName) const
{
	return GetScreenPath(QString::fromStdString(screenName));
}

void HierarchyTreePlatformNode::ActivatePlatform()
{
	if (rootNode)
	{
		FilePath bundleName(rootNode->GetProjectDir().toStdString());
        bundleName.MakeDirectoryPathname();
        
        List<FilePath> resFolders = FilePath::GetResourcesFolders();
        List<FilePath>::const_iterator searchIt = find(resFolders.begin(), resFolders.end(), bundleName);
        
        if(searchIt == resFolders.end())
        {
            FilePath::AddResourcesFolder(bundleName);
        }
	}
}

bool HierarchyTreePlatformNode::Load(const YamlNode* platform, List<QString>& fileNames)
{
	const YamlNode* width = platform->Get(WIDTH_NODE);
	const YamlNode* height = platform->Get(HEIGHT_NODE);
	if (!width || !height)
		return false;
	
	bool result = true;
	SetSize(width->AsInt(), height->AsInt());
	ActivatePlatform();
	
	const YamlNode* screens = platform->Get(SCREENS_NODE);
	if (screens)
	{
		for (uint32 i = 0; i < screens->GetCount(); i++)
		{
			const YamlNode* screen = screens->Get(i);
			if (!screen)
				continue;
			String screenName = screen->AsString();
			
			QString screenPath = GetScreenPath(screenName);
            fileNames.push_back(screenPath);

			HierarchyTreeScreenNode* screenNode = new HierarchyTreeScreenNode(this, QString::fromStdString(screenName));
			
            // Do not load screen now,it will be done on selecting it.
            FileSystem::Instance()->LockFile(screenPath.toStdString(), false);
			AddTreeNode(screenNode);
		}
	}
	
    List<HierarchyTreeAggregatorNode*> aggregatorNodes;
	const YamlNode* aggregators = platform->Get(AGGREGATORS_NODE);
	if (aggregators)
	{
		for (uint32 i = 0; i < aggregators->GetCount(); i++)
		{
			String aggregatorName = aggregators->GetItemKeyName(i);
            if (aggregatorName.empty())
                continue;

			QString aggregatorPath = GetScreenPath(aggregatorName);
            fileNames.push_back(aggregatorPath);

			HierarchyTreeAggregatorNode* aggregatorNode = new HierarchyTreeAggregatorNode(this, QString::fromStdString(aggregatorName), Rect());

            const YamlNode* aggregator = aggregators->Get(i);
			const YamlNode* aggregatorWidth = aggregator->Get(WIDTH_NODE);
			const YamlNode* aggregatorHeight = aggregator->Get(HEIGHT_NODE);
			if (!aggregatorWidth || !aggregatorHeight)
			{
				result = false;
			}
			else
			{
				Rect r = Rect(0, 0, aggregatorWidth->AsInt(), aggregatorHeight->AsInt());
				result &= aggregatorNode->Load(r, aggregatorPath);
                aggregatorNodes.push_back(aggregatorNode);
			}

			AddTreeNode(aggregatorNode);			
		}
	}

    // When all screens and aggregators are loaded, re-sync the aggregator nodes content.
    for (List<HierarchyTreeAggregatorNode*>::iterator iter = aggregatorNodes.begin(); iter != aggregatorNodes.end();
         iter ++)
    {
        (*iter)->UpdateChilds();
    }

	return result;
}

bool HierarchyTreePlatformNode::LoadLocalization(const YamlNode* platform)
{
    if (!platform)
    {
        return false;
    }
    
    const YamlNode* pathNode = platform->Get(LOCALIZATION_PATH_NODE);
    const YamlNode* localeNode = platform->Get(LOCALIZATION_LOCALE_NODE);

    if (pathNode && localeNode &&
        !pathNode->AsString().empty() &&
        !localeNode->AsString().empty())
    {
        localizationPath = pathNode->AsString();
		// YuriCoder, 2013/04/23. Localization path must be absolute - do the conversion
		// for previous projects.
		localizationPath.MakeDirectoryPathname();

        locale = localeNode->AsString();
    }

    return true;
}

bool HierarchyTreePlatformNode::Save(YamlNode* node, bool saveAll, List<QString>& fileNames)
{
	YamlNode* platform = YamlNode::CreateMapNode(false);
	platform->Set(WIDTH_NODE, GetWidth());
	platform->Set(HEIGHT_NODE, GetHeight());

	node->SetNodeToMap( GetName().toStdString(), platform );
	ActivatePlatform();
	
	YamlNode* screens = YamlNode::CreateArrayNode(YamlNode::AR_BLOCK_REPRESENTATION);
	platform->SetNodeToMap( SCREENS_NODE, screens );
	
	YamlNode* aggregators = YamlNode::CreateMapNode(false);
	platform->SetNodeToMap( AGGREGATORS_NODE, aggregators );

    // Add the Localization info - specific for each Platform.
    SaveLocalization(platform);

	FilePath platformFolder = GetPlatformFolder();

	QDir dir;
	dir.mkpath(QString::fromStdString(platformFolder.GetAbsolutePathname()));
	
	bool result = true;
	//save aggregators node before save screens
	for (HIERARCHYTREENODESCONSTITER iter = GetChildNodes().begin();
		 iter != GetChildNodes().end();
		 ++iter)
	{
		HierarchyTreeAggregatorNode* node = dynamic_cast<HierarchyTreeAggregatorNode*>(*iter);
		if (!node)
			continue;

		QString path = GetScreenPath(node->GetName());
		fileNames.push_back(path);
		
		YamlNode* aggregator = YamlNode::CreateMapNode();
		result &= node->Save(aggregator, path, saveAll);
		aggregators->SetNodeToMap(node->GetName().toStdString(), aggregator);
	}
	
	for (HIERARCHYTREENODESCONSTITER iter = GetChildNodes().begin();
		 iter != GetChildNodes().end();
		 ++iter)
	{
		HierarchyTreeAggregatorNode* node = dynamic_cast<HierarchyTreeAggregatorNode*>(*iter);
		if (node)
			continue;	//skip aggregators
		
		HierarchyTreeScreenNode* screenNode = dynamic_cast<HierarchyTreeScreenNode*>(*iter);
		if (!screenNode)
			continue;
		
		QString screenPath = GetScreenPath(screenNode->GetName());
		fileNames.push_back(screenPath);

        if(screenNode->IsLoaded())
        {
            // Save only loaded (and thus may be changed) screens.
            result &= screenNode->Save(screenPath, saveAll);
        }
		
		screens->Add(screenNode->GetName().toStdString());
	}
	return result;
}

bool HierarchyTreePlatformNode::SaveLocalization(YamlNode* platform)
{
    if (!platform)
    {
        return false;
    }

	// YuriCoder, 2013/04/23. Localization path must be absolute.
	if(this->localizationPath.IsEmpty())
	{
		return false;
	}
	this->localizationPath.MakeDirectoryPathname();

	//TODO VK: Fix FilePath::GetFrameworkPath()
	String pathname = this->localizationPath.GetFrameworkPath();

    platform->Set(LOCALIZATION_PATH_NODE, pathname);
    platform->Set(LOCALIZATION_LOCALE_NODE, locale);

    return true;
}

void HierarchyTreePlatformNode::SetLocalizationPath(const FilePath & localizationPath)
{
    this->localizationPath = localizationPath;
}

void HierarchyTreePlatformNode::SetLocale(const String& locale)
{
    this->locale = locale;
}

void HierarchyTreePlatformNode::ReturnTreeNodeToScene()
{
	if (!this->redoParentNode)
	{
		return;
	}
	
	// Need to recover the node previously deleted, taking position into account.
	this->redoParentNode->AddTreeNode(this, redoPreviousNode);
}

void HierarchyTreePlatformNode::SetParent(HierarchyTreeNode* node, HierarchyTreeNode* insertAfter)
{
	if (!node)
		return;
	
	node->RemoveTreeNode(this, false, false);
	node->AddTreeNode(this, insertAfter);
}

bool HierarchyTreePlatformNode::IsAggregatorOrScreenNamePresent(const QString& candidatName)
{
	for (HIERARCHYTREENODESLIST::iterator iter = childNodes.begin(); iter != childNodes.end(); ++iter)
	{
		HierarchyTreeNode* node = (*iter);
		
		HierarchyTreeAggregatorNode* aggregator = dynamic_cast<HierarchyTreeAggregatorNode*>(node);
		HierarchyTreeScreenNode* screen = dynamic_cast<HierarchyTreeScreenNode*>(node);
		if (NULL == aggregator && NULL == screen)
		{
			continue;
		}
		if(node->GetName().compare(candidatName, Qt::CaseInsensitive) == 0)
		{
			return true;
		}
	}
	return false;
}

HierarchyTreeAggregatorNode* HierarchyTreePlatformNode::GetAggregatorNodeByName(const QString& aggregatorName)
{
	for (HIERARCHYTREENODESLIST::iterator iter = childNodes.begin(); iter != childNodes.end(); ++iter)
	{
		HierarchyTreeNode* node = (*iter);
		
		HierarchyTreeAggregatorNode* aggregator = dynamic_cast<HierarchyTreeAggregatorNode*>(node);
		if (NULL == aggregator)
		{
			continue;
		}
        
		if(node->GetName().compare(aggregatorName, Qt::CaseInsensitive) == 0)
		{
			return aggregator;
		}
	}
	return NULL;
}

void HierarchyTreePlatformNode::SetPreviewMode(int width, int height)
{
    previewWidth = width;
    previewHeight = height;
    isPreview = true;
}

void HierarchyTreePlatformNode::DisablePreview()
{
    isPreview = false;
}
