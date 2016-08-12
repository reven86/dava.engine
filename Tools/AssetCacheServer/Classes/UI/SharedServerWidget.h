#pragma once

#include <QWidget>
#include "ApplicationSettings.h"

namespace Ui
{
class SharedServerWidget;
}

class SharedServerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SharedServerWidget(QWidget* parent = nullptr);
    explicit SharedServerWidget(const SharedServer& newServer, QWidget* parent = nullptr);
    ~SharedServerWidget() override;

    ServerID GetServerID() const;
    PoolID GetPoolID() const;

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
    PoolID poolID = 0;
    ServerID serverID = 0;
    Ui::SharedServerWidget* ui;
};

inline PoolID SharedServerWidget::GetPoolID() const
{
    return poolID;
}

inline ServerID SharedServerWidget::GetServerID() const
{
    return serverID;
}
