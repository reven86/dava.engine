#ifndef __QUICKED_PACKAGE_SERIALIZER_H__
#define __QUICKED_PACKAGE_SERIALIZER_H__

#include "Base/BaseObject.h"

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

    bool IsForceQualifiedName() const;
    void SetForceQualifiedName(bool qualifiedName);
    
private:
    bool forceQualifiedName;
};

#endif // __QUICKED_PACKAGE_SERIALIZER_H__
