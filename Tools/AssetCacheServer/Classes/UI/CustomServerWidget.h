#pragma once

#include <QWidget>
#include "ApplicationSettings.h"

namespace Ui
{
class CustomServerWidget;
}

class CustomServerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CustomServerWidget(QWidget* parent = nullptr);
    explicit CustomServerWidget(const RemoteServerParams& newServer, QWidget* parent = nullptr);
    ~CustomServerWidget() override;

    RemoteServerParams GetServerData() const;

    bool IsCorrectData();

    bool IsChecked() const;
    void SetChecked(bool checked);

signals:
    void ServerChecked(bool checked);
    void ParametersChanged();
    void RemoveLater();

private slots:
    void OnParametersChanged();
    void OnChecked(int val);

private:
    Ui::CustomServerWidget* ui;
};
