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


#ifndef __UIEditor__PropertiesGridController__
#define __UIEditor__PropertiesGridController__

#include <QObject>

#include "HierarchyTreeController.h"
#include "HierarchyTreeNode.h"
#include "HierarchyTreeControlNode.h"
#include "BaseMetadata.h"

#include "UI/UIControl.h"

namespace DAVA {

// A controller for Properties Grid.
class PropertiesGridController : public QObject, public Singleton<PropertiesGridController>
{
    Q_OBJECT
public:
    PropertiesGridController(QObject* parent = 0);
    ~PropertiesGridController();
    
    // Get the active tree node or nodes list.
    const HierarchyTreeNode* GetActiveTreeNode() const;
    const HierarchyTreeController::SELECTEDCONTROLNODES &GetActiveTreeNodesList() const;

    // Access to the active UI Control State.
    const Vector<UIControl::eControlState> &GetActiveUIControlStates() const;
    void SetActiveUIControlStates(const Vector<UIControl::eControlState>& newStates);

signals:
    // Generated when Properties Grid is updated and needs to be re-built.
    void PropertiesGridUpdated();
    
    // Generated when UI Controls are deselected and Properties Grid is about to be cleaned.
    void UIControlsDeselected();
    
    // Generated when UI Control State is changed.
    void SelectedUIControlStatesChanged(const Vector<UIControl::eControlState>& newState);

public slots:
    // Emitted by Hierarchy Tree Controller when selected control is changed.
    void OnSelectedTreeItemChanged(const HierarchyTreeNode* selectedNode);
    
    // Emitted by Hierarchy Tree Controller when Control Nodes selection is changed
    // (can contain one or more Control Nodes).
    void OnSelectedControlNodesChanged(const HierarchyTreeController::SELECTEDCONTROLNODES &selectedNodes, HierarchyTreeController::eExpandControlType expandType);
    
    // Selected State is changed.
	void OnSelectedStateChanged(UIControl::eControlState newState);
	void OnSelectedStatesChanged(const Vector<UIControl::eControlState>& newStates);

protected:
    // Connect to the signals.
    void ConnectToSignals();

    // Update the Properties Grid for the single node and for multiple nodes.
    void UpdatePropertiesForSingleNode(const HierarchyTreeNode* selectedNode);
    void UpdatePropertiesForUIControlNodeList(const HierarchyTreeController::SELECTEDCONTROLNODES &selectedNodes);
    
    // Handle situation when nothing in tree is selectted.
    void HandleNothingSelected();

    // Cleanup the selection remembered.
    void CleanupSelection();
    
private:
    // Tree Node currently selected.
    const HierarchyTreeNode* activeNode; // TODO! REMOVE THIS!!!

    // List of Tree Nodes currently selected.
    HierarchyTreeController::SELECTEDCONTROLNODES activeNodes;
    
    // Active UI Control State.
	Vector<UIControl::eControlState> activeUIControlStates;
    
    //Current localization files directory
    QString localizationDirectoryPath;
};
    
}

#endif /* defined(__UIEditor__PropertiesGridController__) */
