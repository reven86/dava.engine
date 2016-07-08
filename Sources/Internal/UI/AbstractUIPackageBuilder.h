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
    virtual ~AbstractUIPackageLoader();
    virtual bool LoadPackage(const FilePath& packagePath, AbstractUIPackageBuilder* builder) = 0;
    virtual bool LoadControlByName(const String& name, AbstractUIPackageBuilder* builder) = 0;
};

class AbstractUIPackageBuilder
{
public:
    AbstractUIPackageBuilder();
    virtual ~AbstractUIPackageBuilder();

    virtual void BeginPackage(const FilePath& packagePath) = 0;
    virtual void EndPackage() = 0;

    virtual bool ProcessImportedPackage(const String& packagePath, AbstractUIPackageLoader* loader) = 0;
    virtual void ProcessStyleSheet(const Vector<UIStyleSheetSelectorChain>& selectorChains, const Vector<UIStyleSheetProperty>& properties) = 0;

    virtual UIControl* BeginControlWithClass(const String& className) = 0;
    virtual UIControl* BeginControlWithCustomClass(const String& customClassName, const String& className) = 0;
    virtual UIControl* BeginControlWithPrototype(const String& packageName, const String& prototypeName, const String* customClassName, AbstractUIPackageLoader* loader) = 0;
    virtual UIControl* BeginControlWithPath(const String& pathName) = 0;
    virtual UIControl* BeginUnknownControl(const YamlNode* node) = 0;
    virtual void EndControl(bool isRoot) = 0;

    virtual void BeginControlPropertiesSection(const String& name) = 0;
    virtual void EndControlPropertiesSection() = 0;

    virtual UIComponent* BeginComponentPropertiesSection(uint32 componentType, uint32 componentIndex) = 0;
    virtual void EndComponentPropertiesSection() = 0;

    virtual UIControlBackground* BeginBgPropertiesSection(int32 index, bool sectionHasProperties) = 0;
    virtual void EndBgPropertiesSection() = 0;

    virtual UIControl* BeginInternalControlSection(int32 index, bool sectionHasProperties) = 0;
    virtual void EndInternalControlSection() = 0;

    virtual void ProcessProperty(const InspMember* member, const VariantType& value) = 0;
};
}

#endif // __DAVAENGINE_UI_ABSTRACT_PACKAGE_LOADER_H__
