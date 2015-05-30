#ifndef __QUICKED_PACKAGE_SERIALIZER_H__
#define __QUICKED_PACKAGE_SERIALIZER_H__

#include "Base/BaseObject.h"

#include "PackageHierarchy/PackageVisitor.h"

class PackageBaseNode;

class PackageSerializer : private PackageVisitor
{
public:
    PackageSerializer();
    virtual ~PackageSerializer();
    
    void SerializePackage(PackageNode *node);
    void SerializePackageNodes(PackageNode *node, const DAVA::Vector<ControlNode*> &nodes);
    
    virtual void PutValue(const DAVA::String &name, const DAVA::VariantType &value) = 0;
    virtual void PutValue(const DAVA::String &name, const DAVA::String &value) = 0;
    virtual void PutValue(const DAVA::String &name, const DAVA::Vector<DAVA::String> &value) = 0;
    virtual void PutValue(const DAVA::String &value) = 0;
    
    virtual void BeginMap(const DAVA::String &name) = 0;
    virtual void BeginMap() = 0;
    virtual void EndMap() = 0;
    
    virtual void BeginArray(const DAVA::String &name) = 0;
    virtual void BeginArray() = 0;
    virtual void EndArray() = 0;

    bool IsForceQualifiedName() const;
    void SetForceQualifiedName(bool qualifiedName);
    
private: // PackageVisitor
    void VisitPackage(PackageNode *node) override;
    void VisitImportedPackages(ImportedPackagesNode *node) override;
    void VisitControls(PackageControlsNode *node) override;
    void VisitControl(ControlNode *node) override;


private:
    void AcceptChildren(PackageBaseNode *node);
    void CollectPrototypeChildrenWithChanges(ControlNode *node, DAVA::Vector<ControlNode*> &out) const;
    bool HasNonPrototypeChildren(ControlNode *node) const;
    
private:
    bool forceQualifiedName;
};

#endif // __QUICKED_PACKAGE_SERIALIZER_H__
