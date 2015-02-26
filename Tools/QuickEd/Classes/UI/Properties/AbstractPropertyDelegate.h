#ifndef __ABSTRACTPROPERTYDELEGATE_H__
#define __ABSTRACTPROPERTYDELEGATE_H__

#include <QList>
#include <QPointer>

class PropertiesTreeItemDelegate;
class QAction;
class QWidget;
class QModelIndex;
class QStyleOptionViewItem;
class QAbstractItemModel;

class AbstractPropertyDelegate
{
public:
    explicit AbstractPropertyDelegate(PropertiesTreeItemDelegate *delegate = NULL);
    virtual ~AbstractPropertyDelegate();

    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const = 0;
    virtual void enumEditorActions(QWidget *parent, const QModelIndex &index, QList<QAction *> &actions) const{};
    virtual void setEditorData(QWidget *editor, const QModelIndex &index) const = 0;
    virtual bool setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const = 0;

protected:
    QPointer<PropertiesTreeItemDelegate> itemDelegate;

};

/*
class PropertyAbstractEditor: public QObject
{
Q_OBJECT
public:
explicit PropertyAbstractEditor(PropertiesTreeItemDelegate *delegate = NULL);
virtual ~PropertyAbstractEditor();

virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const { return NULL; };
virtual void addEditorWidgets(QWidget *parent, const QModelIndex &index) const;
virtual QList<QAction *> enumEditorActions(QWidget *parent, const QModelIndex &index) const;
virtual void setEditorData(QWidget *editor, const QModelIndex &index) const = 0;
virtual bool setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
virtual void paint( QPainter * painter, QStyleOptionViewItem & option, const QModelIndex & index ) const{}

static bool IsValueModified(QWidget *editor)
{
QVariant data = editor->property("valueModified");
return data.toBool();
}

static void SetValueModified(QWidget *editor, bool value)
{
editor->setProperty("valueModified", QVariant(value));
}

static bool IsValueReseted(QWidget *editor)
{
QVariant data = editor->property("valueReseted");
return data.toBool();
}

static void SetValueReseted(QWidget *editor, bool value)
{
editor->setProperty("valueReseted", QVariant(value));
}
private slots:
void resetClicked();

protected:
PropertiesTreeItemDelegate *itemDelegate;

};
*/
#endif // __ABSTRACTPROPERTYDELEGATE_H__
