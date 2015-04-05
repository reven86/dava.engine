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
    UIPackageLoader(AbstractUIPackageBuilder *builder);
    virtual ~UIPackageLoader();

public:
    virtual UIPackage *LoadPackage(const FilePath &packagePath) override;
    virtual UIPackage *LoadPackage(const YamlNode *rootNode, const FilePath &packagePath);
    virtual bool LoadControlByName(const String &name) override;

private:
    void LoadControl(const YamlNode *node, bool root);

    void LoadControlPropertiesFromYamlNode(UIControl *control, const InspInfo *typeInfo, const YamlNode *node);
    void LoadComponentPropertiesFromYamlNode(UIControl *control, const YamlNode *node);
    void LoadBgPropertiesFromYamlNode(UIControl *control, const YamlNode *node);
    void LoadInternalControlPropertiesFromYamlNode(UIControl *control, const YamlNode *node);
    virtual VariantType ReadVariantTypeFromYamlNode(const InspMember *member, const YamlNode *node);

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
    AbstractUIPackageBuilder *builder;
};

};

#endif // __DAVAENGINE_UI_PACKAGE_LOADER_H__
