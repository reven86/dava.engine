#ifndef __UI_EDITOR_PACKAGE_CONTROLS_NODE_H__
#define __UI_EDITOR_PACKAGE_CONTROLS_NODE_H__

#include "PackageBaseNode.h"
#include "PackageControlsNode.h"
#include "ControlNode.h"

namespace DAVA
{
    class UIPackage;
}

class PackageSerializer;
class PackageNode;
class PackageRef;

class PackageControlsNode : public ControlsContainerNode
{
public:
    PackageControlsNode(PackageNode *parent, PackageRef *packageRef);
    virtual ~PackageControlsNode();
    
    void Add(ControlNode *node) override;
    void InsertAtIndex(int index, ControlNode *node) override;
    void Remove(ControlNode *node) override;
    int GetCount() const override;
    ControlNode *Get(int index) const override;

    DAVA::String GetName() const override;
    void SetName(const DAVA::String &name);
    
    virtual PackageRef *GetPackageRef() const override;
    
    int GetFlags() const override;

    virtual bool IsEditingSupported() const override;
    virtual bool IsInsertingSupported() const override;
    virtual bool CanInsertControl(ControlNode *node, DAVA::int32 pos) const override;
    virtual bool CanRemove() const override;
    virtual bool CanCopy() const override;

    ControlNode *FindControlNodeByName(const DAVA::String &name) const;
    void Serialize(PackageSerializer *serializer) const;
    void Serialize(PackageSerializer *serializer, const DAVA::Vector<ControlNode*> &nodes) const;

private:
    DAVA::UIPackage *GetPackage() const;

private:
    DAVA::String name;
    DAVA::Vector<ControlNode*> nodes;
    
    PackageRef *packageRef;
};

#endif // __UI_EDITOR_PACKAGE_CONTROLS_NODE_H__
