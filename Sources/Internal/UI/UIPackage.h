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
    static const int32 CURRENT_VERSION = 6;

    UIPackage();

protected:
    ~UIPackage();

public:
    const Vector<UIControl*>& GetPrototypes() const;
    UIControl* GetPrototype(const String& name) const;
    UIControl* GetPrototype(const FastName& name) const;
    void AddPrototype(UIControl* prototype);
    void RemovePrototype(UIControl* control);

    int32 GetControlsCount() const;
    const Vector<UIControl*>& GetControls() const;
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

private:
    Vector<UIControl*> prototypes;
    Vector<UIControl*> controls;

    UIControlPackageContext* controlPackageContext;
};
};
#endif // __DAVAENGINE_UI_PACKAGE_H__
