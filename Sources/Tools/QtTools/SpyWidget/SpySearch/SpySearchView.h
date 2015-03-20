#ifndef QTTOOLS_SPYSEARCHVIEW_H
#define QTTOOLS_SPYSEARCHVIEW_H


#include <QWidget>
#include <QPointer>


#include "ui_SpySearchView.h"


class QAbstractItemModel;


class SpySearchView
    : public QWidget
    , public Ui::SpySearchView
{
    Q_OBJECT

public:
    struct SearchInfo
    {
    };

signals:
    void triggered( const QPoint& globalPos );

public:
    explicit SpySearchView( QWidget *parent = nullptr );
    ~SpySearchView();

private slots:
    void OnSelectionStarted();
    void OnSelectionDone();
};


#endif // QTTOOLS_SPYSEARCHVIEW_H
