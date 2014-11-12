#ifndef __DAVAENGINE_UI_PACKAGE_LOADER_H__
#define __DAVAENGINE_UI_PACKAGE_LOADER_H__
#include "Base/BaseObject.h"
#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"

#include "AbstractUIPackageBuilder.h"

namespace DAVA
{
class UIYamlLoader;
class UIControl;
class YamlNode;
class FilePath;
class UIPackage;
class UIControlFactory;
class UIControlBackground;
    
class UIPackageLoader : AbstractUIPackageLoader
{
public:
    UIPackageLoader(AbstractUIPackageBuilder *builder);
    virtual ~UIPackageLoader();

public:
    virtual UIPackage *LoadPackage(const FilePath &packagePath) override;
    virtual bool LoadControlByName(const String &name) override;

private:
    void LoadControl(const YamlNode *node, bool root);

    void LoadControlPropertiesFromYamlNode(UIControl *control, const InspInfo *typeInfo, const YamlNode *node);
    void LoadBgPropertiesFromYamlNode(UIControl *control, const YamlNode *node);
    void LoadInternalControlPropertiesFromYamlNode(UIControl *control, const YamlNode *node);
    virtual VariantType ReadVariantTypeFromYamlNode(const InspMember *member, const YamlNode *node);

private:
    enum eItemStatus {
        STATUS_WAIT,
        STATUS_LOADING,
        STATUS_LOADED
    };
    
    struct QueueItem {
        String name;
        const YamlNode *node;
        int status;
    };
    Vector<QueueItem> loadingQueue;
    AbstractUIPackageBuilder *builder;
};

};

#endif // __DAVAENGINE_UI_PACKAGE_LOADER_H__
