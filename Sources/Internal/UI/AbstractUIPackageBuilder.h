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


#ifndef __DAVAENGINE_UI_ABSTRACT_PACKAGE_LOADER_H__
#define __DAVAENGINE_UI_ABSTRACT_PACKAGE_LOADER_H__

#include "Base/BaseObject.h"
#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "UI/Styles/UIStyleSheetSelectorChain.h"

namespace DAVA
{
    
class UIPackage;
class UIControl;
class UIStyleSheet;
class UIComponent;
class UIControlBackground;
class YamlNode;
class AbstractUIPackageBuilder;
    
class AbstractUIPackageLoader
{
public:
    virtual bool LoadPackage(const FilePath &packagePath, AbstractUIPackageBuilder *builder) = 0;
    virtual bool LoadControlByName(const String &name, AbstractUIPackageBuilder *builder) = 0;
};


class AbstractUIPackageBuilder
{
public:
    AbstractUIPackageBuilder();
    virtual ~AbstractUIPackageBuilder();

    virtual void BeginPackage(const FilePath &packagePath) = 0;
    virtual void EndPackage() = 0;
    
    virtual bool ProcessImportedPackage(const String &packagePath, AbstractUIPackageLoader *loader) = 0;
    virtual void ProcessStyleSheet(const Vector<UIStyleSheetSelectorChain> &selectorChains, const Vector<UIStyleSheetProperty> &properties) = 0;

    virtual UIControl *BeginControlWithClass(const String &className) = 0;
    virtual UIControl *BeginControlWithCustomClass(const String &customClassName, const String &className) = 0;
    virtual UIControl *BeginControlWithPrototype(const String &packageName, const String &prototypeName, const String *customClassName, AbstractUIPackageLoader *loader) = 0;
    virtual UIControl *BeginControlWithPath(const String &pathName) = 0;
    virtual UIControl *BeginUnknownControl(const YamlNode *node) = 0;
    virtual void EndControl(bool isRoot) = 0;
    
    virtual void BeginControlPropertiesSection(const String &name) = 0;
    virtual void EndControlPropertiesSection() = 0;
    
    virtual UIComponent *BeginComponentPropertiesSection(uint32 componentType, uint32 componentIndex) = 0;
    virtual void EndComponentPropertiesSection() = 0;
    
    virtual UIControlBackground *BeginBgPropertiesSection(int32 index, bool sectionHasProperties) = 0;
    virtual void EndBgPropertiesSection() = 0;
    
    virtual UIControl *BeginInternalControlSection(int32 index, bool sectionHasProperties) = 0;
    virtual void EndInternalControlSection() = 0;
    
    virtual void ProcessProperty(const InspMember *member, const VariantType &value) = 0;
};
    
}

#endif // __DAVAENGINE_UI_ABSTRACT_PACKAGE_LOADER_H__
