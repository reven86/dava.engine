#pragma onces

#include "FileSystem/FilePath.h"

class ControlNode;

class PackageInformationBuilder;

class FindFilter
{
public:
    FindFilter(const DAVA::String& packagePath, const DAVA::String& prototypeName);
    ~FindFilter();

    virtual bool CanAcceptPackage(PackageInformationBuilder& builder) const;
    virtual bool CanAcceptRootControl(const ControlNode* node) const;
    virtual bool CanAcceptPrototype(const ControlNode* node) const;

private:
    DAVA::String packagePath;
    DAVA::String prototypeName;
};
