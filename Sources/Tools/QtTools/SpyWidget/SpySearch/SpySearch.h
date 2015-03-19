#ifndef QTTOOLS_SPYSEARCH_H
#define QTTOOLS_SPYSEARCH_H


#include <QObject>
#include <QPointer>
#include <QPoint>


class SpySearchView;


class SpySearch
    : public QObject
{
    Q_OBJECT

public:
    explicit SpySearch( QObject *parent = nullptr );
    ~SpySearch();

    SpySearchView *GetView() const;
    void Show();

private slots:
    void trigger( const QPoint& pos );

private:
    QPointer< SpySearchView > view;
};


#endif // QTTOOLS_SPYSEARCH_H
