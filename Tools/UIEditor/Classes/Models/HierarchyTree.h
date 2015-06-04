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


#ifndef __UIEditor__HierarchyTree__
#define __UIEditor__HierarchyTree__

#include "HierarchyTreeRootNode.h"
#include "HierarchyTreePlatformNode.h"
#include "HierarchyTreeControlNode.h"
#include "HierarchyTreeAggregatorNode.h"

#include "BaseMetadata.h"

#include <QString>

using namespace DAVA;

// Hierarchy Tree.
class HierarchyTree
{
public:
	HierarchyTree();
    virtual ~HierarchyTree();

    bool Load(const QString& projectPath);
	
	bool SaveOnlyChangedScreens(const QString& projectPath);
	bool SaveAll(const QString& projectPath);

	const QString& GetActiveProjectPath() const {return rootNode.GetProjectDir();};
	
	void CreateProject();
	void CloseProject();

	HierarchyTreePlatformNode* AddPlatform(const QString& name, const Vector2& size);
	HierarchyTreeScreenNode* AddScreen(const QString& name, HierarchyTreeNode::HIERARCHYTREENODEID platformId);
	HierarchyTreeAggregatorNode* AddAggregator(const QString& name, HierarchyTreeNode::HIERARCHYTREENODEID platformId, const Rect& rect);

	void DeleteNodes(const HierarchyTreeNode::HIERARCHYTREENODESLIST& nodes, bool deleteNodeFromMemory,
					 bool deleteNodeFromScene);
 	
	const HierarchyTreeRootNode* GetRootNode() const {return &rootNode;};
   
	HierarchyTreeNode* GetNode(HierarchyTreeNode::HIERARCHYTREENODEID id) const;
	HierarchyTreeNode* GetNode(const UIControl* control) const;
	
	const HierarchyTreeNode::HIERARCHYTREENODESLIST& GetPlatforms() const;

    // Return all nodes in a tree.
    HierarchyTreeNode::HIERARCHYTREENODESLIST GetNodes() const;

	// Whether the project is created?
	bool IsProjectCreated() const {return projectCreated;};

	// Returns the list of screens currently unsaved. Might be empty.
	List<HierarchyTreeScreenNode*> GetUnsavedScreens();
	
	// Update properties for all controls
	void UpdateControlsData();
    void UpdateControlsData(const HierarchyTreeScreenNode* screenNode);

    // Update the localization for all controls.
    void UpdateLocalization();
    void UpdateLocalization(const HierarchyTreeScreenNode* screenNode);

	bool IsPlatformNamePresent(const QString& name) const ;

protected:
	// Do the save itself for only changed screens or for all screens.
	bool DoSave(const QString& projectPath, bool saveAll);

	// Insert the new scren node to the proper place.
	void InsertScreenNode(HierarchyTreePlatformNode* platformNode, HierarchyTreeScreenNode* screenNode);

    // Recursively traverse the nodes tree.
    void GetNodesRecursive(const HierarchyTreeNode* rootNode, HierarchyTreeNode::HIERARCHYTREENODESLIST& resultList) const;

    // Lock/unlock the project files.
    void LockProjectFiles(const List<QString>& fileNames);
    void UnlockProjectFiles(bool needCleanup);

private:
	void Clear();
    
    // Update Extra Data from/to the control's data.
    void UpdateExtraData(BaseMetadata::eExtraDataUpdateStyle updateStyle);
    void UpdateExtraData(const HierarchyTreeScreenNode* screenNode,BaseMetadata::eExtraDataUpdateStyle updateStyle);

    void UpdateExtraDataRecursive(HierarchyTreeControlNode* node, BaseMetadata::eExtraDataUpdateStyle updateStyle);

	// Update file/folder last modified date
	void UpdateModificationDate(const QString &path);

private:
	HierarchyTreeNode* FindNode(const HierarchyTreeNode* parent, HierarchyTreeNode::HIERARCHYTREENODEID id) const;
	HierarchyTreeNode* FindNode(const HierarchyTreeNode* parent, const UIControl* control) const;
    HierarchyTreeRootNode rootNode;
    
    // List of lock files for the project.
    List<String> projectLockedFiles;
	
	bool projectCreated;
};

#endif /* defined(__UIEditor__HierarchyTree__) */
