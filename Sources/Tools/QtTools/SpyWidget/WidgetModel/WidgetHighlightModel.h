#ifndef QTTOOLS_WIDGETHIGHLIGHTMODEL_H
#define QTTOOLS_WIDGETHIGHLIGHTMODEL_H

#include "QtTools/WarningGuard/QtWarningsHandler.h"
PUSH_QT_WARNING_SUPRESSOR
#include <QIdentityProxyModel>
#include <QSet>
#include <QWidget>
POP_QT_WARNING_SUPRESSOR

class WidgetHighlightModel
: public QIdentityProxyModel
{
    PUSH_QT_WARNING_SUPRESSOR
    Q_OBJECT
    POP_QT_WARNING_SUPRESSOR

public:
    explicit WidgetHighlightModel(QObject* parent = nullptr);
    ~WidgetHighlightModel();

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    void setWidgetList(const QSet<QWidget*>& widgetsToHighlight = QSet<QWidget*>());

private slots:
    void onWidgetDestroyed();

private:
    void invalidate();

    QSet<QWidget*> widgets;
};


#endif // QTTOOLS_WIDGETHIGHLIGHTMODEL_H
