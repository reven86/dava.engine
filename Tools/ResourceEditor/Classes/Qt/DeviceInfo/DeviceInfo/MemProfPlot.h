#pragma once
#include "qcustomplot.h"
class MemProfInfoModel;
struct TagsStat;
class PlotPoint;
class MemProfPlot : public QCustomPlot
{
    Q_OBJECT
public:
    explicit MemProfPlot(QWidget *parent = 0);
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    ~MemProfPlot();
    void setModel(MemProfInfoModel * model);
    
protected slots:
    //bind to model data change signal
    void dataChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight);
    void modelReset();
   
protected:
    bool fixedRange = false;
    MemProfInfoModel * model;
    size_t lastTimeStamp = 0;
    void addStat(int stamp, const TagsStat & stat);
    PlotPoint * point;
};

