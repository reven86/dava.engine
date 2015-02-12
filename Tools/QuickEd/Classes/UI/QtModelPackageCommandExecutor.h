#ifndef __QUICKED_QT_MODEL_PACKAGE_COMMAND_EXECUTOR_H__
#define __QUICKED_QT_MODEL_PACKAGE_COMMAND_EXECUTOR_H__

#include "UIControls/PackageCommandExecutor.h"

class PackageDocument;

class QtModelPackageCommandExecutor : public PackageCommandExecutor
{
public:
    QtModelPackageCommandExecutor(PackageDocument *_document);
    virtual ~QtModelPackageCommandExecutor();
    
    void InsertControlIntoPackage(ControlNode *control, PackageControlsNode *package) override;
    void InsertControlIntoParentControl(ControlNode *control, ControlNode *parentControl) override;
    void AddImportedPackageIntoPackage(PackageControlsNode *importedPackageControls, PackageNode *package) override;
    
    void ChangeProperty(ControlNode *node, BaseProperty *property, const DAVA::VariantType &value) override;
    void ResetProperty(ControlNode *node, BaseProperty *property) override;

private:
    PackageDocument *document;
};

#endif // __QUICKED_QT_MODEL_PACKAGE_COMMAND_EXECUTOR_H__
