#ifndef __DEVICELOGWIDGET_H__
#define __DEVICELOGWIDGET_H__

#include "Base/BaseTypes.h"

#include <QWidget>
#include <QColor>
#include <QPointer>
#include <QScopedPointer>

namespace Ui {
    class MemProfWidget;
} // namespace Ui

class QLabel;
class QCustomPlot;
class QFrame;
class QToolBar;

class QCustomPlot;

namespace DAVA
{
    struct MMStatConfig;
    struct MMCurStat;
}   // namespace DAVA

class AllocPoolModel;
class TagModel;
class GeneralStatModel;
class DumpBriefModel;
class MemoryStatItem;
class ProfilingSession;

class MemProfWidget : public QWidget
{
    Q_OBJECT

signals:
    void OnDumpButton();
    
public:
    explicit MemProfWidget(QWidget *parent = nullptr);
    explicit MemProfWidget(ProfilingSession* profSession, QWidget *parent = nullptr);
    ~MemProfWidget();

public slots:
    void ConnectionEstablished(bool newConnection, ProfilingSession* profSession);
    void ConnectionLost(const DAVA::char8* message);
    void StatArrived();
    void DumpArrived(size_t sizeTotal, size_t sizeRecv);

    void RealtimeToggled(bool checked);
    void DiffClicked();
    void PlotClicked(QMouseEvent* ev);

    void DumpBriefList_OnDoubleClicked(const QModelIndex& index);

private:
    void Init();
    void ReinitPlot();
    void UpdatePlot(const MemoryStatItem& stat);
    void SetPlotData();

private:
    QScopedPointer<Ui::MemProfWidget> ui;

    bool realtime;
    ProfilingSession* profileSession = nullptr;
    QPointer<AllocPoolModel> allocPoolModel;
    QPointer<TagModel> tagModel;
    QPointer<GeneralStatModel> generalStatModel;
    QPointer<DumpBriefModel> dumpBriefModel;

    DAVA::Vector<QColor> poolColors;
};

#endif // __DEVICELOGWIDGET_H__
