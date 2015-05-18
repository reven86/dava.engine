#ifndef QTTOOLS_WIDGETSTATEHELPER_H
#define QTTOOLS_WIDGETSTATEHELPER_H

#include <QObject>
#include <QPointer>


class QWidget;


class WidgetStateHelper
    : public QObject
{
    Q_OBJECT

public:
    enum WidgetEvent
    {
        MaximizeOnShowOnce,
        ScaleOnDisplayChange,
    };
    Q_DECLARE_FLAGS( WidgetEvents, WidgetEvent )

public:
    explicit WidgetStateHelper( QObject *parent = nullptr );
    ~WidgetStateHelper();

    void startTrack( QWidget *w );
    WidgetEvents getTrackedEvents() const;
    void setTrackedEvents( const WidgetEvents& events );

    bool eventFilter( QObject * watched, QEvent * event ) override;

private slots:
    void stopTrack();

private:
    void onShowEvent();

    QPointer< QWidget > trackedWidget;
    WidgetEvents trackedEvents;

public:
    static WidgetStateHelper *create( QWidget *w );
};


Q_DECLARE_METATYPE( WidgetStateHelper::WidgetEvent )
Q_DECLARE_METATYPE( WidgetStateHelper::WidgetEvents )
Q_DECLARE_OPERATORS_FOR_FLAGS( WidgetStateHelper::WidgetEvents )


#endif // QTTOOLS_WIDGETSTATEHELPER_H
