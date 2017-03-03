#include "AbstractUIPackageBuilder.h"
#include "UI/UIControl.h"

namespace DAVA
{
AbstractUIPackageBuilder::UIControlWithTypeInfo::UIControlWithTypeInfo(UIControl* control_)
    : control(control_)
{
}

AbstractUIPackageBuilder::UIControlWithTypeInfo::UIControlWithTypeInfo(const InspInfo* typeInfo_)
    : typeInfo(typeInfo_)
{
}

UIControl* AbstractUIPackageBuilder::UIControlWithTypeInfo::GetControl() const
{
    return control;
}

const InspInfo* AbstractUIPackageBuilder::UIControlWithTypeInfo::GetTypeInfo() const
{
    if (control)
    {
        return control->GetTypeInfo();
    }
    else
    {
        return typeInfo;
    }
}

AbstractUIPackageLoader::~AbstractUIPackageLoader()
{
}

AbstractUIPackageBuilder::AbstractUIPackageBuilder()
{
}

AbstractUIPackageBuilder::~AbstractUIPackageBuilder()
{
}
}
