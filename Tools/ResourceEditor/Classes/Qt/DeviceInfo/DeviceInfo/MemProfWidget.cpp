#include <QLabel>
#include <QVBoxLayout>

#include "MemProfWidget.h"
#include "MemProfPlot.h"
#include "MemProfInfoModel.h"
#include "ui_MemProfWidget.h"
#include "MemoryManager/MemoryManager.h"
MemProfWidget::MemProfWidget(QWidget *parent)
    : QWidget(parent, Qt::Window)
    , ui(new Ui::MemProfWidget())
{
    model = nullptr;
    ui->setupUi(this);
    CreateUI();
    
    plot = ui->plot;
    
    prevOrder = (DAVA::uint32)-1;
    offset = 0;
    
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
   // plot->setInteraction(QCP::Interaction)
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
void MemProfWidget::SetModel(MemProfInfoModel * model)
{
    this->model = model;
    if (dataView != nullptr)
    {
        dataView->setModel(model);
    }
}
void MemProfWidget::ClearStat()
{
    plot->graph(0)->clearData();
    plot->graph(1)->clearData();
    plot->graph(0)->rescaleValueAxis();
    plot->graph(1)->rescaleValueAxis(true);
    plot->replot();
}

void MemProfWidget::UpdateStat(const MemoryProfDataChunk* stat)
{
    DAVA::uint32 alloc = 0;
    DAVA::uint32 total = 0;
   /* for (int i = 0;i < DAVA::MemoryManager::MAX_ALLOC_POOL_COUNT;++i)
    {
        alloc += stat->stat[i][0].allocByApp;
        total += stat->stat[i][0].allocTotal;
    }*/

    //plot->graph(0)->addData(stat->timestamp, total);
    plot->graph(0)->addData(offset, (double)alloc/1000.0);
    plot->graph(1)->addData(offset, (double)total/1000.0);
    plot->graph(0)->rescaleAxes();
    plot->graph(1)->rescaleAxes(true);
    plot->replot();
    
    UpdateLabels(stat, alloc, total);
    offset += 1;
}

void MemProfWidget::UpdateLabels(const MemoryProfDataChunk* stat, DAVA::uint32 alloc, DAVA::uint32 total)
{
    DAVA::uint32 nblocks = 0;
   
    
}

void MemProfWidget::CreateUI()
{
   
    dataView = new QTableView();

    ui->verticalLayout_2->addWidget(dataView);
}
