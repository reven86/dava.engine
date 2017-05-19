#ifndef __DAVAENGINE_UI_PACKAGE_H__
#define __DAVAENGINE_UI_PACKAGE_H__

#include "Base/BaseObject.h"

namespace DAVA
{
class UIControl;
class UIStyleSheet;
class UIControlPackageContext;

class UIPackage : public BaseObject
{
public:
    static const int32 CURRENT_VERSION = 14;

    UIPackage();

protected:
    ~UIPackage();

public:
    const Vector<UIControl*>& GetPrototypes() const;
    UIControl* GetPrototype(const String& name) const;
    UIControl* GetPrototype(const FastName& name) const;
    void AddPrototype(UIControl* prototype);
    void RemovePrototype(UIControl* control);

    const Vector<UIControl*>& GetControls() const;
    UIControl* GetControl(const String& name) const;
    UIControl* GetControl(const FastName& name) const;
    void AddControl(UIControl* control);
    void RemoveControl(UIControl* control);

    UIControlPackageContext* GetControlPackageContext();

    template <class C>
    C GetControl(const String& name) const
    {
        return DynamicTypeCheck<C>(GetControl(name));
    }

    template <class C>
    C GetControl(const FastName& name) const
    {
        return DynamicTypeCheck<C>(GetControl(name));
    }

    template <class C>
    C GetPrototype(const String& name) const
    {
        return DynamicTypeCheck<C>(GetPrototype(name));
    }

    template <class C>
    C GetPrototype(const FastName& name) const
    {
        return DynamicTypeCheck<C>(GetPrototype(name));
    }

private:
    Vector<UIControl*> prototypes;
    Vector<UIControl*> controls;

    UIControlPackageContext* controlPackageContext;
};
}
#endif // __DAVAENGINE_UI_PACKAGE_H__
