#include "WaitDialog.h"

#include "QtTools/Utils/RenderContextGuard.h"

#include <QString>
#include <QLabel>
#include <QDialog>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QApplication>
#include <QThread>
#include <QMetaObject>

namespace DAVA
{
namespace TArc
{
namespace WaitDialogDetail
{
Qt::ConnectionType GetConnectionType()
{
    return (qApp->thread() == QThread::currentThread()) ? Qt::DirectConnection : Qt::QueuedConnection;
}
} // namespace WaitDialogDetail

WaitDialog::WaitDialog(const WaitDialogParams& params, QWidget* parent)
    : dlg(new QDialog(parent, Qt::WindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint)))
{
    label = new QLabel(params.message, dlg.data());
    label->setAlignment(Qt::AlignCenter);
    label->setWordWrap(false);
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(label);
    if (params.needProgressBar)
    {
        progressBar = new QProgressBar(dlg.data());
        progressBar->setRange(params.min, params.max);
        progressBar->setValue(0);
        layout->addWidget(progressBar);
    }

    dlg->setLayout(layout);
    dlg->setWindowModality(Qt::WindowModal);
    originalCursor = dlg->cursor();
    dlg->setCursor(Qt::BusyCursor);
}

WaitDialog::~WaitDialog()
{
    beforeDestroy.Emit(this);
    if (dlg != nullptr)
    {
        QMetaObject::invokeMethod(dlg.data(), "close", WaitDialogDetail::GetConnectionType());
        QMetaObject::invokeMethod(dlg.data(), "deleteLater", WaitDialogDetail::GetConnectionType());
        dlg->setCursor(originalCursor);
    }
}

void WaitDialog::Show()
{
    DVASSERT(!dlg.isNull());
    dlg->setModal(true);
    dlg->show();
    dlg->raise();
    dlg->activateWindow();
    Update();
}

void WaitDialog::SetMessage(const QString& msg)
{
    if (dlg != nullptr)
    {
        RenderContextGuard guard;
        QMetaObject::invokeMethod(label.data(), "setText", WaitDialogDetail::GetConnectionType(), Q_ARG(QString, msg));
        Update();
    }
}

void WaitDialog::SetRange(uint32 min, uint32 max)
{
    if (dlg != nullptr && progressBar != nullptr)
    {
        RenderContextGuard guard;
        QMetaObject::invokeMethod(progressBar.data(), "setRange", WaitDialogDetail::GetConnectionType(),
                                  Q_ARG(int, min), Q_ARG(int, max));
        Update();
    }
}

void WaitDialog::SetProgressValue(uint32 progress)
{
    if (dlg != nullptr && progressBar != nullptr)
    {
        RenderContextGuard guard;
        QMetaObject::invokeMethod(progressBar.data(), "setValue", WaitDialogDetail::GetConnectionType(), Q_ARG(int, progress));
        Update();
    }
}

void WaitDialog::Update()
{
    if (WaitDialogDetail::GetConnectionType() == Qt::DirectConnection)
    {
        QPoint centerPoint = dlg->geometry().center();
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
        if (dlg != nullptr)
        {
            QRect r = dlg->geometry();
            r.moveCenter(centerPoint);
            dlg->move(r.topLeft());
        }
    }
}
} // namespace TArc
} // namespace DAVA