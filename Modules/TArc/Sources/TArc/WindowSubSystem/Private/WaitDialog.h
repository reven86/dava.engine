#pragma once

#include "TArc/DataProcessing/DataWrapper.h"
#include "TArc/WindowSubSystem/UI.h"

#include <QPointer>
#include <QCursor>

class QDialog;
class QProgressBar;
class QPlainTextEdit;
namespace DAVA
{
namespace TArc
{
class WaitDialog : public WaitHandle
{
public:
    WaitDialog(const WaitDialogParams& params, QWidget* parent);
    ~WaitDialog();

    void Show();
    void SetMessage(const QString& msg) override;
    void SetRange(uint32 min, uint32 max) override;
    void SetProgressValue(uint32 progress) override;
    void Update() override;

    DAVA::Signal<WaitHandle*> beforeDestroy;

private:
    QPointer<QDialog> dlg;
    QPointer<QProgressBar> progressBar;
    QPointer<QPlainTextEdit> messageLabel;
    QCursor originalCursor;
};
} // namespace TArc
} // namespace DAVA
