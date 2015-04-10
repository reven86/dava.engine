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
    explicit MemProfWidget(QWidget *parent = nullptr);
    explicit MemProfWidget(ProfilingSession* profSession, QWidget *parent = nullptr);
    ~MemProfWidget();

    void ShowDump(const DAVA::Vector<DAVA::uint8>& v);

public slots:
    void ConnectionEstablished(bool newConnection, ProfilingSession* profSession);
    void ConnectionLost(const DAVA::char8* message);
    void StatArrived();
    void DumpArrived(size_t sizeTotal, size_t sizeRecv);

    void RealtimeToggled(bool checked);
    void PlotClicked(QMouseEvent* ev);
    
private:
    void Init();
    void ReinitPlot();
    void UpdatePlot(const StatItem& stat);
    void SetPlotData();

private:
    QScopedPointer<Ui::MemProfWidget> ui;

    bool realtime;
    ProfilingSession* profileSession = nullptr;
    QPointer<AllocPoolModel> allocPoolModel;
    QPointer<TagModel> tagModel;
    QPointer<GeneralStatModel> generalStatModel;

    DAVA::Vector<QColor> poolColors;
};

#endif // __DEVICELOGWIDGET_H__
