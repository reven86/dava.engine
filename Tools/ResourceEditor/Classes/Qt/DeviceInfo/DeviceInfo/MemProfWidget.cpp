#include <QLabel>
#include <QFrame>
#include <QGridLayout>
#include <QVBoxLayout>

#include "Base/BaseTypes.h"
#include "MemoryManager/MemoryManagerTypes.h"
#include "MemProfWidget.h"
#include "qcustomplot.h"

#include "ui_MemProfWidget.h"

using namespace DAVA;

MemProfWidget::label_pack::~label_pack()
{
    delete title;
    delete alloc;
    delete total;
    delete max_block_size;
    delete nblocks;
}

MemProfWidget::MemProfWidget(QWidget *parent)
    : QWidget(parent, Qt::Window)
    , ui(new Ui::MemProfWidget())
    , tagCount(0)
    , allocPoolCount(0)
    , labels(nullptr)
    , frame(nullptr)
{
    ui->setupUi(this);

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
    
    //plot->graph(0)->setData(x, y0);
    //plot->graph(1)->setData(x, y1);
    
    //plot->graph(0)->rescaleAxes();
    //plot->graph(1)->rescaleAxes(true);
    plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
}

MemProfWidget::~MemProfWidget() {}

void MemProfWidget::AppendText(const QString& text)
{

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

    plot->graph(0)->addData(double(stat->timestamp), (double)alloc / 1000.0);
    plot->graph(1)->addData(double(stat->timestamp), (double)total / 1000.0);
    plot->graph(0)->rescaleAxes();
    plot->graph(1)->rescaleAxes(true);
    plot->replot();

    UpdateLabels(stat, alloc, total);
}

void MemProfWidget::UpdateLabels(const DAVA::MMStat* stat, DAVA::uint32 alloc, DAVA::uint32 total)
{
    uint32 nblocks = 0;
    for (uint32 i = 0;i < stat->allocPoolCount;++i)
    {
        labels[i].alloc->setNum(int(stat->poolStat[i].allocByApp));
        labels[i].total->setNum(int(stat->poolStat[i].allocTotal));
        labels[i].max_block_size->setNum(int(stat->poolStat[i].maxBlockSize));
        labels[i].nblocks->setNum(int(stat->poolStat[i].blockCount));
        nblocks += stat->poolStat[i].blockCount;
    }

    const uint32 R = stat->allocPoolCount;
    labels[R].alloc->setNum(int(alloc));
    labels[R].total->setNum(int(total));
    labels[R].nblocks->setNum(int(nblocks));
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

    labels = new label_pack[allocPoolCount + 1];
    for (uint32 i = 0;i < allocPoolCount + 1;++i)
    {
        labels[i].title = new QLabel("");
        labels[i].alloc = new QLabel("-");
        labels[i].total = new QLabel("-");
        labels[i].max_block_size = new QLabel("-");
        labels[i].nblocks = new QLabel("-");

        l->addWidget(labels[i].title, i, 0);
        l->addWidget(labels[i].alloc, i, 1);
        l->addWidget(labels[i].total, i, 2);
        l->addWidget(labels[i].max_block_size, i, 3);
        l->addWidget(labels[i].nblocks, i, 4);
    }
    for (uint32 i = 0;i < allocPoolCount;++i)
    {
        labels[i].title->setText(config->names[i + config->tagCount].name);
    }
    labels[allocPoolCount].title->setText("Total");

    frame = new QFrame;
    frame->setLayout(l);
    ui->verticalLayout_2->addWidget(frame);
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
