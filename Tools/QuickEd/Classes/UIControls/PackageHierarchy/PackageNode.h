#ifndef __UI_EDITOR_PACKAGE_NODE_H__
#define __UI_EDITOR_PACKAGE_NODE_H__

#include "PackageBaseNode.h"

class ImportedPackagesNode;
class PackageControlsNode;
class PackageSerializer;

class PackageNode : public PackageBaseNode
{
public:
    PackageNode(DAVA::UIPackage *package);
private:
    virtual ~PackageNode();
public:
    virtual int GetCount() const override;
    virtual PackageBaseNode *Get(int index) const override;

    virtual DAVA::String GetName() const override;
    int GetFlags() const override;
    
    DAVA::UIPackage *GetPackage() const;
    ImportedPackagesNode *GetImportedPackagesNode() const;
    PackageControlsNode *GetPackageControlsNode() const;
    
    void Serialize(PackageSerializer *serializer) const;
    
private:
    DAVA::UIPackage *package;
    
    ImportedPackagesNode *importedPackagesNode;
    PackageControlsNode *packageControlsNode;
};

#endif // __UI_EDITOR_PACKAGE_NODE_H__
