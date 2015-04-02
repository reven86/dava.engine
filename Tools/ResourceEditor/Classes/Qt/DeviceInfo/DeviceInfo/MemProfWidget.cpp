#include <QLabel>
#include <QFrame>
#include <QToolBar>
#include <QGridLayout>
#include <QVBoxLayout>

#include "Base/BaseTypes.h"
#include "MemoryManager/MemoryManagerTypes.h"
#include "MemProfWidget.h"
#include "qcustomplot.h"
#include "MemProfInfoModel.h"
#include "MemProfPlot.h"
#include "MemoryItemStyleDelegate.h"
#include "ui_MemProfWidget.h"

#include "ProfilingSession.h"
#include "Models/AllocPoolModel.h"
#include "Models/TagModel.h"

using namespace DAVA;

MemProfWidget::MemProfWidget(QWidget *parent)
    : QWidget(parent, Qt::Window)
    , ui(new Ui::MemProfWidget())
    , toolbar(nullptr)
    , frame(nullptr)
    , model(nullptr)
    , profileSession(nullptr)
{
    ui->setupUi(this);

    toolbar = new QToolBar;
    {
        QAction* actionDump = toolbar->addAction("Memory dump");
        connect(actionDump, SIGNAL(triggered()), this, SIGNAL(OnDumpButton()));
        QAction* actionViewDump = toolbar->addAction("View last memory dump");
        connect(actionViewDump, SIGNAL(triggered()), this, SIGNAL(OnViewDumpButton()));
        QAction* actionViewFileDump = toolbar->addAction("View dump from file");
        connect(actionViewFileDump, SIGNAL(triggered()), this, SIGNAL(OnViewFileDumpButton()));
    }
    ui->verticalLayout_2->insertWidget(0, toolbar);

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

        ui->allocPoolTable->setModel(allocPoolModel);
        ui->tagTable->setModel(tagModel);
    }
    
    model = new MemProfInfoModel();
}

MemProfWidget::~MemProfWidget() 
{
    delete model;
}

void MemProfWidget::ConnectionEstablished(bool newConnection, ProfilingSession* profSession)
{
    profileSession = profSession;
    allocPoolModel->BeginNewProfileSession(profSession);
    tagModel->BeginNewProfileSession(profSession);

    ui->labelStatus->setText("Connection established");
    if (newConnection)
    {
        ReinitPlot();
    }
}

void MemProfWidget::ConnectionLost(const char8* message)
{
    ui->labelStatus->setText(message != nullptr ? QString("Connection lost: %1").arg(message)
                                                : QString("Connection lost"));
}

void MemProfWidget::StatArrived()
{
    const StatItem& stat = profileSession->LastStat();
    uint32 alloc = stat.TotalStat().allocByApp;
    uint32 total = stat.TotalStat().allocTotal;

    allocPoolModel->SetCurrentValues(stat);
    tagModel->SetCurrentValues(stat);
    UpdatePlot(stat);
}

void MemProfWidget::UpdatePlot(const StatItem& stat)
{
    MemProfPlot* plot = ui->plot;
    size_t ngraph = profileSession->AllocPoolCount();

    for (size_t i = 0;i < ngraph;++i)
    {
        QCPGraph* graph = plot->graph(i);

        double key = static_cast<double>(stat.Timestamp()) / 1000.0;
        double val = static_cast<double>(stat.PoolStat()[i].allocByApp) / 1024.0 / 1024.0;
        graph->addData(key, val);

        graph->rescaleAxes(i > 0);
    }

    plot->replot();
}

void MemProfWidget::ReinitPlot()
{
    MemProfPlot* plot = ui->plot;
    size_t ngraph = profileSession->AllocPoolCount();

    const size_t ncolors = poolColors.size();

    plot->clearGraphs();
    for (size_t i = 0;i < ngraph;++i)
    {
        QCPGraph* graph = plot->addGraph();
        QPen pen(poolColors[i % ncolors]);
        pen.setWidth(2);
        graph->setPen(pen);
        //graph->setPen(poolColors[i % ncolors]);
        //graph->setBrush(poolColors[i % ncolors]);
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

void MemProfWidget::ShowDump(const DAVA::Vector<DAVA::uint8>& v)
{
    //DumpViewWidget* w = new DumpViewWidget(v, this);
}

void MemProfWidget::UpdateProgress(size_t total, size_t recv)
{
    int v = static_cast<int>(double(recv) / double(total) * 100.0);
    ui->dumpProgress->setValue(v);
}
