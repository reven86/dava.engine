#ifndef __UI_EDITOR_PACKAGE_CONTROLS_NODE_H__
#define __UI_EDITOR_PACKAGE_CONTROLS_NODE_H__

#include "PackageBaseNode.h"
#include "ControlNode.h"

class PackageSerializer;
class PackageNode;

class PackageControlsNode : public PackageBaseNode
{
public:
    PackageControlsNode(PackageNode *parent, DAVA::UIPackage *_package, const DAVA::FilePath &_packagePath);
    virtual ~PackageControlsNode();
    
    void Add(ControlNode *node);
    void InsertBelow(ControlNode *node, const ControlNode *belowThis);
    void Remove(ControlNode *node);
    virtual int GetCount() const override;
    virtual ControlNode *Get(int index) const override;

    virtual DAVA::String GetName() const;
    void SetName(const DAVA::String &name);
    
    DAVA::UIPackage *GetPackage() const;
    const DAVA::FilePath &GetPackagePath() const;
    
    virtual int GetFlags() const override;
    void SetReadOnly();

    ControlNode *FindControlNodeByName(const DAVA::String &name) const;
    void Serialize(PackageSerializer *serializer) const;
    

private:
    DAVA::String name;
    DAVA::Vector<ControlNode*> nodes;
    bool readOnly;
    
    DAVA::UIPackage *package;
    DAVA::FilePath packagePath;
};

#endif // __UI_EDITOR_PACKAGE_CONTROLS_NODE_H__
