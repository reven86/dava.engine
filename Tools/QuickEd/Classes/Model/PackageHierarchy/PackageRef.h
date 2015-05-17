#ifndef __QUICKED_PACKAGE_REF_H__
#define __QUICKED_PACKAGE_REF_H__

#include "Base/BaseObject.h"

class PackageRef : public DAVA::BaseObject
{
public:
    PackageRef(const DAVA::FilePath &path);
    virtual ~PackageRef();

    DAVA::String GetName() const;
    const DAVA::FilePath &GetPath() const;
    
private:
    DAVA::String name;
    DAVA::FilePath path;
};

#endif // __QUICKED_PACKAGE_REF_H__
