#ifndef __DAVAENGINE_UI_PACKAGE_H__
#define __DAVAENGINE_UI_PACKAGE_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"

namespace DAVA
{
class UIControl;
class YamlNode;

class UIPackage: public BaseObject
{
public:
    UIPackage(const FilePath &packagePath);
protected:
    ~UIPackage();
public:
    String GetName() const;
    const FilePath &GetFilePath() const {
        return packagePath;
    }

    int32 GetControlsCount() const;
    UIControl *GetControl(int32 index) const;
    UIControl *GetControl(const String &name) const;
    
    template<class C>
    C GetControl(const String &name) const
    {
        return DynamicTypeCheck<C>(GetControl(name));
    }
    
    void AddControl(UIControl *control);
    void InsertControlBelow(UIControl *control, const UIControl *belowThis);
    void RemoveControl(UIControl *control);
    
    int32 GetPackagesCount() const;
    UIPackage *GetPackage(int32 index) const;
    UIPackage *GetPackage(const String name) const;
    void AddPackage(UIPackage *package);
    
protected:
    FilePath packagePath;
    Vector<UIControl *> controls;
    
    Vector<UIPackage*> importedPackages;
};

};
#endif // __DAVAENGINE_UI_PACKAGE_H__
