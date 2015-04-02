#ifndef __DEVICELOGWIDGET_H__
#define __DEVICELOGWIDGET_H__

#include <QPointer>
#include <QWidget>
#include <QColor>
#include <QScopedPointer>
#include <qtableview.h>
#include "Base/BaseTypes.h"

namespace Ui {
    class MemProfWidget;
} // namespace Ui

class QLabel;
class QCustomPlot;
class QFrame;
class QToolBar;

class MemProfInfoModel;
class MemProfPlot;

namespace DAVA
{
    struct MMStatConfig;
    struct MMCurStat;
}   // namespace DAVA

class AllocPoolModel;
class TagModel;
class StatItem;
class ProfilingSession;

class MemProfWidget : public QWidget
{
    Q_OBJECT

signals:
    void OnDumpButton();
    void OnViewDumpButton();
    void OnViewFileDumpButton();
    
public:
    explicit MemProfWidget(QWidget *parent = NULL);
    ~MemProfWidget();

    void ShowDump(const DAVA::Vector<DAVA::uint8>& v);

    void UpdateProgress(size_t total, size_t recv);

public slots:
    void ConnectionEstablished(bool newConnection, ProfilingSession* profSession);
    void ConnectionLost(const DAVA::char8* message);
    void StatArrived();
    
private:
    void ReinitPlot();
    void UpdatePlot(const StatItem& stat);

    void CreateUI();
    
private:
    QScopedPointer<Ui::MemProfWidget> ui;

    QToolBar* toolbar;
    QFrame* frame;

    MemProfInfoModel * model;

    ProfilingSession* profileSession;
    QPointer<AllocPoolModel> allocPoolModel;
    QPointer<TagModel> tagModel;

    DAVA::Vector<QColor> poolColors;
};

#endif // __DEVICELOGWIDGET_H__
