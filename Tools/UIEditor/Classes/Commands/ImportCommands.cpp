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


#include "ImportCommands.h"
#include "ScreenWrapper.h"
#include "HierarchyTreeAggregatorControlNode.h"
#include "HierarchyTreeController.h"
#include <QMessageBox>
#include <QSpacerItem>
#include <QGridLayout>

/////////////////////////////////////////////////////////////////////////////////////////////////////

ImportNodesCommand::ImportNodesCommand(const Vector<ImportDialog::FileItem>& files)
:	executionResult(false)
{
	Vector<ImportDialog::FileItem>::const_iterator it;
	for (it = files.begin(); it !=files.end(); ++it)
	{
		switch (it->action)
		{
			case ImportDialog::ACTION_AGGREGATOR:
				aggregatorFiles.push_back(*it);
				break;

			case ImportDialog::ACTION_SCREEN:
				screenFiles.push_back(*it);
				break;

			default:
				break;
		}
	}
}

void ImportNodesCommand::SetExecutionResult(bool result)
{
	executionResult = result;
}

bool ImportNodesCommand::GetExecutionResult()
{
	return executionResult;
}

bool ImportNodesCommand::IsUndoRedoSupported()
{
	return GetExecutionResult();
}

void ImportNodesCommand::IncrementUnsavedChanges()
{
	if (IsUndoRedoSupported() && redoNode)
	{
		redoNode->GetParent()->IncrementUnsavedChanges();
	}
}

void ImportNodesCommand::DecrementUnsavedChanges()
{
	if (IsUndoRedoSupported() && redoNode)
	{
		redoNode->GetParent()->DecrementUnsavedChanges();
	}
}

Set<QString> ImportNodesCommand::GetNodeAggregatorControls(const HierarchyTreeNode* node)
{
	Set<QString> nodeAggregatorControls;

	const HierarchyTreeNode::HIERARCHYTREENODESLIST& children = node->GetChildNodes();
	HierarchyTreeNode::HIERARCHYTREENODESLIST::const_iterator it;
	for (it = children.begin(); it != children.end(); ++it)
	{
		const HierarchyTreeAggregatorControlNode* childNode = dynamic_cast<const HierarchyTreeAggregatorControlNode*>(*it);
		if (childNode)
		{
			String name = childNode->GetAggregatorPath().GetBasename();
			nodeAggregatorControls.insert(QString::fromStdString(name));
		}

		Set<QString> childAggregatorControls = GetNodeAggregatorControls(*it);
		nodeAggregatorControls.insert(childAggregatorControls.begin(), childAggregatorControls.end());
	}

	return nodeAggregatorControls;
}

Set<QString> ImportNodesCommand::GetNodeAggregators(HierarchyTreeNode* node)
{
	Set<QString> nodeAggregators;

	const HierarchyTreeNode::HIERARCHYTREENODESLIST& children = node->GetChildNodes();
	HierarchyTreeNode::HIERARCHYTREENODESLIST::const_iterator it;
	for (it = children.begin(); it != children.end(); ++it)
	{
		const HierarchyTreeAggregatorNode* childNode = dynamic_cast<const HierarchyTreeAggregatorNode*>(*it);
		if (childNode)
		{
			nodeAggregators.insert(childNode->GetName());
		}
	}

	return nodeAggregators;
}

Set<QString> ImportNodesCommand::GetUnimportedAggregators(HierarchyTreeNode* node, HierarchyTreePlatformNode* platform)
{
	DVASSERT(platform && node);

	// Get aggregators, which the current node depends on
	Set<QString> nodeAggregators = GetNodeAggregatorControls(node);

	// Get aggregators, that are already loaded in platform
	Set<QString> platformAggregators = GetNodeAggregators(platform);

	// Add aggregators that are already imported by this command to the loaded list
	HierarchyTreeNode::HIERARCHYTREENODESCONSTITER it;
	for (it = importedNodes.begin(); it != importedNodes.end(); ++it)
	{
		if (dynamic_cast<HierarchyTreeAggregatorNode*>(*it))
		{
			platformAggregators.insert((*it)->GetName());
		}
		else
		{
			break;
		}
	}

	// Get list of unimported aggregators
	Set<QString> unimportedAggregators;
	std::set_difference(nodeAggregators.begin(), nodeAggregators.end(),
						platformAggregators.begin(), platformAggregators.end(),
						std::inserter(unimportedAggregators, unimportedAggregators.begin()));

	return unimportedAggregators;
}

