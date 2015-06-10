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

#include "TexturePacker/ResourcePacker2D.h"
#include "ResourcesManageHelper.h"

#include "AlignDistribute/AlignDistributeManager.h"
#include "ResourcesManageHelper.h"

#include "EditorFontManager.h"

QDir HierarchyTreeController::BaseUnusedItem::GetFullPath(const QString& baseDir,
                                                          const QString& dirName) const
{
    return QDir::cleanPath(QDir(ResourcesManageHelper::GetPlatformRootPath(baseDir)).filePath(dirName));
}

void HierarchyTreeController::PlatformUnusedItem::DeleteFromDisk(const QString& baseDir) const
{
    // Append a separator, since we are deleting the directory.
    QString platformPath = GetFullPath(baseDir, platformName).path() + QDir::separator();
    FileSystem::Instance()->DeleteDirectory(platformPath.toStdString());
}

void HierarchyTreeController::ScreenUnusedItem::DeleteFromDisk(const QString& baseDir) const
{
    QString platformPath = GetFullPath(baseDir, platformName).path() + QDir::separator();
    QString screenPath = QString("%1%2.yaml").arg(platformPath).arg(screenName);
    FileSystem::Instance()->DeleteFile(screenPath.toStdString());
}

HierarchyTreeController::HierarchyTreeController(QObject* parent) :
	QObject(parent)
{
	Clear();
}

HierarchyTreeController::~HierarchyTreeController()
{
	DisconnectFromSignals();
    CleanupUnusedItems();
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

void HierarchyTreeController::UpdateAggregators(const HierarchyTreePlatformNode* platform)
{
    if(platform)
    {
        const HierarchyTreeNode::HIERARCHYTREENODESLIST& childNodes = platform->GetChildNodes();
        for (HierarchyTreeNode::HIERARCHYTREENODESCONSTITER iter = childNodes.begin();
             iter != childNodes.end(); iter ++)
        {
            HierarchyTreeAggregatorNode* aggregatorNode = dynamic_cast<HierarchyTreeAggregatorNode*>(*iter);
            if (aggregatorNode)
            {
                aggregatorNode->UpdateHierarchyTree();
            }
        }
    }
}

void HierarchyTreeController::UpdateSelection(HierarchyTreePlatformNode* activePlatform, HierarchyTreeScreenNode* activeScreen)
{
    bool updateHierarchyTree = false;
    if(activeScreen && !activeScreen->IsLoaded())
    {
        static const uint32 maxLoadedScreenListSize = 10;
        loadedScreenList.push_back(activeScreen);
        
        // Screen was selected, load it now
        QString screenPath = ((HierarchyTreePlatformNode*)(activeScreen)->GetParent())->GetScreenPath(activeScreen->GetName());
        
        LocalizationSystem::Instance()->Cleanup();
        activeScreen->Load(screenPath);
      
        // Update the screen aggregators after screen is loaded.
        UpdateAggregators(activePlatform);

        updateHierarchyTree = true;

        hierarchyTree.UpdateControlsData(activeScreen);
        UpdateLocalization(false);
        // This is done to load fonts from old style ui yaml files.
        EditorFontManager::Instance()->OnProjectLoaded();
        
        if(loadedScreenList.size() > maxLoadedScreenListSize)
        {
            // Unload unused screens from queue
            uint32 screensToUnload = loadedScreenList.size() - maxLoadedScreenListSize;
            uint32 actualUnloaded = 0;
            List<HierarchyTreeScreenNode*>::iterator startIt = loadedScreenList.begin();
            List<HierarchyTreeScreenNode*>::iterator endIt = loadedScreenList.end();
            for(; startIt != endIt; ++startIt)
            {
                HierarchyTreeScreenNode* unloadedScreen = *(startIt);
                if(unloadedScreen != activeScreen)
                {
                    if(unloadedScreen->Unload())
                    {
                        loadedScreenList.erase(startIt);
                        ++actualUnloaded;
                    }
                }
                if(actualUnloaded >= screensToUnload)
                {
                    break;
                }
            }
        }
    }

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

        if (this->activeScreen)
        {
            this->activeScreen->SetStickMode(stickMode);
        }

		emit SelectedScreenChanged(this->activeScreen);
	}
	if (updateLibrary)
		LibraryController::Instance()->UpdateLibrary();
    
    if(updateHierarchyTree)
    {
        HierarchyTreeController::Instance()->EmitHierarchyTreeUpdated();
    }
}

