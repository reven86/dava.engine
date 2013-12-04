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



#ifndef __GRAPH_BASE_H__
#define __GRAPH_BASE_H__

#include "DAVAEngine.h"
#include "NodesPropertyControl.h"

using namespace DAVA;

class GraphBaseDelegate
{
public:
    virtual bool LandscapeEditorActive() = 0;
    virtual NodesPropertyControl *GetPropertyControl(const Rect &rect) = 0;
};

//class EditorScene;
#include "../EditorScene.h"

class GraphBase: 
        public BaseObject, 
        public UIHierarchyDelegate, 
        public NodesPropertyDelegate
{
protected:
    virtual ~GraphBase();
public:
    GraphBase(GraphBaseDelegate *newDelegate, const Rect &rect);
    
    UIControl * GetGraphPanel();
    UIControl * GetPropertyPanel();
    
    void SetScene(EditorScene *scene);
    
    bool GraphOnScreen();
    bool PropertiesOnScreen();
    void UpdateMatricesForCurrentNode();
    
    virtual void SelectNode(BaseObject *node) = 0;
    virtual void UpdatePropertyPanel() = 0;
    
    virtual void RemoveWorkingNode() {};
    virtual void RefreshGraph();
    
    //NodesPropertyDelegate
    virtual void NodesPropertyChanged(const String& forKey);
    
protected:

    //NodesPropertyDelegate
    virtual UIHierarchyCell *CellForNode(UIHierarchy *forHierarchy, void *node);
    virtual void OnCellSelected(UIHierarchy *forHierarchy, UIHierarchyCell *selectedCell);
    
    virtual void FillCell(UIHierarchyCell *cell, void *node) = 0;
    virtual void SelectHierarchyNode(UIHierarchyNode * node) = 0;

    void OnRefreshPropertyControl(BaseObject * object, void * userData, void * callerData);
    
    virtual void CreateGraphPanel(const Rect &rect);
    void CreatePropertyPanel(const Rect &rect);

	// Returns TRUE if it is enough to rebuild selected node for this property.
	bool IsRebuildSelectedNodeEnough(const String& propertyName);

    UIControl *graphPanel;
    UIControl *propertyPanel;

    UIHierarchy * graphTree;

    NodesPropertyControl *propertyControl;
	Rect propertyPanelRect;
    
    GraphBaseDelegate *delegate;
    EditorScene *workingScene;
};



#endif // __GRAPH_BASE_H__