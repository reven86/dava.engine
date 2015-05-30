#ifndef __UI_EDITOR_IMPORTED_PACKAGES_NODE_H__
#define __UI_EDITOR_IMPORTED_PACKAGES_NODE_H__

#include "PackageBaseNode.h"

class PackageNode;
class PackageControlsNode;

class ImportedPackagesNode : public PackageBaseNode
{
public:
    ImportedPackagesNode(PackageBaseNode *parent);
    virtual ~ImportedPackagesNode();

    void Add(PackageNode *node);
    void InsertAtIndex(DAVA::int32 index, PackageNode *node);
    void Remove(PackageNode *node);
    PackageNode *GetImportedPackage(DAVA::int32 index) const;
    int GetCount() const override;
    PackageBaseNode *Get(DAVA::int32 index) const override;
    void Accept(PackageVisitor *visitor) override;

    virtual DAVA::String GetName() const override;
    bool CanInsertImportedPackage() const override;

    PackageNode *FindPackageByName(const DAVA::String &name) const;
    
protected:
    virtual bool IsReadOnly() const override;
    
private:
    DAVA::Vector<PackageNode*> packages;
};

#endif //__UI_EDITOR_IMPORTED_PACKAGES_NODE_H__
