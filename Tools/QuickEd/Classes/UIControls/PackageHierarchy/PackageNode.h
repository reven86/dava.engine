#ifndef __UI_EDITOR_PACKAGE_NODE_H__
#define __UI_EDITOR_PACKAGE_NODE_H__

#include "PackageBaseNode.h"

class ImportedPackagesNode;
class PackageControlsNode;
class PackageSerializer;
class ControlNode;
class PackageRef;

class PackageNode : public PackageBaseNode
{
public:
    PackageNode(PackageRef *_packageRef);
private:
    virtual ~PackageNode();
    
public:
    virtual int GetCount() const override;
    virtual PackageBaseNode *Get(int index) const override;

    virtual DAVA::String GetName() const override;
    int GetFlags() const override;
    
    PackageRef *GetPackageRef() const;
    
    ImportedPackagesNode *GetImportedPackagesNode() const;
    PackageControlsNode *GetPackageControlsNode() const;
    
    void Serialize(PackageSerializer *serializer) const;
    
    void AddControlWithResolvingDependencies(ControlNode *sourceControl);
    
private:
    void CollectPackages(DAVA::Set<DAVA::FilePath> &packages, ControlNode *node) const;
    
private:
    PackageRef *packageRef;
    
    ImportedPackagesNode *importedPackagesNode;
    PackageControlsNode *packageControlsNode;
};

#endif // __UI_EDITOR_PACKAGE_NODE_H__
