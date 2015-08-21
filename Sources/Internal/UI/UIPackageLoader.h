/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


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
    
class UIPackageLoader : public AbstractUIPackageLoader
{
public:
    static const DAVA::int32 MIN_SUPPORTED_VERSION = 0;
    static const DAVA::int32 CURRENT_VERSION = 1;

    static const DAVA::int32 VERSION_WITH_LEGACY_ALIGNS = 0;
    
public:
    UIPackageLoader();
    virtual ~UIPackageLoader();

public:
    virtual bool LoadPackage(const FilePath &packagePath, AbstractUIPackageBuilder *builder) override;
    virtual bool LoadPackage(const YamlNode *rootNode, const FilePath &packagePath, AbstractUIPackageBuilder *builder);
    virtual bool LoadControlByName(const String &name, AbstractUIPackageBuilder *builder) override;

private:
    struct ComponentNode
    {
        const YamlNode *node;
        uint32 type;
        uint32 index;
    };
    
private:
    void LoadStyleSheets(const YamlNode *styleSheetsNode, AbstractUIPackageBuilder *builder);
    void LoadControl(const YamlNode *node, bool root, AbstractUIPackageBuilder *builder);

    void LoadControlPropertiesFromYamlNode(UIControl *control, const InspInfo *typeInfo, const YamlNode *node, AbstractUIPackageBuilder *builder);
    
    void LoadComponentPropertiesFromYamlNode(UIControl *control, const YamlNode *node, AbstractUIPackageBuilder *builder);
    void ProcessLegacyAligns(UIControl *control, const YamlNode *node, AbstractUIPackageBuilder *builder);
    Vector<ComponentNode> ExtractComponentNodes(const YamlNode *node);
    
    void LoadBgPropertiesFromYamlNode(UIControl *control, const YamlNode *node, AbstractUIPackageBuilder *builder);
    void LoadInternalControlPropertiesFromYamlNode(UIControl *control, const YamlNode *node, AbstractUIPackageBuilder *builder);
    virtual VariantType ReadVariantTypeFromYamlNode(const InspMember *member, const YamlNode *node, const DAVA::String &propertyName);

private:
    enum eItemStatus
    {
        STATUS_WAIT,
        STATUS_LOADING,
        STATUS_LOADED
    };
    
    struct QueueItem
    {
        String name;
        const YamlNode *node;
        int32 status;
    };
    
    Vector<QueueItem> loadingQueue;
    DAVA::int32 version = CURRENT_VERSION;
    
    DAVA::Map<DAVA::String, DAVA::String> legacyAlignsMap;
};

};

#endif // __DAVAENGINE_UI_PACKAGE_LOADER_H__
