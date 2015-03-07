#include "MemProfPlot.h"
#include "MemProfInfoModel.h"
#include "plotpoint.h"
MemProfPlot::MemProfPlot(QWidget *parent )
{
    model = nullptr;
    point = new PlotPoint(this);
    point->setVisible(true);
    point->setActive(true);
}

void MemProfPlot::setModel(MemProfInfoModel * model)
{
    
   // assert(model != nullptr);
    this->model = model;
    connect(model, 
        SIGNAL(dataChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight)), 
        this, 
        SLOT(dataChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight)));


    connect(model,
        SIGNAL(modelReset()),
        this,
        SLOT(modelReset()));


}
MemProfPlot::~MemProfPlot()
{
}
void MemProfPlot::mousePressEvent(QMouseEvent *event)
{
    QCustomPlot::mousePressEvent(event);
    if (event->button() == Qt::LeftButton && point) {
        point->startMoving(event->localPos(),
            event->modifiers().testFlag(Qt::ShiftModifier));
        return;
    }
}
void MemProfPlot::mouseMoveEvent(QMouseEvent *event)
{
    QCustomPlot::mouseMoveEvent(event);
    if (event->buttons() == Qt::NoButton) {
        PlotPoint *plotPoint = qobject_cast<PlotPoint*>(itemAt(event->localPos(), true));
        if (plotPoint != point) {
            if (point == NULL) {
                // cursor moved from empty space to item
                plotPoint->setActive(true);
                setCursor(Qt::OpenHandCursor);
            }
            else if (plotPoint == NULL) {
                // cursor move from item to empty space
                point->setActive(false);
                unsetCursor();
            }
            else {
                // cursor moved from item to item
                point->setActive(false);
                plotPoint->setActive(true);
            }
            point = plotPoint;
            replot();
        }
    }
}
void MemProfPlot::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (fixedRange)
    {
        model->forTagStats([this](int stamp,const TagsStat & stat){
            if (stamp > lastTimeStamp)
            {
                addStat(stamp, stat);
            }
        });
        model->showLatestData();
    }
    else
    {

        int x = this->xAxis->pixelToCoord(event->pos().x());
        int y = this->yAxis->pixelToCoord(event->pos().y());

        model->showDataToClosest(x*1000.0);
    }
    fixedRange = !fixedRange;
    
}
void MemProfPlot::addStat(int stamp, const TagsStat & stat)
{
    if (stamp < lastTimeStamp) 
        return;

    size_t alloc = 0, total = 0;
    for (auto & pool : stat.statData)
    {
        alloc += pool[0].allocByApp;
        total += pool[0].allocTotal;
    }
   

    graph(0)->addData(double(stamp) / 1000., (double)alloc / (1024. * 1024.));
    graph(1)->addData(double(stamp) / 1000., (double)total / (1024. * 1024.));
    graph(0)->rescaleAxes();
    graph(1)->rescaleAxes(true);
    replot();
    
    lastTimeStamp = stamp;
}
void MemProfPlot::modelReset()
{
    if (!fixedRange)
    {
        const TagsStat & stat = model->getCurrentTagStat();
        addStat(model->getCurrentTimeStamp(), stat);
    }
}
void MemProfPlot::dataChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight)
{

}