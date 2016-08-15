#include "WaitDialog.h"

#include <QString>
#include <QLabel>
#include <QDialog>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QApplication>
#include <QThread>
#include <QMetaObject>

namespace tarc
{

namespace WaitDialogDetails
{

Qt::ConnectionType GetConnectionType()
{
    return (qApp->thread() == QThread::currentThread()) ? Qt::DirectConnection : Qt::QueuedConnection;
}

}

WaitDialog::WaitDialog(const WaitDialogParams& params, QWidget* parent)
    : dlg(new QDialog(parent, Qt::Dialog | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint))
{
    label = new QLabel(params.message, dlg.data());
    label->setAlignment(Qt::AlignCenter);
    label->setWordWrap(true);

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
}

WaitDialog::~WaitDialog()
{
    if (dlg != nullptr)
    {
        QMetaObject::invokeMethod(dlg.data(), "close", WaitDialogDetails::GetConnectionType());
        QMetaObject::invokeMethod(dlg.data(), "deleteLater", WaitDialogDetails::GetConnectionType());
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
        QMetaObject::invokeMethod(label.data(), "setText", WaitDialogDetails::GetConnectionType(), Q_ARG(QString, msg));
    }
}

void WaitDialog::SetRange(DAVA::uint32 min, DAVA::uint32 max)
{
    if (dlg != nullptr && progressBar != nullptr)
    {
        QMetaObject::invokeMethod(progressBar.data(), "setRange", WaitDialogDetails::GetConnectionType(),
                                  Q_ARG(int, min), Q_ARG(int, max));
    }
}

void WaitDialog::SetProgressValue(DAVA::uint32 progress)
{
    if (dlg != nullptr && progressBar != nullptr)
    {
        QMetaObject::invokeMethod(progressBar.data(), "setValue", WaitDialogDetails::GetConnectionType(), Q_ARG(int, progress));
    }
}

void WaitDialog::Update()
{
    if (WaitDialogDetails::GetConnectionType() == Qt::DirectConnection)
    {
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    }
}

}