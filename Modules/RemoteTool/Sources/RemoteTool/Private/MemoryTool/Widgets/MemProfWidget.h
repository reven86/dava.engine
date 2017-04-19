#pragma once

#include <Base/BaseTypes.h>

#include <QWidget>
#include <QColor>
#include <QPointer>
#include <QScopedPointer>

namespace Ui
{
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
} // namespace DAVA

class AllocPoolModel;
class TagModel;
class GeneralStatModel;
class SnapshotListModel;
class MemoryStatItem;
class ProfilingSession;

class QCPItemTracer;

class MemProfWidget : public QWidget
{
    Q_OBJECT

signals:
    void OnSnapshotButton();

public:
    MemProfWidget(ProfilingSession* profSession, QWidget* parent = nullptr);
    virtual ~MemProfWidget();

public slots:
    void ConnectionEstablished(bool newConnection);
    void ConnectionLost(const DAVA::char8* message);
    void StatArrived(DAVA::uint32 itemCount);
    void SnapshotProgress(DAVA::uint32 totalSize, DAVA::uint32 recvSize);

    void RealtimeToggled(bool checked);
    void ScatterPointsToggled(bool checked);
    void DiffClicked();
    void PlotClicked(QMouseEvent* ev);

    void SnapshotList_OnDoubleClicked(const QModelIndex& index);

private:
    void InitUI();
    void ReinitPlot();
    void UpdatePlot(const MemoryStatItem& stat);
    void SetPlotData();

private:
    QScopedPointer<Ui::MemProfWidget> ui;

    ProfilingSession* profileSession = nullptr;
    QPointer<AllocPoolModel> allocPoolModel;
    QPointer<TagModel> tagModel;
    QPointer<GeneralStatModel> generalStatModel;
    QPointer<SnapshotListModel> snapshotModel;

    DAVA::Vector<QColor> poolColors;
    bool realtimeMode;
    bool showScatterPoints = false;

    QCPItemTracer* plotItemTracer = nullptr;
};
