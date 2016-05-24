#ifndef __REMOTE_ASSET_CACHE_SERVER_H__
#define __REMOTE_ASSET_CACHE_SERVER_H__

#include <QWidget>
#include "ApplicationSettings.h"

namespace Ui
{
class RemoteServerWidget;
}

class RemoteServerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RemoteServerWidget(QWidget* parent = nullptr);
    explicit RemoteServerWidget(const ServerData& newServer, QWidget* parent = nullptr);
    ~RemoteServerWidget() override;

    ServerData GetServerData() const;

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
    Ui::RemoteServerWidget* ui;
};

#endif // __REMOTE_ASSET_CACHE_SERVER_H__
