#include "Base/BaseTypes.h"
#include "MemoryManager/MemoryManagerTypes.h"

#include "ProfilingSession.h"
#include "Models/AllocPoolModel.h"
#include "Models/TagModel.h"
#include "Models/GeneralStatModel.h"
#include "Models/DumpBriefModel.h"

#include <QLabel>
#include <QFrame>
#include <QToolBar>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QMessageBox>
#include "qcustomplot.h"

#include "DumpViewerWidget.h"
#include "DiffViewerWidget.h"

#include "BranchDiff.h"
#include "Branch.h"

#include "MemProfWidget.h"
#include "ui_MemProfWidget.h"

using namespace DAVA;

MemProfWidget::MemProfWidget(QWidget *parent)
    : QWidget(parent, Qt::Window)
    , ui(new Ui::MemProfWidget())
    , realtime(true)
{
    Init();
}

MemProfWidget::MemProfWidget(ProfilingSession* profSession, QWidget *parent)
    : QWidget(parent, Qt::Window)
    , ui(new Ui::MemProfWidget())
    , realtime(false)
    , profileSession(profSession)
{
    Init();

    allocPoolModel->BeginNewProfileSession(profileSession);
    tagModel->BeginNewProfileSession(profileSession);
    generalStatModel->BeginNewProfileSession(profileSession);
    dumpBriefModel->BeginNewProfileSession(profileSession);

    ui->allocPoolTable->resizeRowsToContents();
    ui->tagTable->resizeRowsToContents();
    ui->generalStatTable->resizeRowsToContents();

    ReinitPlot();
    SetPlotData();
}

MemProfWidget::~MemProfWidget() 
{
    if (profileSession != nullptr && profileSession->IsFileMode())
    {
        delete profileSession;
    }
}

void MemProfWidget::ConnectionEstablished(bool newConnection, ProfilingSession* profSession)
{
    DVASSERT(!profSession->IsFileMode());

    profileSession = profSession;
    allocPoolModel->BeginNewProfileSession(profSession);
    tagModel->BeginNewProfileSession(profSession);
    generalStatModel->BeginNewProfileSession(profSession);
    dumpBriefModel->BeginNewProfileSession(profileSession);

    ui->labelStatus->setText("Connection established");
    if (newConnection)
    {
        ui->allocPoolTable->resizeRowsToContents();
        ui->tagTable->resizeRowsToContents();
        ui->generalStatTable->resizeRowsToContents();
        ReinitPlot();
    }
}

void MemProfWidget::ConnectionLost(const char8* message)
{
    ui->dumpProgress->setValue(0);
    ui->labelStatus->setText(message != nullptr ? QString("Connection lost: %1").arg(message)
                                                : QString("Connection lost"));
    profileSession = nullptr;
}

void MemProfWidget::StatArrived()
{
    if (realtime)
    {
        const MemoryStatItem& stat = profileSession->LastStat();

        allocPoolModel->SetCurrentValues(stat);
        tagModel->SetCurrentValues(stat);
        generalStatModel->SetCurrentValues(stat);
        UpdatePlot(stat);
    }
}

void MemProfWidget::DumpArrived(size_t sizeTotal, size_t sizeRecv)
{
    int v = static_cast<int>(double(sizeRecv) / double(sizeTotal) * 100.0);
    ui->dumpProgress->setValue(v);
    if (v >= 100)
    {
        dumpBriefModel->NewDumpArrived();
    }
}

void MemProfWidget::RealtimeToggled(bool checked)
{
    realtime = checked;
    if (profileSession != nullptr && !profileSession->IsFileMode())
    {
        QCustomPlot* plot = ui->plot;
        size_t ngraph = profileSession->AllocPoolCount();

        for (size_t i = 0;i < ngraph;++i)
        {
            QCPGraph* graph = plot->graph(i);
            graph->rescaleAxes(i > 0);
        }
        plot->replot();
    }
}

void MemProfWidget::PlotClicked(QMouseEvent* ev)
{
    QCustomPlot* plot = ui->plot;
    if (!realtime && profileSession != nullptr)
    {
        QCPAxis* xAxis = plot->xAxis;
        double value = xAxis->pixelToCoord(static_cast<double>(ev->pos().x()));
        uint64 timestamp = static_cast<uint64>(value * 1000.0);

        size_t index = profileSession->ClosestStatItem(timestamp);
        if (index != size_t(-1))
        {
            const MemoryStatItem& stat = profileSession->Stat(index);

            allocPoolModel->SetCurrentValues(stat);
            tagModel->SetCurrentValues(stat);
            generalStatModel->SetCurrentValues(stat);
        }
    }
}

void MemProfWidget::DiffClicked()
{
    Vector<int> selected;
    QItemSelectionModel* selModel = ui->dumpBriefList->selectionModel();
    if (selModel->hasSelection())
    {
        QModelIndexList list = selModel->selectedRows(0);
        for (auto x : list)
        {
            selected.push_back(x.row());
        }
    }
    if (selected.size() != 2)
    {
        QMessageBox::warning(this, "Achtung", "Select only two dumps");
        return;
    }

    int index1 = selected[0];
    int index2 = selected[1];
    if (profileSession->LoadSnapshot(index1) && profileSession->LoadSnapshot(index2))
    {
        const MemorySnapshot& snapshot1 = profileSession->Snapshot(index1);
        const MemorySnapshot& snapshot2 = profileSession->Snapshot(index2);

        DiffViewerWidget* w = new DiffViewerWidget(&snapshot1, &snapshot2, this);
        w->resize(800, 600);
        w->show();
    }
}

