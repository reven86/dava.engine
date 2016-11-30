#ifndef __LOGWIDGET_H__
#define __LOGWIDGET_H__


#include "Base/Result.h"
#include "LogModel.h"

#include "QtTools/WarningGuard/QtWarningsHandler.h"
PUSH_QT_WARNING_SUPRESSOR
#include <QWidget>
#include <QPointer>
POP_QT_WARNING_SUPRESSOR

class QTimer;
class LogFilterModel;
class LogModel;

namespace Ui
{
class LogWidget;
};

class LogWidget : public QWidget
{
    PUSH_QT_WARNING_SUPRESSOR
    Q_OBJECT
    POP_QT_WARNING_SUPRESSOR

public:
    explicit LogWidget(QWidget* parent = NULL);
    ~LogWidget();
    void SetConvertFunction(LogModel::ConvertFunc func); //provide mechanism to convert data string to string to be displayed
    QByteArray Serialize() const;
    void Deserialize(const QByteArray& data);

public slots:
    void AddMessage(DAVA::Logger::eLogLevel ll, const QByteArray& msg);

signals:
    void ItemClicked(const QString& data);

private slots:
    void OnCopy();
    void OnBeforeAdded();
    void UpdateScroll();
    void OnItemClicked(const QModelIndex& index);

private:
    void FillFiltersCombo();
    bool eventFilter(QObject* watched, QEvent* event) override;

    QPointer<LogModel> logModel;
    QPointer<LogFilterModel> logFilterModel;
    Ui::LogWidget* ui;
    QTimer* scrollTimer;
};


#endif // __LOGWIDGET_H__
