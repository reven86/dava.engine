#include "WaitDialog.h"

#include "QtTools/Utils/RenderContextGuard.h"

#include <QString>
#include <QLabel>
#include <QPlainTextEdit>
#include <QDialog>
#include <QProgressBar>
#include <QGridLayout>
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
    : dlg(new QDialog(parent, Qt::WindowFlags(Qt::Window | Qt::CustomizeWindowHint)))
{
    QGridLayout* layout = new QGridLayout(dlg);
    layout->setHorizontalSpacing(10);
    layout->setVerticalSpacing(5);
    layout->setContentsMargins(5, 5, 5, 5);

    QLabel* waitPixmap = new QLabel(dlg);
    QSizePolicy sizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(waitPixmap->sizePolicy().hasHeightForWidth());
    waitPixmap->setSizePolicy(sizePolicy);
    waitPixmap->setPixmap(QPixmap(QString::fromUtf8(":/QtIcons/wait.png")));
    waitPixmap->setAlignment(Qt::Alignment(Qt::AlignHCenter | Qt::AlignTop));

    layout->addWidget(waitPixmap, 2, 0, 1, 1);

    messageLabel = new QPlainTextEdit(dlg);
    messageLabel->setEnabled(true);
    QSizePolicy sizePolicy2(QSizePolicy::Expanding, QSizePolicy::Expanding);
    sizePolicy2.setHorizontalStretch(0);
    sizePolicy2.setVerticalStretch(0);
    sizePolicy2.setHeightForWidth(messageLabel->sizePolicy().hasHeightForWidth());
    messageLabel->setSizePolicy(sizePolicy2);
    messageLabel->setAcceptDrops(false);
    messageLabel->setFrameShape(QFrame::NoFrame);
    messageLabel->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    messageLabel->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    messageLabel->setUndoRedoEnabled(false);
    messageLabel->setReadOnly(true);
    messageLabel->setPlainText(params.message);

    QPalette palette = messageLabel->palette();
    palette.setColor(QPalette::Base, Qt::transparent);
    messageLabel->setPalette(palette);

    layout->addWidget(messageLabel, 2, 1, 1, 2);

    if (params.needProgressBar)
    {
        progressBar = new QProgressBar(dlg);
        progressBar->setAlignment(Qt::AlignCenter);
        progressBar->setTextDirection(QProgressBar::TopToBottom);
        progressBar->setMinimum(params.min);
        progressBar->setMaximum(params.max);

        layout->addWidget(progressBar, 3, 0, 1, 3);
    }

    dlg->setLayout(layout);
    dlg->setFixedSize(400, params.needProgressBar ? 150 : 120);
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
        QMetaObject::invokeMethod(messageLabel.data(), "setPlainText", WaitDialogDetail::GetConnectionType(), Q_ARG(QString, msg));
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
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    }
}
} // namespace TArc
} // namespace DAVA