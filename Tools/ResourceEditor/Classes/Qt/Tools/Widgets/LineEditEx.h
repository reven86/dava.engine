#ifndef LINEEDITEX_H
#define LINEEDITEX_H


#include <QLineEdit>
#include <QPointer>
#include <QTimer>
#include <QMap>
#include <QSet>


class QAbstractButton;
class LineEditStyle;

class LineEditEx
    : public QLineEdit
{
    friend class LineEditStyle;

    Q_OBJECT

private:
    typedef QMap< QAction *, QAbstractButton * > ButtonsMap;
    typedef QSet< QWidget * > Widgets;

signals:
    void textUpdated(const QString& text);

public:
    explicit LineEditEx(QWidget* parent = NULL);
    ~LineEditEx();

    void SetAcceptInterval(int msec);
    bool IsDelayedUpdateUsed() const;
    void SetUseDelayedUpdate(bool use);

protected:
    virtual QAbstractButton * CreateButton( const QAction *action );
    virtual QSize ButtonSizeHint(const QAction *action) const;
    virtual void SyncButtonWithAction( const QAction *action, QAbstractButton *button);

    int ButtonsWidth() const;
    void AddCustomWidget(QWidget *w); // LineEditEx takes ownership
    void RemoveCustomWidget(QWidget *w);

private slots:
    void OnTextEdit();
    void OnAcceptEdit();
    void UpdatePadding();
    void OnActionChanged();

private:
    void SetupConnections( bool delayed, bool instant );
    void AddActionHandler(QAction *action);
    void RemoveActionHandler(QAction *action);
    
    void actionEvent(QActionEvent * event);

    // Delayed update
    QPointer<QTimer> timer;
    bool useDelayedUpdate;

    // Extra actions
    ButtonsMap buttons;
    Widgets widgets;
    int buttonsWidth;
};


#endif // LINEEDITEX_H
