#ifndef __LOGWIDGET_H__
#define __LOGWIDGET_H__


#include <QWidget>
#include <QPointer>
#include <QTime>
#include "ui_LogWidget.h"
#include "Base/Result.h"
#include "LogModel.h"

namespace Ui
{
    class LogWidget;
};

class QTimer;
class LogFilterModel;


class LogWidget : public QWidget, public Ui::LogWidget
{
    Q_OBJECT

public:
    explicit LogWidget(QWidget* parent = NULL);
    ~LogWidget() = default;
    void SetConvertFunction(LogModel::ConvertFunc func); //provide mechanism to convert data string to string to be displayed
    LogModel *Model() const;
    QByteArray Serialize() const;
    void Deserialize(const QByteArray &data);
signals:
    void ItemClicked(const QString &data);
public slots:
    void AddResultList(const DAVA::ResultList &resultList);
private slots:
    void OnTextFilterChanged(const QString& text);
    void OnCopy();
    void OnClear();
    void OnBeforeAdded();
    void OnRowAdded();
private:
    void FillFiltersCombo();
    bool eventFilter( QObject* watched, QEvent* event ) override;

    QPointer<LogModel> logModel;
    QPointer<LogFilterModel> logFilterModel;
    QTime time;
    bool onBottom;
};


#endif // __LOGWIDGET_H__
