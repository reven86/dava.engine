#ifndef __DAVAENGINE_UI_STYLESHEET_STRUCTS_H__
#define __DAVAENGINE_UI_STYLESHEET_STRUCTS_H__

#include "Base/IntrospectionBase.h"
#include "Base/BaseTypes.h"
#include "Base/FastName.h"
#include "FileSystem/VariantType.h"
#include "Animation/Interpolation.h"

namespace DAVA
{
enum class ePropertyOwner
{
    CONTROL,
    BACKGROUND,
    COMPONENT,
};

struct UIStyleSheetPropertyGroup
{
    String prefix;
    ePropertyOwner propertyOwner;
    uint32 componentType;
    const InspInfo* typeInfo;

    UIStyleSheetPropertyGroup(const String& prefix_, ePropertyOwner owner_, uint32 componentType_, const InspInfo* typeInfo_)
        : prefix(prefix_)
        , propertyOwner(owner_)
        , componentType(componentType_)
        , typeInfo(typeInfo_)
    {
    }
};

struct UIStyleSheetPropertyDescriptor
{
    UIStyleSheetPropertyGroup* group;

    FastName name;
    VariantType defaultValue;
    const InspMember* memberInfo = nullptr;

    UIStyleSheetPropertyDescriptor(UIStyleSheetPropertyGroup* group_, const FastName& name_, const VariantType& defaultValue_)
        : group(group_)
        , name(name_)
        , defaultValue(defaultValue_)
    {
    }

    String GetFullName() const
    {
        if (group->prefix.empty())
            return String(name.c_str());

        return group->prefix + String("-") + String(name.c_str());
    }
};

struct UIStyleSheetSelector
{
    UIStyleSheetSelector()
        :
        className("")
        ,
        name()
        ,
        stateMask(0)
    {
    }

    String className;
    FastName name;
    int32 stateMask;
    Vector<FastName> classes;
};

struct UIStyleSheetProperty
{
    UIStyleSheetProperty(uint32 aPropertyIndex, const VariantType& aValue, bool aTransition = false, Interpolation::FuncType aTransitionFunction = Interpolation::LINEAR, float32 aTransitionTime = 0.0f)
        :
        propertyIndex(aPropertyIndex)
        ,
        value(aValue)
        ,
        transitionFunction(aTransitionFunction)
        ,
        transitionTime(aTransitionTime)
        ,
        transition(aTransition)
    {
    }

    uint32 propertyIndex;
    VariantType value;

    Interpolation::FuncType transitionFunction;
    float32 transitionTime;
    bool transition;
};

struct UIStyleSheetClass
{
    UIStyleSheetClass(const FastName& tag_, const FastName& clazz_)
        : tag(tag_)
        , clazz(clazz_)
    {
    }

    FastName tag;
    FastName clazz;
};

class UIStyleSheetClassSet
{
public:
    bool AddClass(const FastName& clazz);
    bool RemoveClass(const FastName& clazz);
    bool HasClass(const FastName& clazz) const;
    bool SetTaggedClass(const FastName& tag, const FastName& clazz);
    bool ResetTaggedClass(const FastName& tag);

    bool RemoveAllClasses();

    String GetClassesAsString() const;
    void SetClassesFromString(const String& classes);

private:
    Vector<UIStyleSheetClass> classes;
};
};

#endif