void HierarchyTreeController::UpdateSelection(const HierarchyTreeNode* activeItem)
{
	emit SelectedTreeItemChanged(activeItem);
}

void HierarchyTreeController::ChangeItemSelection(HierarchyTreeControlNode* control,
                                                  eExpandControlType expandType)
{
	if (IsControlSelected(control))
		UnselectControl(control);
	else
		SelectControl(control, expandType);
}

void HierarchyTreeController::SelectControl(HierarchyTreeControlNode* control, eExpandControlType expandType)
{
	if (IsControlSelected(control))
	{
		return;
	}
	
	//add selection
	InsertSelectedControlToList(control);
	
	emit AddSelectedControl(control);
	emit SelectedControlNodesChanged(activeControlNodes, expandType);
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
    stickMode = (int32)NotSticked;

	ResetSelectedControl();
	CleanupNodesDeletedFromScene();
    CleanupUnusedItems();
}

HierarchyTreeNode::HIERARCHYTREENODEID HierarchyTreeController::CreateNewControl(HierarchyTreeNode::HIERARCHYTREENODEID typeId,
                                                                                const QPoint& position)
{
	if (!activeScreen)
	{
		return HierarchyTreeNode::HIERARCHYTREENODEID_EMPTY;
	}
		
	HierarchyTreeNode* parentNode = activeScreen;
	Vector2 parentDelta(0, 0);
    Matrix3 rotationMatrix;

	if (activeControlNodes.size() == 1)
	{
		HierarchyTreeControlNode* parentControlNode = (*activeControlNodes.begin());
		parentNode = parentControlNode;
        UIGeometricData parentGD = parentControlNode->GetUIObject()->GetGeometricData();
        Polygon2 polygon;
        parentGD.GetPolygon(polygon);
        parentDelta = polygon.points[0];
        float32 angle = parentGD.angle;
        if (!FLOAT_EQUAL(angle, 0.0f))
        {
            rotationMatrix.BuildRotation(-angle);
        }
    }
	
	Vector2 point = Vector2(position.x(), position.y());
	DefaultScreen* screen = ScreenWrapper::Instance()->GetActiveScreen();
	if (screen)
    {
		point = screen->LocalToInternal(point);
    }

	point -= parentDelta;
    point = point * rotationMatrix;

	// Can create.
	return CreateNewControl(typeId, point, parentNode);
}

HierarchyTreeNode::HIERARCHYTREENODEID HierarchyTreeController::CreateNewControl(HierarchyTreeNode::HIERARCHYTREENODEID typeId,
                                                                                const Vector2& position,
                                                                                HierarchyTreeNode* parentNode)
{
	// Create the control itself.
    // Add the tree node - we need it before initializing control.
	HierarchyTreeControlNode* controlNode = LibraryController::Instance()->CreateNewControl(parentNode, typeId, position);
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
    
    // add project path to res folders (to allow loading fonts before everything else)
    FilePath bundleName(projectPath.toStdString());
    bundleName.MakeDirectoryPathname();
    EditorFontManager::Instance()->SetProjectDataPath(bundleName.GetAbsolutePathname() + "Data/");
    EditorFontManager::Instance()->SetDefaultFontsPath(FilePath(bundleName.GetAbsolutePathname() + "Data/UI/Fonts/fonts.yaml"));
    
	
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
    loadedScreenList.clear();
	activeControlNodes.clear();
	ResetSelectedControl();
	UpdateSelection(NULL, NULL);
	// Need clean undo/redo stack before close project
	CommandsController::Instance()->CleanupUndoRedoStack();

	CleanupNodesDeletedFromScene();
    CleanupUnusedItems();
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
            FileSystem::Instance()->LockFile(path.toStdString(), false);
			FileSystem::Instance()->DeleteFile(path.toStdString());
		}

		if (platformNode)
		{
			platformNode->SetMarked(true);
			platformNode->SetChildrenMarked(true);

			FileList* platformFiles = new FileList(platformNode->GetPlatformFolder());
			int32 filesCount = platformFiles->GetCount();
			for (int32 i = 0; i < filesCount; i ++)
			{
				FileSystem::Instance()->LockFile(platformFiles->GetPathname(i), false);
			}
			platformFiles->Release();

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

void HierarchyTreeController::UpdateControlsData(const HierarchyTreeScreenNode* screenNode)
{
    hierarchyTree.UpdateControlsData(screenNode);
}

void  HierarchyTreeController::UpdateLocalization(bool takePathFromLocalizationSystem,
													const HierarchyTreeScreenNode* screenNode)
{
 	UpdateLocalizationInternal(takePathFromLocalizationSystem);
    // Localization System is updated; need to look through all controls
    // and cause them to update their texts according to the new Localization.
    hierarchyTree.UpdateLocalization(screenNode);
    ResetSelectedControl();
}

void  HierarchyTreeController::UpdateLocalizationInternal(bool takePathFromLocalizationSystem)
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
            LocalizationSystem::Instance()->SetDirectory(localizationPath);
            LocalizationSystem::Instance()->SetCurrentLocale(locale);
            LocalizationSystem::Instance()->Init();
        }
    }
}

