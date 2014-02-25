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



#include "HierarchyTreeController.h"
#include "ScreenWrapper.h"
#include "DefaultScreen.h"

#include "BaseMetadata.h"
#include "MetadataFactory.h"
#include "LibraryController.h"
#include "CommandsController.h"
#include "PreviewController.h"
#include "ControlCommands.h"
#include "ReloadSpritesCommand.h"

#include "AlignDistribute/AlignDistributeManager.h"

HierarchyTreeController::HierarchyTreeController(QObject* parent) :
	QObject(parent)
{
	Clear();
}

HierarchyTreeController::~HierarchyTreeController()
{
	DisconnectFromSignals();
}

void HierarchyTreeController::ConnectToSignals()
{
	connect(CommandsController::Instance(), SIGNAL(UnsavedChangesNumberChanged()),
			this, SLOT(OnUnsavedChangesNumberChanged()));
}

void HierarchyTreeController::DisconnectFromSignals()
{
	disconnect(CommandsController::Instance(), SIGNAL(UnsavedChangesNumberChanged()),
			   this, SLOT(OnUnsavedChangesNumberChanged()));
}

bool HierarchyTreeController::Load(const QString& projectPath)
{
	CloseProject();
	
	bool res = hierarchyTree.Load(projectPath);
	if (res)
	{
		emit HierarchyTreeUpdated();
		emit ProjectLoaded();
	}
	return res;
}

bool HierarchyTreeController::SaveOnlyChangedScreens(const QString& projectPath)
{
	bool res = hierarchyTree.SaveOnlyChangedScreens(projectPath);
	if (res)
		emit ProjectSaved();
	return res;
}

bool HierarchyTreeController::SaveAll(const QString& projectPath)
{
	bool res = hierarchyTree.SaveAll(projectPath);
	if (res)
		emit ProjectSaved();
	return res;
}

bool HierarchyTreeController::HasUnsavedChanges() const
{
	return hierarchyTree.GetRootNode()->IsNeedSave();
}

List<HierarchyTreeScreenNode*> HierarchyTreeController::GetUnsavedScreens()
{
	return hierarchyTree.GetUnsavedScreens();
}

void HierarchyTreeController::UpdateSelection(const HierarchyTreePlatformNode* activePlatform,
											  const HierarchyTreeScreenNode* activeScreen)
{
	bool updateLibrary = false;
	if (this->activePlatform != activePlatform)
	{
		updateLibrary = true;
		ResetSelectedControl();
		this->activePlatform = (HierarchyTreePlatformNode*)activePlatform;
		if (this->activePlatform)
			this->activePlatform->ActivatePlatform();

        // The platform is changed - update the Localization System.
        UpdateLocalization(false);
		emit SelectedPlatformChanged(this->activePlatform);
	}
	if (this->activeScreen != activeScreen)
	{
		updateLibrary = true;
		if (this->activeScreen)
			this->activeScreen->RemoveSelection();
		
		ResetSelectedControl();
		this->activeScreen = (HierarchyTreeScreenNode*)activeScreen;
		emit SelectedScreenChanged(this->activeScreen);
	}
	if (updateLibrary)
		LibraryController::Instance()->UpdateLibrary();
}

void HierarchyTreeController::UpdateSelection(const HierarchyTreeNode* activeItem)
{
	emit SelectedTreeItemChanged(activeItem);
}

void HierarchyTreeController::ChangeItemSelection(HierarchyTreeControlNode* control)
{
	if (IsControlSelected(control))
		UnselectControl(control);
	else
		SelectControl(control);
}

