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



#ifndef __UIEditor__HierarchyTreeController__
#define __UIEditor__HierarchyTreeController__

#include "DAVAEngine.h"

#include "HierarchyTree.h"

#include <QObject>
#include <QString>
#include <QPoint>
#include <QDir>

#include "HierarchyTreeScreenNode.h"
#include "HierarchyTreeControlNode.h"
#include "HierarchyTreePlatformNode.h"
#include "HierarchyTreeAggregatorNode.h"

#include "AlignDistribute/AlignDistributeEnums.h"

#include "PreviewController.h"

using namespace DAVA;

// Hierarchy Tree Controller for handling UI Editor Project Hierarchy Tree.
class HierarchyTreeController: public QObject, public Singleton<HierarchyTreeController>
{
	Q_OBJECT
	
public:
    // How the selected tree item should be expanded when selected.
    enum eExpandControlType
    {
        NoExpand,                       // No expanding needed
        DeferredExpand,                 // Deferred (delayed) force expand
        DeferredExpandWithMouseCheck,   // Deffered expand with check whether mouse is on the control.
        ImmediateExpand                 // Immediate expand.
    };

    // Unused hierarchy items - needed to delete them after save.
    class BaseUnusedItem
    {
        public:
            BaseUnusedItem() {};
            virtual ~BaseUnusedItem() {};

            // Delete the appropriate item data from disk.
            virtual void DeleteFromDisk(const QString& /*baseDir*/) const  = 0;

        protected:
            // Get the full path to the item based on the base directory.
            QDir GetFullPath(const QString& baseDir, const QString& dirName) const;
    };

    class PlatformUnusedItem : public BaseUnusedItem
    {
        public:
            PlatformUnusedItem(const QString& platform) :
                BaseUnusedItem(), platformName(platform) {};
        
            virtual void DeleteFromDisk(const QString& baseDir) const;
        
        protected:
            QString platformName;
    };

    class ScreenUnusedItem : public PlatformUnusedItem
    {
        public:
            ScreenUnusedItem(const QString& platform, const QString& screen) :
                PlatformUnusedItem(platform), screenName(screen)  {};

            virtual void DeleteFromDisk(const QString& baseDir) const;
        
        protected:
            QString screenName;
    };

	typedef List<HierarchyTreeControlNode*> SELECTEDCONTROLNODES;
	
	explicit HierarchyTreeController(QObject* parent = NULL);
    virtual ~HierarchyTreeController();

	void ConnectToSignals();
	void DisconnectFromSignals();

	bool NewProject(const QString& projectPath);
	bool Load(const QString& projectPath);

	// Perform the save for the changed only screens or for all screens.
	bool SaveOnlyChangedScreens(const QString& projectPath);
	bool SaveAll(const QString& projectPath);

	// Get the list of unsaved screens.
	List<HierarchyTreeScreenNode*> GetUnsavedScreens();

	void CloseProject();

	HierarchyTreePlatformNode* AddPlatform(const QString& name, const Vector2& size);
	HierarchyTreeScreenNode* AddScreen(const QString& name, HierarchyTreeNode::HIERARCHYTREENODEID platform);
	HierarchyTreeAggregatorNode* AddAggregator(const QString& name, HierarchyTreeNode::HIERARCHYTREENODEID platform, const Rect& rect);

	// Two separate versions of CreateNewControl method - by its position (when control is dropped
	// to the screen) and by its direct parent (when the control is dropped to the tree).
	HierarchyTreeNode::HIERARCHYTREENODEID CreateNewControl(HierarchyTreeNode::HIERARCHYTREENODEID typeId, const QPoint& position);
	HierarchyTreeNode::HIERARCHYTREENODEID CreateNewControl(HierarchyTreeNode::HIERARCHYTREENODEID typeId, const Vector2& position,																				 HierarchyTreeNode* parentNode);

	// Return any kind of node (one or multiple) back to the scene.
	void ReturnNodeToScene(HierarchyTreeNode* nodeToReturn);
	void ReturnNodeToScene(const HierarchyTreeNode::HIERARCHYTREENODESLIST& nodesToReturn);

	// Delete one node and several nodes.
	void DeleteNode(const HierarchyTreeNode::HIERARCHYTREENODEID nodeID, bool deleteNodeFromMemory, bool deleteNodeFromScene, bool deleteNodeFromDisk);
	void DeleteNodes(const HierarchyTreeNode::HIERARCHYTREENODESLIST& nodes, bool deleteNodeFromMemory, bool deleteNodeFromScene, bool deleteNodeFromDisk);
	void DeleteNodesFiles(const HierarchyTreeNode::HIERARCHYTREENODESLIST& nodes);

    const HierarchyTree& GetTree() const {return hierarchyTree;};
    
	void UpdateSelection(HierarchyTreePlatformNode* activePlatform,
                         HierarchyTreeScreenNode* activeScreen);

	void UpdateSelection(const HierarchyTreeNode* activeItem);
    void UpdateAggregators(const HierarchyTreePlatformNode* platform);
	
