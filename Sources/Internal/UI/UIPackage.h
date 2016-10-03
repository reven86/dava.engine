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
    static const int32 CURRENT_VERSION = 5;

    UIPackage();

protected:
    ~UIPackage();

public:
    int32 GetControlsCount() const;
    UIControl* GetControl(int32 index) const;
    UIControl* GetControl(const String& name) const;
    UIControl* GetControl(const FastName& name) const;

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

    void AddControl(UIControl* control);
    void RemoveControl(UIControl* control);

    UIControlPackageContext* GetControlPackageContext();

    RefPtr<UIPackage> Clone() const;

    Vector<UIControl*>::const_iterator begin() const;
    Vector<UIControl*>::const_iterator end() const;

    Vector<UIControl*>::iterator begin();
    Vector<UIControl*>::iterator end();

private:
    Vector<UIControl*> controls;

    UIControlPackageContext* controlPackageContext;
};
};
#endif // __DAVAENGINE_UI_PACKAGE_H__
