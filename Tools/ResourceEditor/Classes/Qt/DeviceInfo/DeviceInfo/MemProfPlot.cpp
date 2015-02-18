#include "MemProfPlot.h"
#include "MemProfInfoModel.h"

MemProfPlot::MemProfPlot(QWidget *parent )
{
    model = nullptr;
}

void MemProfPlot::setModel(MemProfInfoModel * model)
{
    
   // assert(model != nullptr);
    this->model = model;
    connect(model, 
        SIGNAL(dataChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight)), 
        this, 
        SLOT(dataChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight)));


}
MemProfPlot::~MemProfPlot()
{
}
void MemProfPlot::mousePressEvent(QMouseEvent *event)
{
    QCustomPlot::mousePressEvent(event);
}
void MemProfPlot::mouseMoveEvent(QMouseEvent *event)
{
    QCustomPlot::mouseMoveEvent(event);
}
void MemProfPlot::dataChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight)
{
   
}