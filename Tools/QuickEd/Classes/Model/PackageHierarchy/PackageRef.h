#ifndef __QUICKED_PACKAGE_REF_H__
#define __QUICKED_PACKAGE_REF_H__

#include "Base/BaseObject.h"

namespace DAVA
{
    class UIPackage;
}

class PackageRef : public DAVA::BaseObject
{
public:
    PackageRef(const DAVA::FilePath &path, DAVA::UIPackage *package);
    virtual ~PackageRef();

    DAVA::String GetName() const;
    const DAVA::FilePath &GetPath() const;
    bool IsPackageLoaded() const;
    DAVA::UIPackage *GetPackage() const;
    
private:
    DAVA::String name;
    DAVA::FilePath path;
    DAVA::UIPackage *package;
};

#endif // __QUICKED_PACKAGE_REF_H__
