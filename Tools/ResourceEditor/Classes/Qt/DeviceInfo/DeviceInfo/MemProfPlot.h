#pragma once
#include "qcustomplot.h"
class MemProfInfoModel;
class MemProfPlot : public QCustomPlot
{
    Q_OBJECT
public:
    explicit MemProfPlot(QWidget *parent = 0);
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    ~MemProfPlot();
    void setModel(MemProfInfoModel * model);
protected slots:
    //bind to model data change signal
    void dataChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight);
protected:
    MemProfInfoModel * model;
};

