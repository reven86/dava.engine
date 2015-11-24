/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "Base/BaseTypes.h"
#include "MemoryManager/MemoryManagerTypes.h"

#include "Qt/DeviceInfo/MemoryTool/BranchDiff.h"
#include "Qt/DeviceInfo/MemoryTool/Branch.h"
#include "Qt/DeviceInfo/MemoryTool/ProfilingSession.h"

#include "Qt/DeviceInfo/MemoryTool/Models/AllocPoolModel.h"
#include "Qt/DeviceInfo/MemoryTool/Models/TagModel.h"
#include "Qt/DeviceInfo/MemoryTool/Models/GeneralStatModel.h"
#include "Qt/DeviceInfo/MemoryTool/Models/SnapshotListModel.h"

#include "Qt/DeviceInfo/MemoryTool/Widgets/SnapshotViewerWidget.h"
#include "Qt/DeviceInfo/MemoryTool/Widgets/SnapshotDiffViewerWidget.h"
#include "Qt/DeviceInfo/MemoryTool/Widgets/MemProfWidget.h"

#include "ui_MemProfWidget.h"

#include <QLabel>
#include <QFrame>
#include <QToolBar>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QMessageBox>
#include "qcustomplot.h"

using namespace DAVA;

MemProfWidget::MemProfWidget(ProfilingSession* profSession, QWidget *parent)
    : QWidget(parent, Qt::Window)
    , ui(new Ui::MemProfWidget())
    , profileSession(profSession)
    , realtimeMode(!profileSession->IsFileMode())
{
    DVASSERT(profileSession != nullptr);

    InitUI();
    if (profileSession->IsFileMode())
    {
        ui->labelStatus->setText(profileSession->MemoryLogFile().GetAbsolutePathname().c_str());

        allocPoolModel->BeginNewProfileSession(profileSession);
        tagModel->BeginNewProfileSession(profileSession);
        generalStatModel->BeginNewProfileSession(profileSession);
        snapshotModel->BeginNewProfileSession(profileSession);

        ui->allocPoolTable->resizeRowsToContents();
        ui->tagTable->resizeRowsToContents();
        ui->generalStatTable->resizeRowsToContents();

        ReinitPlot();
        SetPlotData();
    }
}

MemProfWidget::~MemProfWidget() {}

void MemProfWidget::ConnectionEstablished(bool newConnection)
{
    DVASSERT(!profileSession->IsFileMode());

    allocPoolModel->BeginNewProfileSession(profileSession);
    tagModel->BeginNewProfileSession(profileSession);
    generalStatModel->BeginNewProfileSession(profileSession);
    snapshotModel->BeginNewProfileSession(profileSession);

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
    ui->snapshotProgress->setValue(0);
    ui->labelStatus->setText(message != nullptr ? QString("Connection lost: %1").arg(message)
                                                : QString("Connection lost"));
}

void MemProfWidget::StatArrived(uint32 /*itemCount*/)
{
    const MemoryStatItem& stat = profileSession->LastStat();
    if (realtimeMode)
    {
        allocPoolModel->SetCurrentValues(stat);
        tagModel->SetCurrentValues(stat);
        generalStatModel->SetCurrentValues(stat);
    }
    UpdatePlot(stat);
}

void MemProfWidget::SnapshotProgress(uint32 totalSize, uint32 recvSize)
{
    if (totalSize > 0)
    {
        int percent = static_cast<int>(double(recvSize) / double(totalSize) * 100.0);
        ui->snapshotProgress->setValue(percent);
        if (totalSize == recvSize)
        {
            snapshotModel->NewSnapshotArrived();
        }
    }
    else
    {
        ui->snapshotProgress->setValue(0);
    }
}

void MemProfWidget::RealtimeToggled(bool checked)
{
    realtimeMode = checked;
    if (!profileSession->IsFileMode() && realtimeMode)
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

void MemProfWidget::ScatterPointsToggled(bool checked)
{
    showScatterPoints = checked;
    QCPScatterStyle::ScatterShape shape = showScatterPoints ? QCPScatterStyle::ssDisc : QCPScatterStyle::ssNone;

    QCustomPlot* plot = ui->plot;
    for (int i = 0, n = plot->graphCount(); i < n; ++i)
    {
        QCPGraph* graph = plot->graph(i);
        graph->setScatterStyle(shape);
    }
    plot->replot();
}

void MemProfWidget::PlotClicked(QMouseEvent* ev)
{
    QCustomPlot* plot = ui->plot;
    if (!realtimeMode)
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

            plotItemTracer->setGraphKey(value);
        }
    }
}

void MemProfWidget::DiffClicked()
{
    Vector<int> selected;
    QItemSelectionModel* selModel = ui->snapshotList->selectionModel();
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
        QMessageBox::warning(this, "Achtung", "Select only two snapshots");
        return;
    }

    int index1 = selected[0];
    int index2 = selected[1];
    if (profileSession->LoadSnapshot(index1) && profileSession->LoadSnapshot(index2))
    {
        SnapshotDiffViewerWidget* w = new SnapshotDiffViewerWidget(profileSession, index1, index2, this);
        w->resize(800, 600);
        w->show();
    }
}

void MemProfWidget::SnapshotList_OnDoubleClicked(const QModelIndex& index)
{
    int row = index.row();
    if (profileSession->LoadSnapshot(row))
    {
        SnapshotViewerWidget* w = new SnapshotViewerWidget(profileSession, row, this);
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

        if (realtimeMode)
        {
            graph->rescaleAxes(i > 0);
        }
    }

    if (realtimeMode)
    {
        plot->replot();
    }
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

    plotItemTracer->setGraph(plot->graph(0));
}

void MemProfWidget::InitUI()
{
    ui->setupUi(this);

    QToolBar* toolbar = new QToolBar;
    {
        QAction* actionSnapshot = toolbar->addAction("Memory snapshot");
        connect(actionSnapshot, SIGNAL(triggered()), this, SIGNAL(OnSnapshotButton()));

        if (!profileSession->IsFileMode())
        {
            QAction* actionToggleRealtime = toolbar->addAction("Realtime");
            actionToggleRealtime->setCheckable(true);
            actionToggleRealtime->toggle();
            connect(actionToggleRealtime, &QAction::toggled, this, &MemProfWidget::RealtimeToggled);
        }

        QAction* actionDiff = toolbar->addAction("Diff");
        connect(actionDiff, &QAction::triggered, this, &MemProfWidget::DiffClicked);

        QAction* actionToggleScatterPoints = toolbar->addAction("Scatter points");
        actionToggleScatterPoints->setCheckable(true);
        connect(actionToggleScatterPoints, &QAction::toggled, this, &MemProfWidget::ScatterPointsToggled);
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
        snapshotModel = new SnapshotListModel;

        ui->allocPoolTable->setModel(allocPoolModel);
        ui->tagTable->setModel(tagModel);
        ui->generalStatTable->setModel(generalStatModel);
        ui->snapshotList->setModel(snapshotModel);

        connect(ui->snapshotList, &QListView::doubleClicked, this, &MemProfWidget::SnapshotList_OnDoubleClicked);
    }

    QCustomPlot* plot = ui->plot;
    connect(plot, &QCustomPlot::mousePress, this, &MemProfWidget::PlotClicked);

    plotItemTracer = new QCPItemTracer(plot);
    plotItemTracer->setStyle(QCPItemTracer::tsCrosshair);
    plot->addItem(plotItemTracer); // QCustomPlot will delete plotItemTracer in its destructor
}
