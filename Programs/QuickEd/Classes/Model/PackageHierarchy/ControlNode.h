#ifndef __UI_EDITOR_CONTROL_NODE__
#define __UI_EDITOR_CONTROL_NODE__

#include "PackageBaseNode.h"
#include "ControlsContainerNode.h"

namespace DAVA
{
class UIControlPackageContext;
}

class PackageNode;
class RootProperty;

class ControlNode : public ControlsContainerNode
{
public:
    enum eCreationType
    {
        CREATED_FROM_CLASS,
        CREATED_FROM_PROTOTYPE,
        CREATED_FROM_PROTOTYPE_CHILD
    };

private:
    ControlNode(DAVA::UIControl* control, bool recursively);
    ControlNode(ControlNode* node, eCreationType creationType);
    virtual ~ControlNode();

public:
    static ControlNode* CreateFromControl(DAVA::UIControl* control);
    static ControlNode* CreateFromControlWithChildren(DAVA::UIControl* control);
    static ControlNode* CreateFromPrototype(ControlNode* sourceNode);
    static ControlNode* CreateFromPrototypeChild(ControlNode* sourceNode);

public:
    ControlNode* Clone();

    void Add(ControlNode* node) override;
    void InsertAtIndex(int index, ControlNode* node) override;
    void Remove(ControlNode* node) override;
    int GetCount() const override;
    ControlNode* Get(int index) const override;
    void Accept(PackageVisitor* visitor) override;

    ControlNode* FindByName(const DAVA::String& name) const;

    virtual DAVA::String GetName() const override;
    DAVA::UIControl* GetControl() const override;
    DAVA::UIControlPackageContext* GetPackageContext() const;
    void SetPackageContext(DAVA::UIControlPackageContext* context);

    ControlNode* GetPrototype() const;
    const DAVA::Vector<ControlNode*>& GetInstances() const;
    bool IsDependsOnPackage(PackageNode* package) const;

    virtual bool IsEditingSupported() const override;
    virtual bool IsInsertingControlsSupported() const override;
    virtual bool CanInsertControl(const ControlNode* node, DAVA::int32 pos) const override;
    virtual bool CanRemove() const override;
    virtual bool CanCopy() const override;
    bool CanMoveTo(const ControlsContainerNode* node, DAVA::int32 pos) const;

    eCreationType GetCreationType() const
    {
        return creationType;
    }

    RootProperty* GetRootProperty() const
    {
        return rootProperty;
    }
    void RefreshProperties();

    void MarkAsRemoved();
    void MarkAsAlive();

    DAVA::String GetPathToPrototypeChild() const;

    DAVA::Vector<ControlNode*>::const_iterator begin() const override;
    DAVA::Vector<ControlNode*>::const_iterator end() const override;

    DAVA::Vector<ControlNode*>::iterator begin() override;
    DAVA::Vector<ControlNode*>::iterator end() override;

private:
    bool IsInstancedFrom(const ControlNode* prototype) const;
    bool IsDependsOnPrototype(const ControlNode* prototype) const;
    bool IsParentOf(const ControlNode* node) const;
    void AddControlToInstances(ControlNode* control);
    void RemoveControlFromInstances(ControlNode* control);

private:
    DAVA::UIControl* control;
    RootProperty* rootProperty;
    DAVA::Vector<ControlNode*> nodes;

    ControlNode* prototype;
    DAVA::Vector<ControlNode*> instances; // weak

    eCreationType creationType;
};


#endif // __UI_EDITOR_CONTROL_NODE__