void HierarchyTreeController::SelectControl(HierarchyTreeControlNode* control)
{
	if (IsControlSelected(control))
	{
		return;
	}
	
	//add selection
	InsertSelectedControlToList(control);
	UIControl* uiControl = control->GetUIObject();
	if (uiControl)
	{
		uiControl->SetDebugDraw(true);
		uiControl->SetDrawPivotPointMode(UIControl::DRAW_ONLY_IF_NONZERO);
		uiControl->SetDebugDrawColor(Color(1.f, 0, 0, 1.f));
	
		//YZ draw parent rect
		UIControl* parentUiControl = uiControl->GetParent();
		if (parentUiControl)
		{
			parentUiControl->SetDebugDrawColor(Color(0.55f, 0.55f, 0.55f, 1.f));
			parentUiControl->SetDebugDraw(true);
		}
	}
	
	emit AddSelectedControl(control);
	emit SelectedControlNodesChanged(activeControlNodes);
	UpdateSelection(control);
}

void HierarchyTreeController::UnselectControl(HierarchyTreeControlNode* control, bool emitSelectedControlNodesChanged)
{
	if (false == IsControlSelected(control))
	{
		return;
	}
	
	//remove selection
	RemoveSelectedControlFromList(control);

	UIControl* uiControl = control->GetUIObject();
	if (uiControl)
	{
		uiControl->SetDebugDraw(false);
		uiControl->SetState(uiControl->GetInitialState());
		uiControl->SetDrawPivotPointMode(UIControl::DRAW_NEVER);

		//YZ draw parent rect
		UIControl* parentToRemove = uiControl->GetParent();
		if (parentToRemove)
		{
			bool removeParentDraw = true;
			for (SELECTEDCONTROLNODES::iterator iter = activeControlNodes.begin(); iter != activeControlNodes.end(); ++iter)
			{
				HierarchyTreeControlNode* control = (*iter);
				UIControl* uiControl = control->GetUIObject();
				if (uiControl)
				{
					UIControl* parentUiControl = uiControl->GetParent();
					if (parentToRemove == uiControl ||
						parentToRemove == parentUiControl)
					{
						removeParentDraw = false;
						break;
					}
				}
			}
			if (removeParentDraw)
				parentToRemove->SetDebugDraw(false);
		}
	}
	emit RemoveSelectedControl(control);
	if (emitSelectedControlNodesChanged)
		emit SelectedControlNodesChanged(activeControlNodes);
}

void HierarchyTreeController::InsertSelectedControlToList(HierarchyTreeControlNode* control)
{
	SELECTEDCONTROLNODES::iterator iter = std::find(activeControlNodes.begin(), activeControlNodes.end(), control);
	if (iter == activeControlNodes.end())
	{
		activeControlNodes.push_back(control);
	}
}

void HierarchyTreeController::RemoveSelectedControlFromList(HierarchyTreeControlNode* control)
{
	SELECTEDCONTROLNODES::iterator iter = std::find(activeControlNodes.begin(), activeControlNodes.end(), control);
	if (iter != activeControlNodes.end())
	{
		activeControlNodes.erase(iter);
	}
}

bool HierarchyTreeController::IsControlSelected(HierarchyTreeControlNode* control) const
{
	SELECTEDCONTROLNODES::const_iterator iter = std::find(activeControlNodes.begin(), activeControlNodes.end(), control);
	return (iter != activeControlNodes.end());
}

void HierarchyTreeController::ResetSelectedControl()
{
	while (activeControlNodes.size())
	{
		UnselectControl(*(activeControlNodes.begin()), false);
	}
	
	emit SelectedControlNodesChanged(activeControlNodes);
}

void HierarchyTreeController::Clear()
{
	activePlatform = NULL;
    activeScreen = NULL;
	
	ResetSelectedControl();
	CleanupNodesDeletedFromScene();
}

HierarchyTreeNode::HIERARCHYTREENODEID HierarchyTreeController::CreateNewControl(const QString& strType, const QPoint& position)
{
	if (!activeScreen)
	{
		return HierarchyTreeNode::HIERARCHYTREENODEID_EMPTY;
	}
		
	HierarchyTreeNode* parentNode = activeScreen;
	Vector2 parentDelta(0, 0);
	if (activeControlNodes.size() == 1)
	{
		HierarchyTreeControlNode* parentControlNode = (*activeControlNodes.begin());
		parentNode = parentControlNode;
		//parentDelta = parentControlNode->GetUIObject()->GetPosition();
		parentDelta = parentControlNode->GetParentDelta();
	}
	
	Vector2 point = Vector2(position.x(), position.y());
	DefaultScreen* screen = ScreenWrapper::Instance()->GetActiveScreen();
	if (screen)
		point = screen->LocalToInternal(point);
	point -= parentDelta;
	
	// Can create.
	return CreateNewControl(strType, point, parentNode);
}

