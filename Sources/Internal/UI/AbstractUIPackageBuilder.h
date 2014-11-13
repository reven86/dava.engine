#ifndef __DAVAENGINE_UI_ABSTRACT_PACKAGE_LOADER_H__
#define __DAVAENGINE_UI_ABSTRACT_PACKAGE_LOADER_H__

#include "Base/BaseObject.h"
#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"

namespace DAVA
{
    
    class UIPackage;
    class UIControl;
    class UIControlBackground;
    class YamlNode;
    
    class AbstractUIPackageLoader
    {
    public:
        virtual UIPackage *LoadPackage(const FilePath &packagePath) = 0;
        virtual bool LoadControlByName(const String &name) = 0;
    };

    
    class AbstractUIPackageBuilder
    {
    public:
        AbstractUIPackageBuilder();
        virtual ~AbstractUIPackageBuilder();
        
        virtual UIPackage *BeginPackage(const FilePath &packagePath) = 0;
        virtual void EndPackage() = 0;
        
        virtual UIPackage *ProcessImportedPackage(const String &packagePath, AbstractUIPackageLoader *loader) = 0;
        
        virtual UIControl *BeginControlWithClass(const String &className) = 0;
        virtual UIControl *BeginControlWithCustomClass(const String &customClassName, const String &className) = 0;
        virtual UIControl *BeginControlWithPrototype(const String &packageName, const String &prototypeName, const String &customClassName, AbstractUIPackageLoader *loader) = 0;
        virtual UIControl *BeginControlWithPath(const String &pathName) = 0;
        virtual UIControl *BeginUnknownControl(const YamlNode *node) = 0;
        virtual void EndControl() = 0;
        
        virtual void BeginControlPropretiesSection(const String &name) = 0;
        virtual void EndControlPropertiesSection() = 0;
        
        virtual UIControlBackground *BeginBgPropertiesSection(int index, bool sectionHasProperties) = 0;
        virtual void EndBgPropertiesSection() = 0;
        
        virtual UIControl *BeginInternalControlSection(int index, bool sectionHasProperties) = 0;
        virtual void EndInternalControlSection() = 0;
        
        virtual void ProcessProperty(const InspMember *member, const VariantType &value) = 0;
    };
    
}

#endif // __DAVAENGINE_UI_ABSTRACT_PACKAGE_LOADER_H__
