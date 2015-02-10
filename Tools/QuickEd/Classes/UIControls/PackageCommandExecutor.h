#ifndef __QUICKED_PACKAGE_COMMAND_EXECUTOR_H__
#define __QUICKED_PACKAGE_COMMAND_EXECUTOR_H__

#include "Base/BaseObject.h"

class ControlNode;
class PackageControlsNode;
class PackageNode;

class PackageCommandExecutor : public DAVA::BaseObject
{
public:
    PackageCommandExecutor();
    virtual ~PackageCommandExecutor();
    
    virtual void InsertControlIntoPackage(ControlNode *control, PackageControlsNode *package) = 0;
    virtual void InsertControlIntoParentControl(ControlNode *control, ControlNode *parentControl) = 0;
    virtual void AddImportedPackageIntoPackage(PackageControlsNode *importedPackageControls, PackageNode *package) = 0;
};

class DefaultPackageCommandExecutor : public PackageCommandExecutor
{
public:
    DefaultPackageCommandExecutor();
    virtual ~DefaultPackageCommandExecutor();

    void InsertControlIntoPackage(ControlNode *control, PackageControlsNode *package) override;
    void InsertControlIntoParentControl(ControlNode *control, ControlNode *parentControl) override;
    void AddImportedPackageIntoPackage(PackageControlsNode *importedPackageControls, PackageNode *package) override;
};

#endif // __QUICKED_PACKAGE_COMMAND_EXECUTOR_H__
