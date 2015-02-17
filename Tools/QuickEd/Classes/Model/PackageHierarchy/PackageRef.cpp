#include "PackageRef.h"

#include "UI/UIPackage.h"

using namespace DAVA;

PackageRef::PackageRef(const FilePath &_path, UIPackage *_package)
    : path(_path)
    , package(SafeRetain(_package))
{
    name = path.GetBasename();
}

PackageRef::~PackageRef()
{
    SafeRelease(package);
}

String PackageRef::GetName() const
{
    return name;
}

const FilePath &PackageRef::GetPath() const
{
    return path;
}

bool PackageRef::IsPackageLoaded() const
{
    return package != nullptr;
}

UIPackage *PackageRef::GetPackage() const
{
    return package;
}
