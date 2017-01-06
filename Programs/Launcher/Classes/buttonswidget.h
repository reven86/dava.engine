#ifndef BUTTONSWIDGET_H
#define BUTTONSWIDGET_H

#include <QWidget>
#include <QString>

namespace Ui
{
class ButtonsWidget;
}

class ButtonsWidget : public QWidget
{
    Q_OBJECT

public:
    enum ButtonsState
    {
        BUTTONS_STATE_DISABLED_ALL = 0,
        BUTTONS_STATE_INSTALLED = 1,
        BUTTONS_STATE_AVALIBLE = 1 << 1,

        BUTTONS_STATE_COUNT
    };

    explicit ButtonsWidget(int rowNum, QWidget* parent = 0);
    ~ButtonsWidget();

    void SetButtonsState(int state);

signals:
    void OnRun(int rowNum);
    void OnRemove(int rowNum);
    void OnInstall(int rowNum);

public slots:
    void OnRunClicked();
    void OnRemoveClicked();
    void OnInstallClicked();

private:
    Ui::ButtonsWidget* ui;
    int rowNumber;
};

#endif // BUTTONSWIDGET_H
