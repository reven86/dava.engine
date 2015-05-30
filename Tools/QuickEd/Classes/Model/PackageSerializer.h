#ifndef __QUICKED_PACKAGE_SERIALIZER_H__
#define __QUICKED_PACKAGE_SERIALIZER_H__

#include "Base/BaseObject.h"

#include "PackageHierarchy/PackageVisitor.h"
#include "ControlProperties/PropertyVisitor.h"

class PackageBaseNode;
class AbstractProperty;

class PackageSerializer : private PackageVisitor, private PropertyVisitor
{
public:
    PackageSerializer();
    virtual ~PackageSerializer();
    
    void SerializePackage(PackageNode *package);
    void SerializePackageNodes(PackageNode *package, const DAVA::Vector<ControlNode*> &controls);
    
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

private: // PackageVisitor
    void VisitPackage(PackageNode *node) override;
    void VisitImportedPackages(ImportedPackagesNode *node) override;
    void VisitControls(PackageControlsNode *node) override;
    void VisitControl(ControlNode *node) override;

private:
    void AcceptChildren(PackageBaseNode *node);
    void CollectPackages(DAVA::Vector<PackageNode*> &packages, ControlNode *node) const;
    void CollectPrototypeChildrenWithChanges(ControlNode *node, DAVA::Vector<ControlNode*> &out) const;
    bool HasNonPrototypeChildren(ControlNode *node) const;

private: // PropertyVisitor
    void VisitRootProperty(RootProperty *property) override;
    
    void VisitControlSection(ControlPropertiesSection *property) override;
    void VisitComponentSection(ComponentPropertiesSection *property) override;
    void VisitBackgroundSection(BackgroundPropertiesSection *property) override;
    void VisitInternalControlSection(InternalControlPropertiesSection *property) override;

    void VisitNameProperty(NameProperty *property) override;
    void VisitPrototypeNameProperty(PrototypeNameProperty *property) override;
    void VisitClassProperty(ClassProperty *property) override;
    void VisitCustomClassProperty(CustomClassProperty *property) override;
    
    void VisitIntrospectionProperty(IntrospectionProperty *property) override;

private:
    void AcceptChildren(AbstractProperty *property);

private:
    DAVA::Vector<PackageNode*> importedPackages;
    DAVA::Vector<ControlNode*> controls;
};

#endif // __QUICKED_PACKAGE_SERIALIZER_H__
