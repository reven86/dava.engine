#ifndef __UI_EDITOR_CONTROL_NODE__
#define __UI_EDITOR_CONTROL_NODE__

#include "PackageBaseNode.h"
#include "ControlsContainerNode.h"

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
    ControlNode(DAVA::UIControl *control);
    ControlNode(ControlNode *node, eCreationType creationType);
    virtual ~ControlNode();

public:
    static ControlNode *CreateFromControl(DAVA::UIControl *control);
    static ControlNode *CreateFromPrototype(ControlNode *sourceNode);
    static ControlNode *CreateFromPrototypeChild(ControlNode *sourceNode);

public:
    ControlNode *Clone();
    
    void Add(ControlNode *node) override;
    void InsertAtIndex(int index, ControlNode *node) override;
    void Remove(ControlNode *node) override;
    int GetCount() const override;
    ControlNode *Get(int index) const override;
    void Accept(PackageVisitor *visitor) override;
    
    ControlNode *FindByName(const DAVA::String &name) const;
    
    virtual DAVA::String GetName() const override;
    DAVA::String GetQualifiedName(bool forceQualifiedName = false) const;
    
    DAVA::UIControl *GetControl() const;
    ControlNode *GetPrototype() const;
    const DAVA::Vector<ControlNode*> &GetInstances() const;

    virtual bool IsEditingSupported() const override;
    virtual bool IsInsertingSupported() const override;
    virtual bool CanInsertControl(ControlNode *node, DAVA::int32 pos) const override;
    virtual bool CanRemove() const override;
    virtual bool CanCopy() const override;

    eCreationType GetCreationType() const { return creationType; }

    RootProperty *GetRootProperty() const {return rootProperty; }
    void RefreshProperties();

    void MarkAsRemoved();
    void MarkAsAlive();

    DAVA::String GetPathToPrototypeChild(bool withRootPrototypeName = false) const;

private:
    bool IsInstancedFrom(const ControlNode *prototype) const;
    void AddControlToInstances(ControlNode *control);
    void RemoveControlFromInstances(ControlNode *control);

private:
    DAVA::UIControl *control;
    RootProperty *rootProperty;
    DAVA::Vector<ControlNode*> nodes;
    
    ControlNode *prototype;
    DAVA::Vector<ControlNode*> instances; // weak

    eCreationType creationType;
};


#endif // __UI_EDITOR_CONTROL_NODE__
