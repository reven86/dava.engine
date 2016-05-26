#ifndef __RESOURCEEDITORQT__QTWAITDIALOG__
#define __RESOURCEEDITORQT__QTWAITDIALOG__

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QEventLoop>

namespace Ui
{
class QtWaitDialog;
}

class QtWaitDialog
: public QWidget
{
    Q_OBJECT

public:
    QtWaitDialog(QWidget* parent = 0);
    ~QtWaitDialog();

    void Exec(const QString& title, const QString& message, bool hasWaitbar, bool hasCancel);
    void Show(const QString& title, const QString& message, bool hasWaitbar, bool hasCancel);
    void Reset();

    void SetMessage(const QString& message);

    void SetRange(int min, int max);
    void SetRangeMin(int min);
    void SetRangeMax(int max);
    void SetValue(int value);
    void EnableCancel(bool enable);

    bool WasCanceled() const;

signals:
    void canceled();

protected slots:
    void CancelPressed();
    void WaitCanceled();

private:
    void processEvents();

    void Setup(const QString& title, const QString& message, bool hasWaitbar, bool hasCancel);
    Ui::QtWaitDialog* ui;

    bool wasCanceled;
    bool isRunnedFromExec;
    QEventLoop loop;
};

#endif // __RESOURCEEDITORQT__MAINWAITDIALOG__
