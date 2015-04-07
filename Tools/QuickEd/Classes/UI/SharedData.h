#ifndef __QUICKED_WIDGET_CONTEXT_H__
#define __QUICKED_WIDGET_CONTEXT_H__

#include <QtCore>
#include <QWidget>
#include <QSharedPointer>
#include <DAVAEngine.h>
#include "Model/PackageHierarchy/ControlNode.h"
#include "Document.h"

struct WidgetContext
{
};

class SharedData : public QObject
{
    Q_OBJECT
public:
    SharedData(QObject *parent = nullptr);
    QVariant& GetData(const QByteArray &role);
    Document *GetDocument() const; //TODO - this is deprecated
    void SetData(const QByteArray &role, const QVariant &value);
    WidgetContext* GetContext(QWidget* requester) const;
    void SetContext(QWidget* requester, WidgetContext* widgetContext);
signals:
    void DataChanged(const QByteArray &role);
private:
    QMap < QByteArray, QVariant > values;
    std::map < QWidget*, std::unique_ptr<WidgetContext> > contexts;
};

inline Document* SharedData::GetDocument() const
{
    return qobject_cast<Document*>(parent());
}


Q_DECLARE_METATYPE(SharedData*);
Q_DECLARE_METATYPE(QAbstractItemModel*)
Q_DECLARE_METATYPE(QPointer<QAbstractItemModel>)
Q_DECLARE_METATYPE(QItemSelection);
Q_DECLARE_METATYPE(ControlNode*);
Q_DECLARE_METATYPE(QList<ControlNode*>);
Q_DECLARE_METATYPE(DAVA::UIControl*);
Q_DECLARE_METATYPE(QPersistentModelIndex);
Q_DECLARE_METATYPE(QList<QPersistentModelIndex>);

#endif // __QUICKED_WIDGET_CONTEXT_H__
