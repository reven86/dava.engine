#ifndef __LOGWIDGET_H__
#define __LOGWIDGET_H__


#include <QWidget>
#include <QPointer>
#include <QTime>
#include "ui_LogWidget.h"
#include "Base/Result.h"
#include "Base/JSONconverter.h"

namespace Ui
{
    class LogWidget;
};

class QTimer;
class LogModel;
class LogFilterModel;


class LogWidget : public QWidget, public Ui::LogWidget
{
    Q_OBJECT

public:
    explicit LogWidget(QWidget* parent = NULL);
    ~LogWidget() = default;

    LogModel *Model();
    QByteArray Serialize() const;
    void Deserialize(const QByteArray &data);
public slots:
    void AddResultList(const DAVA::ResultList &resultList);
signals:
    void ItemClicked(const DAVA::JSONconverter& data);
private slots:
    void OnTextFilterChanged(const QString& text);
    void OnCopy();
    void OnClear();
    void OnClicked(const QModelIndex &index);
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

Q_DECLARE_METATYPE(DAVA::JSONconverter);



#endif // __LOGWIDGET_H__
