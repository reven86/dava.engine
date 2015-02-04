#ifndef __UI_EDITOR_PACKAGE_CONTROLS_NODE_H__
#define __UI_EDITOR_PACKAGE_CONTROLS_NODE_H__

#include "PackageBaseNode.h"
#include "ControlNode.h"

namespace DAVA
{
    class UIPackage;
}

class PackageSerializer;
class PackageNode;
class PackageRef;

class PackageControlsNode : public PackageBaseNode
{
public:
    PackageControlsNode(PackageNode *parent, PackageRef *packageRef);
    virtual ~PackageControlsNode();
    
    void Add(ControlNode *node);
    void InsertBelow(ControlNode *node, const ControlNode *belowThis);
    void Remove(ControlNode *node);
    virtual int GetCount() const override;
    virtual ControlNode *Get(int index) const override;

    virtual DAVA::String GetName() const;
    void SetName(const DAVA::String &name);
    
    PackageRef *GetPackageRef() const;
    
    virtual int GetFlags() const override;
    void SetReadOnly();

    ControlNode *FindControlNodeByName(const DAVA::String &name) const;
    void Serialize(PackageSerializer *serializer) const;
    

private:
    DAVA::UIPackage *GetPackage() const;

private:
    DAVA::String name;
    DAVA::Vector<ControlNode*> nodes;
    bool readOnly;
    
    PackageRef *packageRef;
};

#endif // __UI_EDITOR_PACKAGE_CONTROLS_NODE_H__
