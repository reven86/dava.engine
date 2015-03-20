#ifndef QTTOOLS_SPYSEARCH_H
#define QTTOOLS_SPYSEARCH_H


#include <QObject>
#include <QPointer>
#include <QPoint>


class SpySearchView;
class WidgetListModel;


class SpySearch
    : public QObject
{
    Q_OBJECT

public:
    explicit SpySearch( QObject *parent = nullptr );
    ~SpySearch();

    SpySearchView *GetView() const;

public slots:
    void show();

private slots:
    void trigger( const QPoint& pos );
    void onWidgetSelect( const QModelIndex& index );

private:
    bool isSelf( QWidget *w ) const;
    void showWidgetInfo( QWidget *w ) const;

    QPointer< SpySearchView > view;
    QPointer< WidgetListModel > resultModel;
};


#endif // QTTOOLS_SPYSEARCH_H
