#ifndef QTTOOLS_SPYSEARCHVIEW_H
#define QTTOOLS_SPYSEARCHVIEW_H

#include "QtTools/WarningGuard/QtWarningsHandler.h"
PUSH_QT_WARNING_SUPRESSOR
#include <QWidget>
#include <QPointer>

#include "ui_SpySearchView.h"
POP_QT_WARNING_SUPRESSOR

class QAbstractItemModel;

class SpySearchView : public QWidget, public Ui::SpySearchView
{
    PUSH_QT_WARNING_SUPRESSOR
    Q_OBJECT
    POP_QT_WARNING_SUPRESSOR

public:
    struct SearchInfo
    {
    };

signals:
    void triggered(const QPoint& globalPos);

public:
    explicit SpySearchView(QWidget* parent = nullptr);
    ~SpySearchView();

private slots:
    void OnSelectionStarted();
    void OnSelectionDone();
};


#endif // QTTOOLS_SPYSEARCHVIEW_H