	void ChangeItemSelection(HierarchyTreeControlNode* control, eExpandControlType expandType = ImmediateExpand);
	void SelectControl(HierarchyTreeControlNode* control, eExpandControlType expandType = ImmediateExpand);
	void UnselectControl(HierarchyTreeControlNode* control, bool emitSelectedControlNodesChanged = true);
	bool IsControlSelected(HierarchyTreeControlNode* control) const;
	void ResetSelectedControl();
	
    // Synchronize the selection - select the nodes from the list, unselect the remainigns.
    void SynchronizeSelection(const QList<HierarchyTreeControlNode*>& selectedNodes);

	HierarchyTreePlatformNode* GetActivePlatform() const;
    HierarchyTreeScreenNode* GetActiveScreen() const;
	
    void EmitHierarchyTreeUpdated(bool needRestoreSelection = true);

    const SELECTEDCONTROLNODES& GetActiveControlNodes() const;

	// Loock through all controls and update their values
	void UpdateControlsData();
    void UpdateControlsData(const HierarchyTreeScreenNode* screenNode);

    // Look through all controls and update their localized texts.
    void UpdateLocalization(bool takePathFromLocalizationSystem);
    void UpdateLocalization(bool takePathFromLocalizationSystem, const HierarchyTreeScreenNode* screenNode);
	void UpdateLocalizationInternal(bool takePathFromLocalizationSystem);
    
	bool HasUnsavedChanges() const;

	HierarchyTreeScreenNode* GetScreenNodeForNode(HierarchyTreeNode* node);

	// Align/Distribute logic.
	void AlignSelectedControls(eAlignControlsType alignType);
	void DistributeSelectedControls(eDistributeControlsType distributeType);
	
	// Adjust control size logic
	void AdjustSelectedControlsSize();

    // Repack and reload sprites, return repacking errors.
    Set<String> RepackAndReloadSprites();
    
    // Preview mode control.
    void EnablePreview(const PreviewSettingsData& data, bool applyScale);
    void SetPreviewMode(const PreviewSettingsData& data);
    void DisablePreview();

    // Set the Stick Mode.
    void SetStickMode(int32 mode);

    // Access to the hierarchy tree nodes list.
    HierarchyTreeNode::HIERARCHYTREENODESLIST GetNodes() const;

    // Add the unused items.
    void AddUnusedItem(BaseUnusedItem* item);

    // Perform the physical deletion from the disk of the unused items.
    void DeleteUnusedItemsFromDisk(const QString& projectPath);

private:
	void DeleteNodesInternal(const HierarchyTreeNode::HIERARCHYTREENODESLIST& nodes);
	String GetNewControlName(const String& baseName);
	
signals:
	void ProjectCreated();
	void ProjectClosed();
	void ProjectLoaded();
	void ProjectSaved();
	void PlatformCreated();
//	void PlatformDeleted();
//	void ScreenCreated();
//	void Scree
	
	void HierarchyTreeUpdated(bool needRestoreSelection = true);
	void SelectedPlatformChanged(const HierarchyTreePlatformNode*);
	void SelectedScreenChanged(const HierarchyTreeScreenNode*);
	
	void AddSelectedControl(const HierarchyTreeControlNode*);
	void RemoveSelectedControl(const HierarchyTreeControlNode*);
	void SelectedControlNodesChanged(const HierarchyTreeController::SELECTEDCONTROLNODES &, HierarchyTreeController::eExpandControlType expandType = ImmediateExpand);
	
	void SelectedTreeItemChanged(const HierarchyTreeNode*);
	
protected slots:
	void OnUnsavedChangesNumberChanged();

protected:
	void Clear();
    void CleanupUnusedItems();

	// Register/unregister nodes removed from scene.
	void RegisterNodesDeletedFromScene(const HierarchyTreeNode::HIERARCHYTREENODESLIST& nodes);
	void RegisterNodeDeletedFromScene(HierarchyTreeNode* node);
	void UnregisterNodeDeletedFromScene(HierarchyTreeNode* node);

	// Cleanup the memory used by nodes removed from scene.
	void CleanupNodesDeletedFromScene();

	// Insert/remove selected control to the list with dups check.
	void InsertSelectedControlToList(HierarchyTreeControlNode* control);
	void RemoveSelectedControlFromList(HierarchyTreeControlNode* control);

	// Whether align/distribute is possible.
	bool CanPerformAlign(eAlignControlsType alignType);
	bool CanPerformDistribute(eDistributeControlsType distributeType);
    
    // Hierarchy Tree.
    HierarchyTree hierarchyTree;
    
	HierarchyTreePlatformNode* activePlatform;
    HierarchyTreeScreenNode* activeScreen;
    
	SELECTEDCONTROLNODES activeControlNodes;
	
	// Nodes deleted from the scene, but not from memory. Have to be
	// cleaned up separately.
	Set<HierarchyTreeNode*> deletedFromSceneNodes;
	
	// Active Platform/Active Screen after nodes deletion.
	HierarchyTreePlatformNode* activePlatformAfterDeleteNodes;
    HierarchyTreeScreenNode* activeScreenAfterDeleteNodes;

    // Stick mode set from MainWindow.
    int32 stickMode;
    
    // List of unused items to be deleted.
    List<BaseUnusedItem*> unusedItems;
    
    // List of loaded screens
    List<HierarchyTreeScreenNode*> loadedScreenList;
};

#endif /* defined(__UIEditor__HierarchyTreeController__) */
