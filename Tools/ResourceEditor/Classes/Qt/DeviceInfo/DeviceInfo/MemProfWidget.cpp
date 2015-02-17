#include <QLabel>
#include <QVBoxLayout>

#include "Base/BaseTypes.h"
#include "MemProfWidget.h"
#include "qcustomplot.h"

#include "ui_MemProfWidget.h"

MemProfWidget::MemProfWidget(QWidget *parent)
    : QWidget(parent, Qt::Window)
    , ui(new Ui::MemProfWidget())
{
    ui->setupUi(this);
    CreateUI();
    
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
void MemProfWidget::CreateUI()
{
    const char* t[] = {
        "Internal use",
        "Allocated by new",
        "Allicated for STL",
        "Allocated for class",
        "Other alloc",
        "Total"
    };
    QGridLayout* l = new QGridLayout();
    for (int i = 0;i < COUNT_OF(t);++i)
    {
        labels[i].title = new QLabel(t[i]);
        labels[i].alloc = new QLabel("0");
        labels[i].total = new QLabel("0");
        labels[i].max_block_size = new QLabel("0");
        labels[i].nblocks = new QLabel("0");
        
        l->addWidget(labels[i].title, i, 0);
        l->addWidget(labels[i].alloc, i, 1);
        l->addWidget(labels[i].total, i, 2);
        l->addWidget(labels[i].max_block_size, i, 3);
        l->addWidget(labels[i].nblocks, i, 4);
    }
    ui->verticalLayout_2->addLayout(l);
}
