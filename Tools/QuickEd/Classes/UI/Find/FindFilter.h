#pragma onces

#include "FileSystem/FilePath.h"
#include "Base/FastName.h"

class PackageInformation;
class ControlInformation;

class FindFilter
{
public:
    FindFilter();
    virtual ~FindFilter();

    virtual bool CanAcceptPackage(const std::shared_ptr<PackageInformation>& package) const = 0;
    virtual bool CanAcceptControl(const std::shared_ptr<ControlInformation>& control) const = 0;
};

class PrototypeUsagesFilter : public FindFilter
{
public:
    PrototypeUsagesFilter(const DAVA::String& packagePath, const DAVA::FastName& prototypeName);
    ~PrototypeUsagesFilter();

    bool CanAcceptPackage(const std::shared_ptr<PackageInformation>& package) const override;
    bool CanAcceptControl(const std::shared_ptr<ControlInformation>& control) const override;

private:
    DAVA::String packagePath;
    DAVA::FastName prototypeName;
};