HierarchyTreeNode::HIERARCHYTREENODEID HierarchyTreeController::CreateNewControl(const QString& strType, const Vector2& position,
																				 HierarchyTreeNode* parentNode)
{
	// Create the control itself.
	String type = strType.toStdString();
	String newName = activeScreen->GetNewControlName(type);

    // Add the tree node - we need it before initializing control.
	HierarchyTreeControlNode* controlNode = LibraryController::Instance()->CreateNewControl(parentNode, strType,
																							QString::fromStdString(newName),
																							position);
	if (!controlNode)
	{
		return HierarchyTreeNode::HIERARCHYTREENODEID_EMPTY;
	}
	
	emit HierarchyTreeUpdated();
	ResetSelectedControl();
	SelectControl(controlNode);
	
	return controlNode->GetId();
}

void HierarchyTreeController::ReturnNodeToScene(HierarchyTreeNode* nodeToReturn)
{
	if (!nodeToReturn)
	{
		return;
	}

	nodeToReturn->ReturnTreeNodeToScene();
	UnregisterNodeDeletedFromScene(nodeToReturn);

	emit HierarchyTreeUpdated();
	ResetSelectedControl();
	
	HierarchyTreeControlNode* controlNode = dynamic_cast<HierarchyTreeControlNode*>(nodeToReturn);
	if (controlNode)
	{
		SelectControl(controlNode);
	}
}

void HierarchyTreeController::ReturnNodeToScene(const HierarchyTreeNode::HIERARCHYTREENODESLIST& nodesToReturn)
{
	
	for (HierarchyTreeNode::HIERARCHYTREENODESLIST::const_iterator iter = nodesToReturn.begin();
		 iter != nodesToReturn.end(); iter ++)
	{
		HierarchyTreeNode* nodeToReturn = (*iter);
		if (nodeToReturn)
		{
			nodeToReturn->ReturnTreeNodeToScene();
			UnregisterNodeDeletedFromScene(nodeToReturn);
		}
	}

	emit HierarchyTreeUpdated();
	ResetSelectedControl();

	// Select the first one, if any.
	if (nodesToReturn.size() == 0 )
	{
		return;
	}

	HierarchyTreeControlNode* controlNode = dynamic_cast<HierarchyTreeControlNode*>(nodesToReturn.front());
	if (controlNode)
	{
		SelectControl(controlNode);
	}
}

HierarchyTreePlatformNode* HierarchyTreeController::GetActivePlatform() const
{
	return activePlatform;
}

HierarchyTreeScreenNode* HierarchyTreeController::GetActiveScreen() const
{
	return activeScreen;
}

const HierarchyTreeController::SELECTEDCONTROLNODES& HierarchyTreeController::GetActiveControlNodes() const
{
	return activeControlNodes;
}

void HierarchyTreeController::EmitHierarchyTreeUpdated(bool needRestoreSelection)
{
    emit HierarchyTreeUpdated(needRestoreSelection);
}

bool HierarchyTreeController::NewProject(const QString& projectPath)
{
	hierarchyTree.CreateProject();
	
	bool res = SaveAll(projectPath);
	if (res)
	{
		emit ProjectCreated();
	}
	
	return res;
}

HierarchyTreePlatformNode* HierarchyTreeController::AddPlatform(const QString& name, const Vector2& size)
{
	HierarchyTreePlatformNode* platformNode = hierarchyTree.AddPlatform(name, size);
	if (platformNode)
	{
		UpdateSelection(platformNode, NULL);
	}

	EmitHierarchyTreeUpdated();
	
	return platformNode;
}

