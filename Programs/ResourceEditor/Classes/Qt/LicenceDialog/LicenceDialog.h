#ifndef __LICENCE_DIALOG_H__
#define __LICENCE_DIALOG_H__

#include <QScopedPointer>
#include <QDialog>
#include <QUrl>

namespace Ui
{
class LicenceDialog;
}

class LicenceDialog
: public QDialog
{
    Q_OBJECT

public:
    explicit LicenceDialog(QWidget* parent = 0);
    ~LicenceDialog();

    bool process();
    void setHtmlText(const QString& text);

private slots:
    void onAccept();
    void onDecline();
    void onAcceptCheckbox();

private:
    QScopedPointer<Ui::LicenceDialog> ui;
    bool accepted;
};


#endif // __LICENCE_DIALOG_H__
