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

#include "DumpViewWidget.h"

using namespace DAVA;

MemProfWidget::LabelPack::~LabelPack()
{
    delete allocInternal;
    delete internalBlockCount;
    delete ghostBlockCount;
    delete ghostSize;
    delete realSize;
}
MemProfWidget::MemProfWidget(QWidget *parent)
    : QWidget(parent, Qt::Window)
    , ui(new Ui::MemProfWidget())
    , tagCount(0)
    , allocPoolCount(0)
    , toolbar(nullptr)
    , frame(nullptr)
    , labels(nullptr)
    , model(nullptr)
    , tableView(nullptr)
{
    ui->setupUi(this);

    toolbar = new QToolBar;
    QAction* actionDump = toolbar->addAction("Memory dump");
    connect(actionDump, SIGNAL(triggered()), this, SIGNAL(OnDumpButton()));
    QAction* actionViewDump = toolbar->addAction("View last memory dump");
    connect(actionViewDump, SIGNAL(triggered()), this, SIGNAL(OnViewDumpButton()));
    ui->verticalLayout_2->insertWidget(0, toolbar);
    
    plot = ui->plot;
    
    plot->addGraph();
    plot->graph(0)->setPen(QPen(Qt::blue));
    plot->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20)));
    plot->graph(0)->setAntialiasedFill(false);
    
    plot->addGraph();
    plot->graph(1)->setPen(QPen(Qt::red));
    //plot->graph(1)->setBrush(QBrush(QColor(0, 255, 0, 20)));
    plot->graph(1)->setAntialiasedFill(false);
    
    plot->xAxis2->setVisible(true);
    plot->xAxis2->setTickLabels(false);
    plot->yAxis2->setVisible(true);
    plot->yAxis2->setTickLabels(false);
    
    connect(plot->xAxis, SIGNAL(rangeChanged(QCPRange)), plot->xAxis2, SLOT(setRange(QCPRange)));
    connect(plot->yAxis, SIGNAL(rangeChanged(QCPRange)), plot->yAxis2, SLOT(setRange(QCPRange)));
    
    plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
    
    tableView = new QTableView(this);
    model = new MemProfInfoModel();
    tableView->setModel(model);
    tableView->setItemDelegate(new MemoryItemStyleDelegate(tableView));
    plot->setModel(model);
}

MemProfWidget::~MemProfWidget() 
{
    delete model;
}

void MemProfWidget::AppendText(const QString& text)
{

}

void MemProfWidget::ShowDump(const DAVA::Vector<DAVA::uint8>& v)
{
    //DumpViewWidget* w = new DumpViewWidget(v, this);
}

void MemProfWidget::ChangeStatus(const char* status, const char* reason)
{
    QString s(status);
    if (reason)
    {
        s += ": ";
        s += reason;
    }
    ui->labelStatus->setText(s);
}

void MemProfWidget::ClearStat()
{
    plot->graph(0)->clearData();
    plot->graph(1)->clearData();
    plot->graph(0)->rescaleValueAxis();
    plot->graph(1)->rescaleValueAxis(true);
    plot->replot();
}

void MemProfWidget::SetStatConfig(const DAVA::MMStatConfig* config)
{
    if (config != nullptr)
    {
        tagCount = config->tagCount;
        allocPoolCount = config->allocPoolCount;
        CreateLabels(config);
    }
    model->setConfig(config);
}

void MemProfWidget::UpdateStat(const MMStat* stat)
{
    uint32 alloc = 0;
    uint32 total = 0;
    for (uint32 i = 0;i < stat->allocPoolCount;++i)
    {
        alloc += stat->poolStat[i].allocByApp;
        total += stat->poolStat[i].allocTotal;
    }

    model->addMoreData(stat);
    UpdateLabels(stat, alloc, total);
}

void MemProfWidget::UpdateProgress(size_t total, size_t recv)
{
    int v = static_cast<int>(double(recv) / double(total) * 100.0);
    ui->dumpProgress->setValue(v);
}

