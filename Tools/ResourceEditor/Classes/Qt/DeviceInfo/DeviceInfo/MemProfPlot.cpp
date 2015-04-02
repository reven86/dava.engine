#include "MemProfPlot.h"
#include "MemProfInfoModel.h"

MemProfPlot::MemProfPlot(QWidget *parent )
{
    model = nullptr;

    horizontal = new QCPItemStraightLine(this);
    horizontal->setAntialiased(false);
    horizontal->setVisible(true);
    addItem(horizontal);

    vertical = new QCPItemStraightLine(this);
    vertical->setAntialiased(false);
    vertical->setVisible(true);
    addItem(vertical);
}

MemProfPlot::~MemProfPlot()
{
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

void MemProfPlot::mousePressEvent(QMouseEvent *event)
{
    QCustomPlot::mousePressEvent(event);
}

void MemProfPlot::mouseMoveEvent(QMouseEvent *event)
{
    if (!fixedRange)
    {
        QPointF mGripDelta;
        mGripDelta.setX(xAxis->pixelToCoord(event->pos().x()));
        mGripDelta.setY(yAxis->pixelToCoord(event->pos().y()));

        horizontal->point1->setCoords(mGripDelta.x(), mGripDelta.y());
        horizontal->point2->setCoords(mGripDelta.x()+1, mGripDelta.y());

        vertical->point1->setCoords(mGripDelta.x(), mGripDelta.y());
        vertical->point2->setCoords(mGripDelta.x(), mGripDelta.y() + 1);
        replot();
    }

    QCustomPlot::mouseMoveEvent(event);
    
}

void MemProfPlot::mouseDoubleClickEvent(QMouseEvent *event)
{
    /*if (fixedRange)
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
    */
}

void MemProfPlot::addStat(int stamp, const TagsStat & stat)
{
    if (stamp < lastTimeStamp) 
        return;

    size_t alloc = 0, total = 0;
    for (auto & lable : stat.statData)
        for (auto & pool : lable)
        {
            alloc += pool.allocByApp;
            total += pool.allocTotal;
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
    /*if (!fixedRange)
    {
        const TagsStat & stat = model->getCurrentTagStat();
        addStat(model->getCurrentTimeStamp(), stat);
    }*/
}

void MemProfPlot::dataChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight)
{
}