HierarchyTreeScreenNode* HierarchyTreeController::AddScreen(const QString& name, HierarchyTreeNode::HIERARCHYTREENODEID platform)
{
	HierarchyTreeScreenNode* screenNode = hierarchyTree.AddScreen(name, platform);
	if (screenNode)
	{
		UpdateSelection(screenNode->GetPlatform(), screenNode);
		EmitHierarchyTreeUpdated();
	}
	
	return screenNode;
}

HierarchyTreeAggregatorNode* HierarchyTreeController::AddAggregator(const QString& name, HierarchyTreeNode::HIERARCHYTREENODEID platform, const Rect& rect)
{
	HierarchyTreeAggregatorNode* screenNode = hierarchyTree.AddAggregator(name, platform, rect);
	if (screenNode)
	{
		EmitHierarchyTreeUpdated();
	}
	
	return screenNode;
}

void HierarchyTreeController::CloseProject()
{
	activeControlNodes.clear();
	ResetSelectedControl();
	UpdateSelection(NULL, NULL);
	// Need clean undo/redo stack before close project
	CommandsController::Instance()->CleanupUndoRedoStack();
	CleanupNodesDeletedFromScene();
	hierarchyTree.CloseProject();

	EmitHierarchyTreeUpdated();
	emit ProjectClosed();
}

void HierarchyTreeController::DeleteNode(const HierarchyTreeNode::HIERARCHYTREENODEID nodeID,
										 bool deleteNodeFromMemory, bool deleteNodeFromScene,
										 bool deleteNodeFromDisk)
{
	HierarchyTreeNode::HIERARCHYTREENODESLIST nodes;
	
	HierarchyTreeNode* nodeToDelete = GetTree().GetNode(nodeID);
	if (!nodeToDelete)
	{
		return;
	}

	nodes.push_back(nodeToDelete);
	DeleteNodes(nodes, deleteNodeFromMemory, deleteNodeFromScene, deleteNodeFromDisk);
}

void HierarchyTreeController::DeleteNodes(const HierarchyTreeNode::HIERARCHYTREENODESLIST& nodes,
										  bool deleteNodeFromMemory, bool deleteNodeFromScene,
										  bool deleteNodeFromDisk)
{
	if (!nodes.size())
		return;
	
	// Deletion of nodes might change the active platform or screen.
	this->activePlatformAfterDeleteNodes = this->activePlatform;
	this->activeScreenAfterDeleteNodes = this->activeScreen;

	DeleteNodesInternal(nodes);
	UpdateSelection(this->activePlatformAfterDeleteNodes, this->activeScreenAfterDeleteNodes);

	emit SelectedControlNodesChanged(activeControlNodes);
	
	if (deleteNodeFromScene)
	{
		RegisterNodesDeletedFromScene(nodes);
	}
	if (deleteNodeFromDisk)
	{
		DeleteNodesFiles(nodes);
	}

	hierarchyTree.DeleteNodes(nodes, deleteNodeFromMemory, deleteNodeFromScene);
	EmitHierarchyTreeUpdated();
}

void HierarchyTreeController::DeleteNodesFiles(const HierarchyTreeNode::HIERARCHYTREENODESLIST &nodes)
{
	HierarchyTreeNode::HIERARCHYTREENODESLIST::const_iterator iter;
	for (iter = nodes.begin(); iter != nodes.end(); ++iter)
	{
		HierarchyTreeScreenNode* screenNode = dynamic_cast<HierarchyTreeScreenNode*>(*iter);
		HierarchyTreePlatformNode* platformNode = dynamic_cast<HierarchyTreePlatformNode*>(*iter);

		if (screenNode)
		{
			screenNode->SetMarked(true);
			QString path = screenNode->GetPlatform()->GetScreenPath(screenNode->GetName());
			FileSystem::Instance()->DeleteFile(path.toStdString());
		}

		if (platformNode)
		{
			platformNode->SetMarked(true);
			platformNode->SetChildrenMarked(true);
			FileSystem::Instance()->DeleteDirectory(platformNode->GetPlatformFolder(), true);
		}
	}
}

