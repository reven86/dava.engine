#ifndef __QUICKED_PACKAGE_SERIALIZER_H__
#define __QUICKED_PACKAGE_SERIALIZER_H__

#include "Base/BaseObject.h"

namespace DAVA {
    class YamlNode;
}

class PackageSerializer
{
public:
    PackageSerializer();
    virtual ~PackageSerializer();
    
    virtual void PutValue(const DAVA::String &name, const DAVA::VariantType &value) = 0;
    virtual void PutValue(const DAVA::String &name, const DAVA::String &value) = 0;
    virtual void PutValue(const DAVA::String &name, const DAVA::Vector<DAVA::String> &value) = 0;
    virtual void PutValue(const DAVA::String &value) = 0;
    
    virtual void BeginMap(const DAVA::String &name) = 0;
    virtual void BeginMap() = 0;
    virtual void EndMap() = 0;
    
    virtual void BeginArray(const DAVA::String &name) = 0;
    virtual void BeginArray() = 0;
    virtual void EndArray() = 0;
};

class YamlPackageSerializer : public PackageSerializer
{
public:
    YamlPackageSerializer();
    virtual ~YamlPackageSerializer();
    
    virtual void PutValue(const DAVA::String &name, const DAVA::VariantType &value) override;
    virtual void PutValue(const DAVA::String &name, const DAVA::String &value) override;
    virtual void PutValue(const DAVA::String &name, const DAVA::Vector<DAVA::String> &value) override;
    virtual void PutValue(const DAVA::String &value) override;

    virtual void BeginMap() override;
    virtual void BeginMap(const DAVA::String &name) override;
    virtual void EndMap() override;
    
    virtual void BeginArray() override;
    virtual void BeginArray(const DAVA::String &name) override;
    virtual void EndArray() override;
    
    DAVA::YamlNode *GetYamlNode() const;
    
private:
    DAVA::Vector<DAVA::YamlNode*> nodesStack;
};

#endif // __QUICKED_PACKAGE_SERIALIZER_H__
