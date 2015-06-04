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


#ifndef __UIEditor__HierarchyTreeScreenNode__
#define __UIEditor__HierarchyTreeScreenNode__

#include "HierarchyTreeNode.h"
#include "ScreenControl.h"

#include "Guides/GuidesManager.h"

using namespace DAVA;

class HierarchyTreeControlNode;
class HierarchyTreePlatformNode;

// "Screen" node for the Hierarchy Tree.
class HierarchyTreeScreenNode: public HierarchyTreeNode
{
public:
	static const float32 POSITION_UNDEFINED;

	HierarchyTreeScreenNode(HierarchyTreePlatformNode* parent, const QString& name);
	HierarchyTreeScreenNode(HierarchyTreePlatformNode* parent, const HierarchyTreeScreenNode* base, bool needLoad = true);
	~HierarchyTreeScreenNode();
	
	HierarchyTreePlatformNode* GetPlatform() const {return parent;};
	ScreenControl* GetScreen() const {return screen;};
	
	void SetScale(float scale);
	float GetScale() const;
	
	void SetPosX(int x);
	int GetPosX() const;
	void SetPosY(int y);
	int GetPosY() const;
	
	virtual void SetParent(HierarchyTreeNode* node, HierarchyTreeNode* insertAfter);
	virtual HierarchyTreeNode* GetParent();
	
	String GetNewControlName(const String& baseName) const;
	
	bool IsNameExist(const QString& name, const HierarchyTreeNode* parent) const;
	
	bool Load(const QString& path);
	bool Save(const QString& path, bool saveAll);
    bool Unload();
	
	virtual void ReturnTreeNodeToScene();
	virtual Rect GetRect() const;
	
	virtual void RemoveSelection() {};

	// Access to the screen unsaved changes counter.
	int32 GetUnsavedChanges() const {return unsavedChangesCounter;};

	virtual bool IsNeedSave() const;

    // Facade to the Guide Controller.
    // Methods related to the adding existing guide.
    void StartNewGuide(GuideData::eGuideType guideType);
    void MoveNewGuide(const Vector2& pos);
    
    bool CanAcceptNewGuide() const;
    const GuideData* AcceptNewGuide();
    void CancelNewGuide();

    // Methods related to the moving existing guide.
    bool StartMoveGuide(const Vector2& pos);
    void MoveGuide(const Vector2& pos);
    const GuideData* GetMoveGuide() const;

    // Get the active (selected or moved) guide.
    const GuideData* GetActiveGuide() const;

    Vector2 GetMoveGuideStartPos() const;
    const GuideData* AcceptMoveGuide();
    const GuideData* CancelMoveGuide();

    // Selected Guide methods.
    bool AreGuidesSelected() const;
    List<GuideData> DeleteSelectedGuides();

    // Calculate the stick to guides.
    int32 CalculateStickToGuides(const List<Rect>& controlsRectList, Vector2& offset) const;

    // Get the stick treshold.
    int32 GetGuideStickTreshold() const;

    // Access to the guides.
    void AddGuide(const GuideData& guideData);
    bool RemoveGuide(const GuideData& guideData);

    void SetGuidePosition(GuideData* guideData, const Vector2& newPos);
    bool UpdateGuidePosition(const GuideData& guideData, const Vector2& newPos);

    const List<GuideData*> GetGuides(bool includeNewGuide) const;
    const List<GuideData*> GetSelectedGuides(bool includeNewGuide = false) const;
    
    // Set the guide enabled mode.
    bool AreGuidesEnabled() const;
    void SetGuidesEnabled(bool value);

    // Lock/unlock guides.
    bool AreGuidesLocked() const;
    void LockGuides(bool value);

    // Get/set the stick mode.
    int32 GetStickMode() const;
    void SetStickMode(int32 stickMode);

    // Access to Guides Manager.
    const GuidesManager& GetGuidesManager() const;

    // Access to the list of control rects for this node.
    List<GuidesManager::StickedRect> GetControlRectsList(bool includeScreenBounds) const;
    void GetControlRectsListRecursive(const HierarchyTreeControlNode* rootNode, List<GuidesManager::StickedRect>& rectsList) const;
    
    bool IsLoaded() const {return loaded;}

protected:
    virtual Rect GetOwnRect() const;
	void CombineRectWithChild(Rect& rect) const;
    
    // Initialize the control before adding to hierarchy tree.
    virtual void InitializeControlBeforeAddingToTree(UIControl* uiControl);

private:
	void BuildHierarchyTree(HierarchyTreeNode* parent, List<UIControl*> child);
	
protected:
	HierarchyTreePlatformNode* parent;
	ScreenControl* screen;
	
    GuidesManager guides;

	float scale;
	int posX;
	int posY;
    bool loaded;
};

#endif /* defined(__UIEditor__HierarchyTreeScreenNode__) */