void HierarchyTreeController::DeleteNodesInternal(const HierarchyTreeNode::HIERARCHYTREENODESLIST& nodes)
{
	const HierarchyTreePlatformNode* activePlatform = this->activePlatform;
	const HierarchyTreeScreenNode* activeScreen = this->activeScreen;
	
	HierarchyTreeNode::HIERARCHYTREENODESLIST::const_iterator iter;
	for (iter = nodes.begin(); iter != nodes.end(); ++iter)
	{
		if (activeScreen && activeScreen->GetId() == (*iter)->GetId())
		{
			activeScreen = NULL;
			this->activeScreenAfterDeleteNodes = NULL;
		}
	
		if (activePlatform && activePlatform->GetId() == (*iter)->GetId())
		{
			activePlatform = NULL;
			this->activePlatformAfterDeleteNodes = NULL;
		}

		HierarchyTreeControlNode* controlNode = dynamic_cast<HierarchyTreeControlNode*>((*iter));
		if (controlNode)
		{
			UnselectControl(controlNode);
			DeleteNodesInternal(controlNode->GetChildNodes());
		}
	}
}

void HierarchyTreeController::SynchronizeSelection(const QList<HierarchyTreeControlNode*>& selectedNodes)
{
    SELECTEDCONTROLNODES nodesToDeselect = activeControlNodes;

    foreach(HierarchyTreeControlNode* selectedNode, selectedNodes)
    {
        SelectControl(selectedNode);
        nodesToDeselect.remove(selectedNode);
    }

    // In case some nodes remain in the deselected list - unselect them.
    for (SELECTEDCONTROLNODES::iterator iter = nodesToDeselect.begin(); iter != nodesToDeselect.end();
         iter ++)
    {
        UnselectControl(*iter, false);
    }
}

void HierarchyTreeController::UpdateControlsData()
{
	 hierarchyTree.UpdateControlsData();
}

void HierarchyTreeController::UpdateLocalization(bool takePathFromLocalizationSystem)
{
    // Update the Active Platform.
    HierarchyTreePlatformNode* activePlatformNode = GetActivePlatform();
    if (!activePlatformNode)
    {
        return;
    }

    if (takePathFromLocalizationSystem)
    {
        // FROM LocalizationSystem TO Active Platform
        activePlatformNode->SetLocalizationPath(LocalizationSystem::Instance()->GetDirectoryPath());
        activePlatformNode->SetLocale(LocalizationSystem::Instance()->GetCurrentLocale());
    }
    else
    {
        // FROM Active Platform TO Localization System.
        const FilePath & localizationPath = activePlatformNode->GetLocalizationPath();
        const String& locale = activePlatformNode->GetLocale();

        if (localizationPath.IsEmpty() || locale.empty())
        {
            // No Localization Path is already set - cleanup the Localization System.
            LocalizationSystem::Instance()->Cleanup();
        }
        else
        {
            // Re-setup the Localization System with the values stored on Platform level.
            LocalizationSystem::Instance()->SetCurrentLocale(locale);
            LocalizationSystem::Instance()->InitWithDirectory(localizationPath);
        }
    }
    
    // Localization System is updated; need to look through all controls
    // and cause them to update their texts according to the new Localization.
    hierarchyTree.UpdateLocalization();
}

void HierarchyTreeController::RegisterNodesDeletedFromScene(const HierarchyTreeNode::HIERARCHYTREENODESLIST& nodes)
{
	HierarchyTreeNode::HIERARCHYTREENODESLIST::const_iterator iter;
	for (iter = nodes.begin(); iter != nodes.end(); ++iter)
	{
		RegisterNodeDeletedFromScene(*iter);
	}
}

void HierarchyTreeController::RegisterNodeDeletedFromScene(HierarchyTreeNode* node)
{
	deletedFromSceneNodes.insert(node);
	
}

void HierarchyTreeController::UnregisterNodeDeletedFromScene(HierarchyTreeNode* node)
{
	deletedFromSceneNodes.erase(node);
}

