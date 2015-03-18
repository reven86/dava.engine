#ifndef QTTOOLS_SPYDRAGWIDGET_H
#define QTTOOLS_SPYDRAGWIDGET_H


#include <QLabel>



class SpyDragWidget
    : public QLabel
{
    Q_OBJECT

public:
    explicit SpyDragWidget( QWidget *parent = nullptr );
    ~SpyDragWidget();
};


#endif // QTTOOLS_SPYDRAGWIDGET_H
