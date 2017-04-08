#ifndef __DAVAENGINE_UI_PACKAGE_LOADER_H__
#define __DAVAENGINE_UI_PACKAGE_LOADER_H__
#include "Base/BaseObject.h"
#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "UI/AbstractUIPackageBuilder.h"

namespace DAVA
{
class UIYamlLoader;
class UIControl;
class YamlNode;
class FilePath;
class UIPackage;
class UIControlBackground;

class UIPackageLoader : public AbstractUIPackageLoader
{
public:
    static const DAVA::int32 MIN_SUPPORTED_VERSION = 0;

    static const DAVA::int32 VERSION_WITH_LEGACY_ALIGNS = 0;
    static const DAVA::int32 LAST_VERSION_WITH_LINEAR_LAYOUT_LEGACY_ORIENTATION = 1;
    static const DAVA::int32 LAST_VERSION_WITH_LEGACY_SPRITE_MODIFICATION = 2;
    static const DAVA::int32 LAST_VERSION_WITHOUT_PROTOTYPES_SUPPORT = 5;

public:
    UIPackageLoader();
    UIPackageLoader(const DAVA::Map<DAVA::String, DAVA::Set<DAVA::FastName>>& legacyPrototypes);
    virtual ~UIPackageLoader();

public:
    virtual bool LoadPackage(const FilePath& packagePath, AbstractUIPackageBuilder* builder) override;
    virtual bool LoadPackage(const YamlNode* rootNode, const FilePath& packagePath, AbstractUIPackageBuilder* builder);
    virtual bool LoadControlByName(const FastName& name, AbstractUIPackageBuilder* builder) override;

private:
    struct ComponentNode
    {
        const YamlNode* node;
        uint32 type;
        uint32 index;
    };

private:
    void LoadStyleSheets(const YamlNode* styleSheetsNode, AbstractUIPackageBuilder* builder);
    void LoadControl(const YamlNode* node, AbstractUIPackageBuilder::eControlPlace controlPlace, AbstractUIPackageBuilder* builder);

    void LoadControlPropertiesFromYamlNode(UIControl* control, const Reflection& ref, const YamlNode* node, AbstractUIPackageBuilder* builder);

    void LoadComponentPropertiesFromYamlNode(UIControl* control, const YamlNode* node, AbstractUIPackageBuilder* builder);
    void ProcessLegacyAligns(UIControl* control, const YamlNode* node, AbstractUIPackageBuilder* builder);
    Vector<ComponentNode> ExtractComponentNodes(const YamlNode* node);

    virtual Any ReadAnyFromYamlNode(const Reflection& ref, const YamlNode* node, const String& name);

private:
    enum eItemStatus
    {
        STATUS_WAIT,
        STATUS_LOADING,
        STATUS_LOADED
    };

    struct QueueItem
    {
        FastName name;
        const YamlNode* node;
        int32 status;
    };

    Vector<QueueItem> loadingQueue;
    DAVA::int32 version = 0;

    DAVA::Map<DAVA::String, DAVA::String> legacyAlignsMap;
    DAVA::Map<DAVA::String, DAVA::Set<DAVA::FastName>> legacyPrototypes;
};
};

#endif // __DAVAENGINE_UI_PACKAGE_LOADER_H__
