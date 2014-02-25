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
	
int HierarchyTreePlatformNode::GetWidth() const
{
    return isPreview ? previewWidth : width;
}

int HierarchyTreePlatformNode::GetHeight() const
{
    return isPreview ? previewHeight : height;
}

Vector2 HierarchyTreePlatformNode::GetSize() const
{
    return Vector2(GetWidth(), GetHeight());
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

bool HierarchyTreePlatformNode::Load(const YamlNode* platform)
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
		for (int i = 0; i < screens->GetCount(); i++)
		{
			const YamlNode* screen = screens->Get(i);
			if (!screen)
				continue;
			String screenName = screen->AsString();
			
			QString screenPath = GetScreenPath(screenName);
			HierarchyTreeScreenNode* screenNode = new HierarchyTreeScreenNode(this, QString::fromStdString(screenName));
			result &= screenNode->Load(screenPath);
			AddTreeNode(screenNode);
		}
	}
	
	const YamlNode* aggregators = platform->Get(AGGREGATORS_NODE);
	if (aggregators)
	{
		for (int i = 0; i < aggregators->GetCount(); i++)
		{
			const YamlNode* aggregator = aggregators->Get(i);
			if (!aggregator)
				continue;
			String aggregatorName = aggregator->AsString();

			QString aggregatorPath = GetScreenPath(aggregatorName);
			HierarchyTreeAggregatorNode* aggregatorNode = new HierarchyTreeAggregatorNode(this, QString::fromStdString(aggregatorName), Rect());

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
			}

			AddTreeNode(aggregatorNode);			
		}
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

bool HierarchyTreePlatformNode::Save(YamlNode* node, bool saveAll)
{
	YamlNode* platform = new YamlNode(YamlNode::TYPE_MAP);
	platform->Set(WIDTH_NODE, GetWidth());
	platform->Set(HEIGHT_NODE, GetHeight());

	node->SetNodeToMap( GetName().toStdString(), platform );
	ActivatePlatform();
	
	YamlNode* screens = new YamlNode(YamlNode::TYPE_ARRAY, YamlNode::REPRESENT_ARRAY_AS_MULTI_LINE);
	platform->SetNodeToMap( SCREENS_NODE, screens );
	
	YamlNode* aggregators = new YamlNode(YamlNode::TYPE_MAP);
	platform->SetNodeToMap( AGGREGATORS_NODE, aggregators );

    // Add the Localization info - specific for each Platform.
    SaveLocalization(platform);

	FilePath platformFolder = GetPlatformFolder();

	QDir dir;
	dir.mkpath(QString::fromStdString(platformFolder.GetAbsolutePathname()));
	
	bool result = true;
	int saveIndex = 0;
	//save aggregators node before save screens
	for (HIERARCHYTREENODESCONSTITER iter = GetChildNodes().begin();
		 iter != GetChildNodes().end();
		 ++iter)
	{
		HierarchyTreeAggregatorNode* node = dynamic_cast<HierarchyTreeAggregatorNode*>(*iter);
		if (!node)
			continue;

		QString path = GetScreenPath(node->GetName());
		
		YamlNode* aggregator = new YamlNode(YamlNode::TYPE_MAP);
		result &= node->Save(aggregator, path, saveAll);
		// Set additional index node for aggregator to keep its order after save
		aggregator->Set(YamlNode::SAVE_INDEX_NAME, saveIndex);
		aggregators->SetNodeToMap(node->GetName().toStdString(), aggregator);
		saveIndex++;
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
		result &= screenNode->Save(screenPath, saveAll);
		
		screens->AddValueToArray(screenNode->GetName().toStdString());
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
		if(node->GetName().compare(candidatName) == 0)
		{
			return true;
		}
	}
	return false;
}

void HierarchyTreePlatformNode::EnablePreview(int width, int height)
{
    previewWidth = width;
    previewHeight = height;
    isPreview = true;
}

void HierarchyTreePlatformNode::DisablePreview()
{
    isPreview = false;
}
