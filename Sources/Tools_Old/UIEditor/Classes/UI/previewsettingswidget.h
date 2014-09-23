#ifndef PREVIEWSETTINGSWIDGET_H
#define PREVIEWSETTINGSWIDGET_H

#include <QWidget>

namespace Ui {
class PreviewSettingsWidget;
}

class PreviewSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PreviewSettingsWidget(QWidget *parent = 0);
    ~PreviewSettingsWidget();

signals:
    void PreviewModeChanged(int previewSettingsIndex);
    
protected:
	virtual void showEvent(QShowEvent *);

    // Refresh the settings list.
    void FillSettingsList();

protected slots:
    void OnSelectedModeChanged();

private:
    Ui::PreviewSettingsWidget *ui;
};

#endif // PREVIEWSETTINGSWIDGET_H