void MemProfWidget::UpdateLabels(const DAVA::MMStat* stat, DAVA::uint32 alloc, DAVA::uint32 total)
{
   
    labels[1].alloc->setText(MemoryItemStyleDelegate::formatMemoryData(alloc));
    labels[1].total->setText(MemoryItemStyleDelegate::formatMemoryData(total));
    labels[1].allocInternal->setText(MemoryItemStyleDelegate::formatMemoryData(stat->generalStat.allocInternal));
    labels[1].ghostBlockCount->setNum(static_cast<int>(stat->generalStat.ghostBlockCount));
    labels[1].ghostSize->setText(MemoryItemStyleDelegate::formatMemoryData(stat->generalStat.ghostSize));
    labels[1].realSize->setText(MemoryItemStyleDelegate::formatMemoryData(stat->generalStat.realSize));
    labels[1].internalBlockCount->setNum(static_cast<int>(stat->generalStat.internalBlockCount));
}

/*
void MemProfWidget::UpdateStat(const net_mem_stat_t* stat)
{
    uint32_t alloc = 0;
    uint32_t total = 0;
    for (int i = 0;i < MEMPROF_MEM_COUNT;++i)
    {
        alloc += stat->stat[i][0].alloc_size;
        total += stat->stat[i][0].total_size;
    }

    //plot->graph(0)->addData(stat->timestamp, total);
    plot->graph(0)->addData(offset, (double)alloc/1000.0);
    plot->graph(1)->addData(offset, (double)total/1000.0);
    plot->graph(0)->rescaleAxes();
    plot->graph(1)->rescaleAxes(true);
    plot->replot();
    
    UpdateLabels(stat, alloc, total);
    offset += 1;
}
*/
/*
void MemProfWidget::UpdateLabels(const net_mem_stat_t* stat, uint32_t alloc, uint32_t total)
{
    uint32_t nblocks = 0;
    for (int i = 0;i < MEMPROF_MEM_COUNT;++i)
    {
        labels[i].alloc->setNum(int(stat->stat[i][0].alloc_size));
        labels[i].total->setNum(int(stat->stat[i][0].total_size));
        labels[i].max_block_size->setNum(int(stat->stat[i][0].max_block_size));
        labels[i].nblocks->setNum(int(stat->stat[i][0].nblocks));
        nblocks += stat->stat[i][0].nblocks;
    }
    
    const int R = MEMPROF_MEM_COUNT;
    labels[R].alloc->setNum(int(alloc));
    labels[R].total->setNum(int(total));
    //labels[R].max_block_size->setNum(int(stat->stat[i][0].max_block_size));
    labels[R].nblocks->setNum(int(nblocks));
}
*/

void MemProfWidget::CreateLabels(const DAVA::MMStatConfig* config)
{
    Deletelabels();
    QGridLayout* l = new QGridLayout();

    //one fro titles and one for 
    labels = new LabelPack[2];
   
    labels[0].alloc = new QLabel("alloc");
    labels[0].total = new QLabel("total");
    labels[0].allocInternal = new QLabel("allocInternal");
    labels[0].ghostBlockCount = new QLabel("ghostBlockCount");
    labels[0].ghostSize = new QLabel("ghostSize");
    labels[0].realSize = new QLabel("realSize");
    labels[0].internalBlockCount = new QLabel("internalBlockCount");

    l->addWidget(labels[0].alloc , 0, 0);
    l->addWidget(labels[0].total, 0, 1);
    l->addWidget(labels[0].allocInternal, 0, 2);
    l->addWidget(labels[0].ghostBlockCount, 0, 3);
    l->addWidget(labels[0].ghostSize, 0, 4);
    l->addWidget(labels[0].realSize, 0, 5);
    l->addWidget(labels[0].internalBlockCount, 0, 6);

    labels[1].alloc = new QLabel("");
    labels[1].total = new QLabel("");
    labels[1].allocInternal = new QLabel("");
    labels[1].ghostBlockCount = new QLabel("");
    labels[1].ghostSize = new QLabel("");
    labels[1].realSize = new QLabel("");
    labels[1].internalBlockCount = new QLabel("");

    l->addWidget(labels[1].alloc, 1, 0);
    l->addWidget(labels[1].total, 1, 1);
    l->addWidget(labels[1].allocInternal, 1, 2);
    l->addWidget(labels[1].ghostBlockCount, 1, 3);
    l->addWidget(labels[1].ghostSize, 1, 4);
    l->addWidget(labels[1].realSize, 1, 5);
    l->addWidget(labels[1].internalBlockCount, 1, 6);


    frame = new QFrame;
    frame->setLayout(l);
    ui->verticalLayout_2->addWidget(frame);
    ui->verticalLayout_2->addWidget(tableView);
}

void MemProfWidget::Deletelabels()
{
    if (frame)
    {
        delete[] labels;
        delete frame;
        frame = nullptr;
    }
}