bool ImportNodesCommand::LoadAggregators()
{
	HierarchyTreePlatformNode* platform = dynamic_cast<HierarchyTreePlatformNode*>(HierarchyTreeController::Instance()->GetTree().GetNode(platformId));
	if (!platform)
	{
		return false;
	}

	FilePath platformPath = platform->GetPlatformFolder();

	bool result = true;
	Vector<ImportDialog::FileItem>::const_iterator it;
	for (it = aggregatorFiles.begin(); it != aggregatorFiles.end(); ++it)
	{
		Rect rect(0, 0, it->size.width(), it->size.height());
		HierarchyTreeAggregatorNode* node = new HierarchyTreeAggregatorNode(platform, it->fileName, rect);

        FilePath filename = platformPath + (it->fileName.toStdString() + ".yaml");
		if (node->Load(rect, QString::fromStdString(filename.GetAbsolutePathname())) == false)
		{
			importErrorsAggregators.push_back(it->fileName);
			SafeDelete(node);
			result = false;

			continue;
		}

		node->SetMarked(true);
		importedNodes.push_back(node);
	}

	return result;
}

bool ImportNodesCommand::LoadScreens()
{
	HierarchyTreePlatformNode* platform = dynamic_cast<HierarchyTreePlatformNode*>(HierarchyTreeController::Instance()->GetTree().GetNode(platformId));
	if (!platform)
	{
		return false;
	}

	FilePath platformPath = platform->GetPlatformFolder();

	bool result = true;
	Vector<ImportDialog::FileItem>::const_iterator it;
	for (it = screenFiles.begin(); it != screenFiles.end(); ++it)
	{
		HierarchyTreeScreenNode* node = new HierarchyTreeScreenNode(platform, it->fileName);

        FilePath filename = platformPath + (it->fileName.toStdString() + ".yaml");
		if (node->Load(QString::fromStdString(filename.GetAbsolutePathname())) == false)
		{
			importErrorsScreens.insert(std::pair<QString, QString>(it->fileName, ""));
			SafeDelete(node);
			result = false;

			continue;
		}

		Set<QString> unimportedAggregators = GetUnimportedAggregators(node, platform);
		if (!unimportedAggregators.empty())
		{
			Set<QString>::const_iterator cIt;
			for (cIt = unimportedAggregators.begin(); cIt != unimportedAggregators.end(); ++cIt)
			{
				importErrorsScreens.insert(std::pair<QString, QString>(it->fileName, *cIt));
			}

			SafeDelete(node);
			result = false;

			continue;
		}

		node->SetMarked(true);
		importedNodes.push_back(node);
	}

	return result;
}

void ImportNodesCommand::ReplaceAggregatorControls()
{
	HierarchyTreeNode::HIERARCHYTREENODESITER it;
	for (it = importedNodes.begin(); it != importedNodes.end(); ++it)
	{
		HierarchyTreeAggregatorNode* node = dynamic_cast<HierarchyTreeAggregatorNode*>(*it);
		if (node)
		{
			node->UpdateHierarchyTree();
		}
		else
		{
			break;
		}
	}
}