void MemProfWidget::DumpBriefList_OnDoubleClicked(const QModelIndex& index)
{
    int row = index.row();
    if (profileSession->LoadSnapshot(row))
    {
        const MemorySnapshot& snapshot = profileSession->Snapshot(row);
        DumpViewerWidget* w = new DumpViewerWidget(&snapshot, this);
        w->resize(800, 600);
        w->show();
    }
}

void MemProfWidget::UpdatePlot(const MemoryStatItem& stat)
{
    QCustomPlot* plot = ui->plot;
    size_t ngraph = profileSession->AllocPoolCount();

    for (size_t i = 0;i < ngraph;++i)
    {
        QCPGraph* graph = plot->graph(i);

        double key = static_cast<double>(stat.Timestamp()) / 1000.0;
        double val = static_cast<double>(stat.PoolStat()[i].allocByApp) / 1024.0 / 1024.0;
        graph->addData(key, val);

        if (realtime)
            graph->rescaleAxes(i > 0);
    }

    if (realtime)
        plot->replot();
}

void MemProfWidget::SetPlotData()
{
    const size_t ntrends = profileSession->AllocPoolCount();
    const size_t nstat = profileSession->StatCount();
    Vector<QCPDataMap*> trends;
    trends.reserve(ntrends);
    for (size_t i = 0;i < ntrends;++i)
    {
        trends.push_back(new QCPDataMap);
    }
    for (size_t i = 0;i < nstat;++i)
    {
        const MemoryStatItem& item = profileSession->Stat(i);
        const Vector<AllocPoolStat>& vstat = item.PoolStat();
        double key = static_cast<double>(item.Timestamp() / 1000.0);
        for (size_t j = 0;j < ntrends;++j)
        {
            double value = static_cast<double>(vstat[j].allocByApp) / 1024.0 / 1024.0;
            trends[j]->insert(key, QCPData(key, value));
        }
    }

    QCustomPlot* plot = ui->plot;
    for (size_t i = 0;i < ntrends;++i)
    {
        QCPGraph* graph = plot->graph(i);
        graph->setData(trends[i], false);

        graph->rescaleAxes(i > 0);
    }
    plot->replot();
}

void MemProfWidget::ReinitPlot()
{
    QCustomPlot* plot = ui->plot;
    size_t ngraph = profileSession->AllocPoolCount();

    const size_t ncolors = poolColors.size();

    plot->clearGraphs();
    for (size_t i = 0;i < ngraph;++i)
    {
        QCPGraph* graph = plot->addGraph();
        QPen pen(poolColors[i % ncolors]);
        pen.setWidth(2);
        graph->setPen(pen);
        graph->setAntialiasedFill(false);
    }

    plot->xAxis2->setVisible(true);
    plot->xAxis2->setTickLabels(false);
    plot->yAxis2->setVisible(true);
    plot->yAxis2->setTickLabels(false);

    plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);

    connect(plot->xAxis, SIGNAL(rangeChanged(QCPRange)), plot->xAxis2, SLOT(setRange(QCPRange)));
    connect(plot->yAxis, SIGNAL(rangeChanged(QCPRange)), plot->yAxis2, SLOT(setRange(QCPRange)));
}

void MemProfWidget::Init()
{
    ui->setupUi(this);

    QToolBar* toolbar = new QToolBar;
    {
        QAction* actionDump = toolbar->addAction("Memory dump");
        connect(actionDump, SIGNAL(triggered()), this, SIGNAL(OnDumpButton()));

        QAction* actionToggleRealtime = toolbar->addAction("Realtime");
        actionToggleRealtime->setCheckable(true);
        if (realtime)
            actionToggleRealtime->toggle();
        connect(actionToggleRealtime, &QAction::toggled, this, &MemProfWidget::RealtimeToggled);

        QAction* actionDiff = toolbar->addAction("Diff");
        connect(actionDiff, &QAction::triggered, this, &MemProfWidget::DiffClicked);
    }
    ui->vertLayout->insertWidget(0, toolbar);

    poolColors = {
        QColor(Qt::darkRed),
        QColor(Qt::darkBlue),
        QColor(Qt::darkGreen),
        QColor(Qt::darkYellow),
        QColor(Qt::darkMagenta),
        QColor(Qt::darkCyan),
        QColor(Qt::darkGray),
        QColor(Qt::black)
    };

    {
        tagModel = new TagModel;
        tagModel->SetTagColors(QColor(200, 255, 200), QColor(Qt::lightGray));

        allocPoolModel = new AllocPoolModel;
        allocPoolModel->SetPoolColors(poolColors);

        generalStatModel = new GeneralStatModel;
        dumpBriefModel = new DumpBriefModel;

        ui->allocPoolTable->setModel(allocPoolModel);
        ui->tagTable->setModel(tagModel);
        ui->generalStatTable->setModel(generalStatModel);
        ui->dumpBriefList->setModel(dumpBriefModel);

        connect(ui->dumpBriefList, &QListView::doubleClicked, this, &MemProfWidget::DumpBriefList_OnDoubleClicked);
    }

    QCustomPlot* plot = ui->plot;
    connect(plot, &QCustomPlot::mousePress, this, &MemProfWidget::PlotClicked);
}
