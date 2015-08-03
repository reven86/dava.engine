#ifndef __LOGWIDGET_H__
#define __LOGWIDGET_H__


#include <QWidget>
#include <QPointer>
#include <QTime>
#include "Base/Result.h"

class QTimer;
class LogFilterModel;
class LogModel;

namespace Ui
{
    class LogWidget;
};

class LogWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LogWidget(QWidget* parent = NULL);
    ~LogWidget();

    LogModel *Model() const;
    QByteArray Serialize() const;
    void Deserialize(const QByteArray &data);
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
    Ui::LogWidget *ui;
};


#endif // __LOGWIDGET_H__
