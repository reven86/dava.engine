#pragma once

#include "TArc/Controls/ControlProxy.h"
#include "TArc/Controls/ControlDescriptor.h"
#include "TArc/Controls/Private/ValidatorDelegate.h"
#include "TArc/Utils/QtConnections.h"
#include "TArc/WindowSubSystem/UI.h"

#include <Base/BaseTypes.h>
#include <QWidget>

class QLineEdit;
class QToolButton;
namespace DAVA
{
namespace TArc
{
class FilePathEdit : public ControlProxyImpl<QWidget>, private ValidatorDelegate
{
public:
    enum class Fields : uint32
    {
        Value,
        PlaceHolder,
        IsReadOnly,
        IsEnabled,
        FieldCount
    };

    DECLARE_CONTROL_PARAMS(Fields);

    FilePathEdit(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent = nullptr);
    FilePathEdit(const Params& params, ContextAccessor* accessor, Reflection model, QWidget* parent = nullptr);

private:
    void UpdateControl(const ControlDescriptor& changedFields) override;
    void SetupControl();

    void EditingFinished();
    void ButtonClicked();

    M::ValidationResult Validate(const Any& value) const override;
    void ShowHint(const QString& message) override;

    bool IsFile() const;
    FileDialogParams GetFileDialogParams() const;

    void ProcessValidationResult(M::ValidationResult& validationResult, FilePath& path);

private:
    QtConnections connections;
    QLineEdit* edit = nullptr;
    QToolButton* button = nullptr;
};
}
} // namespace DAVA
