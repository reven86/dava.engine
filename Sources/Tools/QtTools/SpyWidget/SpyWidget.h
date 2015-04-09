#ifndef QTTOOLS_SPYWIDGET_H
#define QTTOOLS_SPYWIDGET_H

#include <QWidget>

#include "ui_SpyWidget.h"



class SpyWidget
    : public QWidget
    , public Ui::SpyWidget
{
    Q_OBJECT

public:
    explicit SpyWidget( QWidget *parent = nullptr );
    ~SpyWidget();

private slots:
    void onKeepOnTopChanged();
};


#endif // QTTOOLS_SPYWIDGET_H