void ImportNodesCommand::ShowErrorMessage(const QString& message)
{
	QString errorMessage = message;
	QString errorDetails = "";

	if (!importErrorsAggregators.empty())
	{
		errorDetails = QObject::tr("Error importing aggregators:\n");
		Vector<QString>::const_iterator it;
		for (it = importErrorsAggregators.begin(); it != importErrorsAggregators.end(); ++it)
		{
			errorDetails += (*it) + " - error loading file\n";
		}
		errorDetails += "\n";
	}

	if (!importErrorsScreens.empty())
	{
		errorDetails += QObject::tr("Error importing screens:\n");

		MultiMap<QString, QString>::const_iterator it;
		for (it = importErrorsScreens.begin();
			 it != importErrorsScreens.end();
			 it = importErrorsScreens.upper_bound(it->first))
		{
			if (it->second == "")
			{
				errorDetails += "   " + it->first + " - error loading file\n";
			}
			else
			{
				errorDetails += "   " + it->first + " - missing aggregators:\n";
				MultiMap<QString, QString>::const_iterator it2;
				for (it2 = it; it2 != importErrorsScreens.upper_bound(it->first); ++it2)
				{
					errorDetails += "      " + it2->second + "\n";
				}
			}
		}
	}

	QMessageBox msg;
	if (errorDetails != "")
	{
		msg.setInformativeText(QObject::tr("\nSee details for more information"));
		msg.setDetailedText(errorDetails);
	}
	msg.setText(errorMessage);
	msg.setStandardButtons(QMessageBox::Ok);
	QSpacerItem* horizontalSpacer = new QSpacerItem(350, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
	QGridLayout* layout = (QGridLayout*)msg.layout();
	layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());
	msg.exec();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

ImportScreensCommand::ImportScreensCommand(HierarchyTreeNode::HIERARCHYTREENODEID platformId,
										   const Vector<ImportDialog::FileItem>& files)
:	ImportNodesCommand(files)
{
	this->platformId = platformId;
}

void ImportScreensCommand::Execute()
{
	// importedNodes could be empty only for first executing of command
	if (importedNodes.empty())
	{
		SetExecutionResult(false);

		bool loaded = LoadAggregators();
		loaded &= LoadScreens();

		if (!loaded)
		{
			ShowErrorMessage("Import error!");
			Cleanup();
			return;
		}

		SetParents();
		ReplaceAggregatorControls();

		PrepareRemoveFromSceneInformationForNodes();
		redoNode = importedNodes.front();

		SetExecutionResult(true);
	}
	else
	{
		ReturnNodesToScene();
	}
}

void ImportScreensCommand::Rollback()
{
	HierarchyTreeController::Instance()->DeleteNodes(importedNodes, false, true, false);
}

void ImportScreensCommand::PrepareRemoveFromSceneInformationForNodes()
{
	HierarchyTreeNode::HIERARCHYTREENODESITER it;
	for (it = importedNodes.begin(); it != importedNodes.end(); ++it)
	{
		(*it)->PrepareRemoveFromSceneInformation();
	}
}

void ImportScreensCommand::ReturnNodesToScene()
{
	HierarchyTreeNode::HIERARCHYTREENODESITER it;
	for (it = importedNodes.begin(); it != importedNodes.end(); ++it)
	{
		HierarchyTreeController::Instance()->ReturnNodeToScene(*it);
	}
}

void ImportScreensCommand::SetParents()
{
	HierarchyTreePlatformNode* platform = dynamic_cast<HierarchyTreePlatformNode*>(HierarchyTreeController::Instance()->GetTree().GetNode(platformId));
	if (!platform)
	{
		return;
	}

	HierarchyTreeNode::HIERARCHYTREENODESITER it;
	for (it = importedNodes.begin(); it != importedNodes.end(); ++it)
	{
		(*it)->SetParent(platform, NULL);
	}
}

void ImportScreensCommand::Cleanup()
{
	HierarchyTreeNode::HIERARCHYTREENODESITER it;
	for (it = importedNodes.begin(); it != importedNodes.end(); ++it)
	{
		SafeDelete((*it));
	}
	importedNodes.clear();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

ImportPlatformCommand::ImportPlatformCommand(const QString& platformPath,
											 const QString& platformName,
											 const QSize& platformSize,
											 const Vector<ImportDialog::FileItem>& files)
:	ImportNodesCommand(files)
{
	this->platformPath = platformPath;
	this->platformName = platformName;
	this->platformSize = platformSize;
}

void ImportPlatformCommand::Execute()
{
	if (!redoNode)
	{
		SetExecutionResult(false);

		redoNode = HierarchyTreeController::Instance()->AddPlatform(platformName,
																	Vector2(platformSize.width(),
																			platformSize.height()));
		platformId = redoNode->GetId();

		bool loaded = LoadAggregators();
		loaded &= LoadScreens();

		if (!loaded)
		{
			ShowErrorMessage("Error importing " + platformName + "!");
			Cleanup();
			return;
		}

		HierarchyTreeNode::HIERARCHYTREENODESITER it;
		for (it = importedNodes.begin(); it != importedNodes.end(); ++it)
		{
			redoNode->AddTreeNode(*it);
		}
		
		// After project loaded - update all controls properties data
		HierarchyTreeController::Instance()->UpdateControlsData();

		ReplaceAggregatorControls();

		SetExecutionResult(true);
		redoNode->SetMarked(true);
		redoNode->SetChildrenMarked(true);

		PrepareRemoveFromSceneInformation();
	}
	else
	{
		ReturnRedoNodeToScene();
	}
}

void ImportPlatformCommand::Rollback()
{
	if (this->redoNode)
	{
		HierarchyTreeController::Instance()->DeleteNode(this->redoNode->GetId(), false, true, false);
	}
}

void ImportPlatformCommand::Cleanup()
{
	if (redoNode)
	{
		redoNode->GetParent()->RemoveTreeNode(redoNode, true, true);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////