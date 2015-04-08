#ifndef QTTOOLS_WIDGETSHADE_H
#define QTTOOLS_WIDGETSHADE_H


#include <QWidget>
#include <QPointer>



class WidgetShade
    : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetShade( QWidget *source );
    ~WidgetShade();
};


#endif // QTTOOLS_WIDGETSHADE_H
