#pragma onces

#include "FileSystem/FilePath.h"
#include "Base/FastName.h"

class PackageInformation;
class ControlInformation;

class FindFilter
{
public:
    FindFilter(const DAVA::String& packagePath, const DAVA::FastName& prototypeName);
    ~FindFilter();

    virtual bool CanAcceptPackage(const std::shared_ptr<PackageInformation>& package) const;
    virtual bool CanAcceptControl(const std::shared_ptr<ControlInformation>& control) const;

private:
    DAVA::String packagePath;
    DAVA::FastName prototypeName;
};
