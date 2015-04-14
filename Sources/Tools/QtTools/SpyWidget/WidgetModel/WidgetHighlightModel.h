#ifndef QTTOOLS_WIDGETHIGHLIGHTMODEL_H
#define QTTOOLS_WIDGETHIGHLIGHTMODEL_H


#include <QIdentityProxyModel>
#include <QSet>
#include <QWidget>


class WidgetHighlightModel
    : public QIdentityProxyModel
{
    Q_OBJECT

public:
    explicit WidgetHighlightModel( QObject *parent = nullptr );
    ~WidgetHighlightModel();

    QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const override;

    void setWidgetList( const QSet< QWidget * >& widgetsToHighlight = QSet< QWidget * >() );

private slots:
    void onWidgetDestroyed();

private:
    void invalidate();

    QSet< QWidget * > widgets;
};


#endif // QTTOOLS_WIDGETHIGHLIGHTMODEL_H
