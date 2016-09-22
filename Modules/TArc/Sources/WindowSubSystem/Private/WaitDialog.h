#pragma once

#include "DataProcessing/DataWrapper.h"
#include "WindowSubSystem/UI.h"

#include <QPointer>

class QDialog;
class QProgressBar;
class QLabel;
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

private:
    QPointer<QDialog> dlg;
    QPointer<QProgressBar> progressBar;
    QPointer<QLabel> label;
};
} // namespace TArc
} // namespace DAVA
