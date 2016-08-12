#pragma once

#include <QWidget>
#include "ApplicationSettings.h"

namespace Ui
{
class SharedPoolWidget;
}

class SharedPoolWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SharedPoolWidget(QWidget* parent = nullptr);
    explicit SharedPoolWidget(const SharedPool& pool, QWidget* parent = nullptr);
    ~SharedPoolWidget() override;

    PoolID GetPoolID() const;

    bool IsChecked() const;
    void SetChecked(bool checked);

signals:
    void PoolChecked(bool checked);
    void RemoveLater();

private slots:
    void OnChecked(int val);

private:
    PoolID poolID = 0;
    Ui::SharedPoolWidget* ui;
};

inline PoolID SharedPoolWidget::GetPoolID() const
{
    return poolID;
}
