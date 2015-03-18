#ifndef QTTOOLS_SPYSEARCHVIEW_H
#define QTTOOLS_SPYSEARCHVIEW_H


#include <QWidget>

#include "ui_SpySearchView.h"



class SpySearchView
    : public QWidget
    , public Ui::SpySearchView
{
    Q_OBJECT

public:
    explicit SpySearchView( QWidget *parent = nullptr );
    ~SpySearchView();
};


#endif // QTTOOLS_SPYSEARCHVIEW_H
