#include "PackageSerializer.h"

PackageSerializer::PackageSerializer()
    : forceQualifiedName(false)
{
    
}

PackageSerializer::~PackageSerializer()
{
 
    
}

bool PackageSerializer::IsForceQualifiedName() const
{
    return forceQualifiedName;
}

void PackageSerializer::SetForceQualifiedName(bool qualifiedName)
{
    forceQualifiedName = qualifiedName;
}