void HierarchyTreeController::UpdateLocalization(bool takePathFromLocalizationSystem)
{
 	UpdateLocalizationInternal(takePathFromLocalizationSystem);
    // Localization System is updated; need to look through all controls
    // and cause them to update their texts according to the new Localization.
    hierarchyTree.UpdateLocalization();
    ResetSelectedControl();
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

 Set<String> HierarchyTreeController::RepackAndReloadSprites()
{
    Set<String> errorsSet;
    ResourcePacker2D *resPacker = new ResourcePacker2D();
	resPacker->InitFolders(ResourcesManageHelper::GetSpritesDatasourceDirectory().toStdString(),
                           ResourcesManageHelper::GetSpritesDirectory().toStdString());
    
    resPacker->PackResources(GPU_PNG);
    errorsSet = resPacker->GetErrors();
	SafeDelete(resPacker);

    Sprite::ReloadSprites();

    return errorsSet;
}

void HierarchyTreeController::EnablePreview(const PreviewSettingsData& data, bool applyScale)
{
    if (PreviewController::Instance()->IsPreviewEnabled() || !activePlatform ||
        !activeScreen || !activeScreen->GetScreen())
    {
        return;
    }

    // We are entering Preview Mode - nothing should be selected.
    ResetSelectedControl();
    PreviewController::Instance()->EnablePreview(applyScale);
    SetPreviewMode(data);
}

void HierarchyTreeController::SetPreviewMode(const PreviewSettingsData& data)
{
    uint32 screenDPI = Core::Instance()->GetScreenDPI();
    const PreviewTransformData& transformData = PreviewController::Instance()->SetPreviewMode(data, activePlatform->GetSize(true), screenDPI);

    activePlatform->SetPreviewMode(transformData.screenSize.x, transformData.screenSize.y);
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

void HierarchyTreeController::SetStickMode(int32 mode)
{
    stickMode = mode;
    if (activeScreen)
    {
        activeScreen->SetStickMode(mode);
    }
}

HierarchyTreeNode::HIERARCHYTREENODESLIST HierarchyTreeController::GetNodes() const
{
    return hierarchyTree.GetNodes();
}

void HierarchyTreeController::AddUnusedItem(BaseUnusedItem* item)
{
    if (!item)
    {
        DVASSERT(false);
        return;
    }

    unusedItems.push_back(item);
}

void HierarchyTreeController::CleanupUnusedItems()
{
    for (List<BaseUnusedItem*>::iterator iter = unusedItems.begin(); iter != unusedItems.end(); iter ++)
    {
        BaseUnusedItem* item = *iter;
        SafeDelete(item);
    }

    unusedItems.clear();
}

void HierarchyTreeController::DeleteUnusedItemsFromDisk(const QString& projectPath)
{
    for (List<BaseUnusedItem*>::iterator iter = unusedItems.begin(); iter != unusedItems.end(); iter ++)
    {
        (*iter)->DeleteFromDisk(projectPath);
    }

    CleanupUnusedItems();
}
