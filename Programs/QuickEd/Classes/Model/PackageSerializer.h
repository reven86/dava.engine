#ifndef __QUICKED_PACKAGE_SERIALIZER_H__
#define __QUICKED_PACKAGE_SERIALIZER_H__

#include "Base/BaseObject.h"

#include "PackageHierarchy/PackageVisitor.h"
#include "ControlProperties/PropertyVisitor.h"

#include <Base/Result.h>

class PackageBaseNode;
class AbstractProperty;
class ValueProperty;

class PackageSerializer : private PackageVisitor, private PropertyVisitor
{
public:
    PackageSerializer();
    virtual ~PackageSerializer();

    void SerializePackage(PackageNode* package);
    void SerializePackageNodes(PackageNode* package, const DAVA::Vector<ControlNode*>& controls, const DAVA::Vector<StyleSheetNode*>& styles);

    virtual void PutValue(const DAVA::String& name, const DAVA::String& value, bool quotes) = 0;
    virtual void PutValue(const DAVA::String& name, const DAVA::Vector<DAVA::String>& value) = 0;
    virtual void PutValue(const DAVA::String& value, bool quotes) = 0;

    virtual void BeginMap(const DAVA::String& name, bool quotes = false) = 0;
    virtual void BeginMap() = 0;
    virtual void EndMap() = 0;

    virtual void BeginArray(const DAVA::String& name, bool flow = false) = 0;
    virtual void BeginArray() = 0;
    virtual void EndArray() = 0;

    bool HasErrors() const;
    const DAVA::ResultList& GetResults() const;

private: // PackageVisitor
    void VisitPackage(PackageNode* node) override;
    void VisitImportedPackages(ImportedPackagesNode* node) override;
    void VisitControls(PackageControlsNode* node) override;
    void VisitControl(ControlNode* node) override;
    void VisitStyleSheets(StyleSheetsNode* node) override;
    void VisitStyleSheet(StyleSheetNode* node) override;

private:
    void AcceptChildren(PackageBaseNode* node);
    void CollectPackages(DAVA::Vector<PackageNode*>& packages, const ControlNode* node) const;
    bool IsControlNodeDependsOnStylesFromPackage(const ControlNode* node, const PackageNode* package) const;
    bool IsControlInSerializationList(const ControlNode* control) const;
    void CollectPrototypeChildrenWithChanges(const ControlNode* node, DAVA::Vector<ControlNode*>& out) const;
    bool HasNonPrototypeChildren(const ControlNode* node) const;

private: // PropertyVisitor
    void VisitRootProperty(RootProperty* property) override;

    void VisitControlSection(ControlPropertiesSection* property) override;
    void VisitComponentSection(ComponentPropertiesSection* property) override;

    void VisitNameProperty(NameProperty* property) override;
    void VisitPrototypeNameProperty(PrototypeNameProperty* property) override;
    void VisitClassProperty(ClassProperty* property) override;
    void VisitCustomClassProperty(CustomClassProperty* property) override;

    void VisitIntrospectionProperty(IntrospectionProperty* property) override;

    void VisitStyleSheetRoot(StyleSheetRootProperty* property) override;
    void VisitStyleSheetSelectorProperty(StyleSheetSelectorProperty* property) override;
    void VisitStyleSheetProperty(StyleSheetProperty* property) override;

private:
    void AcceptChildren(AbstractProperty* property);
    void PutValueProperty(const DAVA::String& name, ValueProperty* property);

private:
    DAVA::Vector<PackageNode*> importedPackages;
    DAVA::Vector<ControlNode*> controls;
    DAVA::Vector<ControlNode*> prototypes;
    DAVA::Vector<StyleSheetNode*> styles;

    DAVA::ResultList results;
};

#endif // __QUICKED_PACKAGE_SERIALIZER_H__
