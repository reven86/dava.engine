#pragma once

#include "Settings/Settings.h"

#include <Base/Singleton.h>
#include <Base/IntrospectionBase.h>
#include <Base/FastNameMap.h>
#include <FileSystem/VariantType.h>

#include <QObject>

struct SettingsNode
{
    DAVA::VariantType value;
    DAVA::InspDesc desc;
};

enum class RenderingBackend : int
{
    DX11 = 0,
    DX9,
    OpenGL
};

class SettingsManager : public DAVA::Singleton<SettingsManager>
{
public:
    SettingsManager();
    ~SettingsManager();

    static DAVA::VariantType GetValue(const DAVA::FastName& path);
    static void SetValue(const DAVA::FastName& path, const DAVA::VariantType& value);

    static DAVA::VariantType GetValue(const DAVA::String& path)
    {
        return GetValue(DAVA::FastName(path));
    }
    static void SetValue(const DAVA::String& path, const DAVA::VariantType& value)
    {
        SetValue(DAVA::FastName(path), value);
    }

    static size_t GetSettingsCount();
    static DAVA::FastName GetSettingsName(size_t index);
    static SettingsNode* GetSettingsNode(const DAVA::FastName& name);

    static void ResetPerProjectSettings();

    static void ResetToDefault();

    DAVA_DEPRECATED(static void UpdateGPUSettings());

protected:
    DAVA::Vector<DAVA::FastName> settingsOrder;
    DAVA::FastNameMap<SettingsNode> settingsMap;

    void Init();
    void Save();
    void Load();
    void CreateValue(const DAVA::FastName& path, const DAVA::VariantType& defaultValue, const DAVA::InspDesc& description = DAVA::InspDesc(""));
    DAVA_DEPRECATED(bool CustomTextureViewGPULoad(const DAVA::String& paramName, const DAVA::VariantType& src_value, DAVA::VariantType& dstValue));
};
