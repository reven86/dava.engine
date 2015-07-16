/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#ifndef __QUICKED_PACKAGE_SERIALIZER_H__
#define __QUICKED_PACKAGE_SERIALIZER_H__

#include "Base/BaseObject.h"

#include "PackageHierarchy/PackageVisitor.h"
#include "ControlProperties/PropertyVisitor.h"

class PackageBaseNode;
class AbstractProperty;
class ValueProperty;

class PackageSerializer : private PackageVisitor, private PropertyVisitor
{
public:
    static const DAVA::int32 CURRENT_VERSION = 1;
    
public:
    PackageSerializer();
    virtual ~PackageSerializer();
    
    void SerializePackage(PackageNode *package);
    void SerializePackageNodes(PackageNode *package, const DAVA::Vector<ControlNode*> &controls, const DAVA::Vector<StyleSheetNode*> &styles);
    
    virtual void PutValue(const DAVA::String &name, const DAVA::VariantType &value) = 0;
    virtual void PutValue(const DAVA::String &name, const DAVA::String &value) = 0;
    virtual void PutValue(const DAVA::String &name, const DAVA::Vector<DAVA::String> &value) = 0;
    virtual void PutValue(const DAVA::VariantType &value) = 0;
    virtual void PutValue(const DAVA::String &value) = 0;
    
    virtual void BeginMap(const DAVA::String &name, bool quotes = false) = 0;
    virtual void BeginMap() = 0;
    virtual void EndMap() = 0;
    
    virtual void BeginArray(const DAVA::String &name, bool flow = false) = 0;
    virtual void BeginArray() = 0;
    virtual void EndArray() = 0;

private: // PackageVisitor
    void VisitPackage(PackageNode *node) override;
    void VisitImportedPackages(ImportedPackagesNode *node) override;
    void VisitControls(PackageControlsNode *node) override;
    void VisitControl(ControlNode *node) override;
    void VisitStyleSheets(StyleSheetsNode *node) override;
    void VisitStyleSheet(StyleSheetNode *node) override;

private:
    void AcceptChildren(PackageBaseNode *node);
    void CollectPackages(DAVA::Vector<PackageNode*> &packages, ControlNode *node) const;
    bool IsControlInSerializationList(ControlNode *control) const;
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

    void VisitStyleSheetRoot(StyleSheetRootProperty *property) override;
    void VisitStyleSheetSelectorProperty(StyleSheetSelectorProperty *property) override;
    void VisitStyleSheetProperty(StyleSheetProperty *property) override;
private:
    void AcceptChildren(AbstractProperty *property);
    void PutValueProperty(const DAVA::String &name, ValueProperty *property);

private:
    DAVA::Vector<PackageNode*> importedPackages;
    DAVA::Vector<ControlNode*> controls;
    DAVA::Vector<StyleSheetNode*> styles;
};

#endif // __QUICKED_PACKAGE_SERIALIZER_H__
