#include "PackageRef.h"

#include "UI/UIPackage.h"

using namespace DAVA;

PackageRef::PackageRef(const FilePath &_path)
    : path(_path)
{
    name = path.GetBasename();
}

PackageRef::~PackageRef()
{
}

String PackageRef::GetName() const
{
    return name;
}

const FilePath &PackageRef::GetPath() const
{
    return path;
}