void HierarchyTreeController::CleanupNodesDeletedFromScene()
{
	// Cleanup the nodes deleted from scene, but not from memory.
	Logger::Debug("CLEANUP: Amount of nodes deleted from the scene %i", deletedFromSceneNodes.size());
	for (Set<HierarchyTreeNode*>::iterator iter = deletedFromSceneNodes.begin();
		 iter != deletedFromSceneNodes.end(); iter ++)
	{
		HierarchyTreeNode* node = (*iter);
		if (node)
		{
			delete(node);
		}
	}

	deletedFromSceneNodes.clear();
}

void HierarchyTreeController::OnUnsavedChangesNumberChanged()
{
	emit HierarchyTreeUpdated();
}

HierarchyTreeScreenNode* HierarchyTreeController::GetScreenNodeForNode(HierarchyTreeNode* node)
{
	bool foundScreen = false;
	HierarchyTreeNode* screen = node->GetParent();
	do
	{
		if (dynamic_cast<HierarchyTreeScreenNode*>(screen))
		{
			foundScreen = true;
		}
		else
		{
			screen = screen->GetParent();
		}
	} while (!foundScreen && screen);

	if (foundScreen)
	{
		return dynamic_cast<HierarchyTreeScreenNode*>(screen);
	}

	return NULL;
}

void HierarchyTreeController::AlignSelectedControls(eAlignControlsType alignType)
{
	if (!CanPerformAlign(alignType))
	{
		return;
	}

	BaseCommand* command = new ControlsAlignDistributeCommand(activeControlNodes, alignType);
    CommandsController::Instance()->ExecuteCommand(command);
	SafeRelease(command);
}

void HierarchyTreeController::DistributeSelectedControls(eDistributeControlsType distributeType)
{
	if (!CanPerformDistribute(distributeType))
	{
		return;
	}

	BaseCommand* command = new ControlsAlignDistributeCommand(activeControlNodes, distributeType);
    CommandsController::Instance()->ExecuteCommand(command);
	SafeRelease(command);
}

void HierarchyTreeController::AdjustSelectedControlsSize()
{
	BaseCommand* command = new ControlsAdjustSizeCommand(activeControlNodes);
    CommandsController::Instance()->ExecuteCommand(command);
	SafeRelease(command);
}

bool HierarchyTreeController::CanPerformAlign(eAlignControlsType /*alignType*/)
{
	// Align is not possible if less than two controls selected.
	return activeControlNodes.size() >= 2;
}

bool HierarchyTreeController::CanPerformDistribute(eDistributeControlsType /*distributeType*/)
{
	// Distribute is not possible if less than three controls selected.
	return activeControlNodes.size() >= 3;
}

void HierarchyTreeController::RepackAndReloadSprites()
{
    ReloadSpritesCommand* cmd = new ReloadSpritesCommand(hierarchyTree.GetRootNode());
    CommandsController::Instance()->ExecuteCommand(cmd);
    SafeRelease(cmd);
}

void HierarchyTreeController::EnablePreview(const PreviewSettingsData& data)
{
    if (PreviewController::Instance()->IsPreviewEnabled() || !activePlatform ||
        !activeScreen || !activeScreen->GetScreen())
    {
        return;
    }

    // We are entering Preview Mode - nothing should be selected.
    ResetSelectedControl();

    uint32 screenDPI = Core::Instance()->GetScreenDPI();
    const PreviewTransformData& transformData = PreviewController::Instance()->EnablePreview(data, activePlatform->GetSize(), screenDPI);
    activePlatform->EnablePreview(transformData.screenSize.x, transformData.screenSize.y);
    activeScreen->GetScreen()->SetSize(transformData.screenSize);

    emit SelectedScreenChanged(activeScreen);
}

void HierarchyTreeController::DisablePreview()
{
    if (!PreviewController::Instance()->IsPreviewEnabled() || !activePlatform ||
        !activeScreen || !activeScreen->GetScreen())
    {
        return;
    }

    PreviewController::Instance()->DisablePreview();
    activePlatform->DisablePreview();
    activeScreen->GetScreen()->SetSize(activePlatform->GetSize());
    
    emit SelectedScreenChanged(activeScreen);
}
