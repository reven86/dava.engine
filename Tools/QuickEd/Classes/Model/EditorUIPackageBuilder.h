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


#ifndef __EDITOR_UI_PACKAGE_BUILDER_H__
#define __EDITOR_UI_PACKAGE_BUILDER_H__

#include "UI/AbstractUIPackageBuilder.h"
#include "FileSystem/FilePath.h"
#include "UI/Styles/UIStyleSheetStructs.h"
#include "Model/ControlProperties/SectionProperty.h"

class PackageNode;
class ControlNode;
class StyleSheetNode;
class ControlsContainerNode;
class IntrospectionProperty;

class EditorUIPackageBuilder : public DAVA::AbstractUIPackageBuilder
{
public:
    EditorUIPackageBuilder();
    virtual ~EditorUIPackageBuilder();

    virtual void BeginPackage(const DAVA::FilePath &packagePath) override;
    virtual void EndPackage() override;
    
    virtual bool ProcessImportedPackage(const DAVA::String &packagePath, DAVA::AbstractUIPackageLoader *loader) override;
    virtual void ProcessStyleSheet(const DAVA::Vector<DAVA::UIStyleSheetSelectorChain> &selectorChains, const DAVA::Vector<DAVA::UIStyleSheetProperty> &properties) override;

    virtual DAVA::UIControl *BeginControlWithClass(const DAVA::String &className) override;
    virtual DAVA::UIControl *BeginControlWithCustomClass(const DAVA::String &customClassName, const DAVA::String &className) override;
    virtual DAVA::UIControl *BeginControlWithPrototype(const DAVA::String &packageName, const DAVA::String &prototypeName, const DAVA::String *customClassName, DAVA::AbstractUIPackageLoader *loader) override;
    virtual DAVA::UIControl *BeginControlWithPath(const DAVA::String &pathName) override;
    virtual DAVA::UIControl *BeginUnknownControl(const DAVA::YamlNode *node) override;
    virtual void EndControl(bool isRoot) override;
    
    virtual void BeginControlPropertiesSection(const DAVA::String &name) override;
    virtual void EndControlPropertiesSection() override;
    
    virtual DAVA::UIComponent *BeginComponentPropertiesSection(DAVA::uint32 componentType, DAVA::uint32 componentIndex) override;
    virtual void EndComponentPropertiesSection() override;
    
    virtual DAVA::UIControlBackground *BeginBgPropertiesSection(int index, bool sectionHasProperties) override;
    virtual void EndBgPropertiesSection() override;
    
    virtual DAVA::UIControl *BeginInternalControlSection(int index, bool sectionHasProperties) override;
    virtual void EndInternalControlSection() override;
    
    virtual void ProcessProperty(const DAVA::InspMember *member, const DAVA::VariantType &value) override;
    
    DAVA::RefPtr<PackageNode> BuildPackage() const;
    const DAVA::Vector<ControlNode*> &GetRootControls() const;
    const DAVA::Vector<PackageNode*> &GetImportedPackages() const;
    const DAVA::Vector<StyleSheetNode*> &GetStyles() const;
    void AddImportedPackage(PackageNode *node);

private:
    ControlNode *FindRootControl(const DAVA::String &name) const;

private:
    struct ControlDescr {
        ControlNode *node;
        bool addToParent;

        ControlDescr();
        ControlDescr(ControlNode *node, bool addToParent);
        ControlDescr(const ControlDescr &descr);
        ~ControlDescr();
        ControlDescr &operator=(const ControlDescr &descr);
    };
    
private:
    DAVA::FilePath packagePath;
    DAVA::List<ControlDescr> controlsStack;
    
    DAVA::Vector<PackageNode*> importedPackages;
    DAVA::Vector<ControlNode*> rootControls;
    DAVA::Vector<StyleSheetNode*> styleSheets;
    DAVA::Vector<DAVA::FilePath> declinedPackages;
    
    DAVA::BaseObject *currentObject;
    SectionProperty<IntrospectionProperty> *currentSection;
};

#endif // __EDITOR_UI_PACKAGE_BUILDER_H__
