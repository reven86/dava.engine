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



#ifndef __NODES_PROPERTY_CONTROL_H__
#define __NODES_PROPERTY_CONTROL_H__

#include "DAVAEngine.h"
#include "PropertyList.h"

#include "CreatePropertyControl.h"

using namespace DAVA;


class NodesPropertyDelegate
{
public:
    
    virtual void NodesPropertyChanged(const String &forKey) = 0;
    
};

class NodesPropertyControl: public UIControl, public PropertyListDelegate, public CreatePropertyControlDelegate, public UIListDelegate
{
    static const int32 CELL_HEIGHT = 20;
protected:
    virtual ~NodesPropertyControl();
public:
    NodesPropertyControl(const Rect & rect, bool createNodeProperties);
    
    virtual void WillAppear();
    virtual void WillDisappear();

    virtual void ReadFrom(Entity *sceneNode);
    virtual void ReadFrom(DataNode *dataNode);
    


    void UpdateFieldsForCurrentNode();
	void UpdateMatricesForCurrentNode();

    
    virtual void OnStringPropertyChanged(PropertyList *forList, const String &forKey, const String &newValue);
    virtual void OnFloatPropertyChanged(PropertyList *forList, const String &forKey, float newValue);
    virtual void OnIntPropertyChanged(PropertyList *forList, const String &forKey, int newValue);
    virtual void OnBoolPropertyChanged(PropertyList *forList, const String &forKey, bool newValue);
    virtual void OnFilepathPropertyChanged(PropertyList *forList, const String &forKey, const FilePath &newValue);
    virtual void OnComboIndexChanged(PropertyList *forList, const String &forKey, int32 newItemIndex, const String &newItemKey);
    virtual void OnMatrix4Changed(PropertyList *forList, const String &forKey, const Matrix4 & matrix4);
    virtual void OnSectionExpanded(PropertyList *forList, const String &forKey, bool isExpanded);
    virtual void OnDistancePropertyChanged(PropertyList *forList, const String &forKey, float32 newValue, int32 index);
    virtual void OnSliderPropertyChanged(PropertyList *forList, const String &forKey, float32 newValue);

    
    
    void SetDelegate(NodesPropertyDelegate *delegate);
    
    virtual void NodeCreated(bool success, const String &name, int32 type, VariantType *defaultValue = NULL);
    
    
    virtual int32 ElementsCount(UIList * list);
	virtual UIListCell *CellAtIndex(UIList *list, int32 index);
	virtual int32 CellHeight(UIList * list, int32 index);
	virtual void OnCellSelected(UIList *forList, UIListCell *selectedCell);

    void SetWorkingScene(Scene *scene);
    
    virtual void SetSize(const Vector2 &newSize);
    
protected:

    int32 GetTrianglesForLodLayer(LodComponent::LodData *lodData);
    
    
    bool GetHeaderState(const String & headerName, bool defaultValue = true);
    void SetHeaderState(const String & headerName, bool newState);
    
    
    void AddChildLodSection();
    void ReleaseChildLodData();
    
    void SetChildLodDistances();
    void RestoreChildLodDistances();
    
    
    
    NodesPropertyDelegate *nodesDelegate;
    PropertyList *propertyList;
    
    bool createNodeProperties;
    
    UIButton *btnPlus;
    UIButton *btnMinus;

    void OnSetDistancesForLodNodes(BaseObject * object, void * userData, void * callerData);

    
    void OnPlus(BaseObject * object, void * userData, void * callerData);
    void OnMinus(BaseObject * object, void * userData, void * callerData);

	static const int32 PROP_CONTROL_ELEM_COUNT = 10;
    CreatePropertyControl *propControl;
    Entity *currentSceneNode;
    DataNode *currentDataNode;
    
    Scene *workingScene;

    
    UIList *deletionList;
    UIControl *listHolder;
    UIButton *btnCancel;
    void OnCancel(BaseObject * object, void * userData, void * callerData);
    
    Vector<LodComponent *>childLodComponents;
    Vector<float32 *>childDistances;
};



#endif // __NODES_PROPERTY_CONTROL_H__