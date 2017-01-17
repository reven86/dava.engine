#ifndef __UI_EDITOR_PACKAGE_CONTROLS_NODE_H__
#define __UI_EDITOR_PACKAGE_CONTROLS_NODE_H__

#include "PackageBaseNode.h"

#include "PackageVisitor.h"
#include "PackageControlsNode.h"
#include "ControlNode.h"

namespace DAVA
{
class UIPackage;
}

class PackageNode;

class PackageControlsNode : public ControlsContainerNode
{
public:
    PackageControlsNode(PackageNode* parent, const DAVA::String& name);
    virtual ~PackageControlsNode();

    void Add(ControlNode* node) override;
    void InsertAtIndex(int index, ControlNode* node) override;
    void Remove(ControlNode* node) override;
    int GetCount() const override;
    ControlNode* Get(int index) const override;

    void Accept(PackageVisitor* visitor) override;

    DAVA::String GetName() const override;

    virtual bool IsEditingSupported() const override;
    virtual bool IsInsertingControlsSupported() const override;
    virtual bool CanInsertControl(const ControlNode* node, DAVA::int32 pos) const override;
    virtual bool CanRemove() const override;
    virtual bool CanCopy() const override;

    void RefreshControlProperties();

    ControlNode* FindControlNodeByName(const DAVA::String& name) const;
    ControlNode* FindControlNodeByPath(const DAVA::String& path) const;

    DAVA::Vector<ControlNode*>::const_iterator begin() const override;
    DAVA::Vector<ControlNode*>::const_iterator end() const override;

    DAVA::Vector<ControlNode*>::iterator begin() override;
    DAVA::Vector<ControlNode*>::iterator end() override;

private:
    DAVA::Vector<ControlNode*> nodes;
    DAVA::String name;
};

#endif // __UI_EDITOR_PACKAGE_CONTROLS_NODE_H__
