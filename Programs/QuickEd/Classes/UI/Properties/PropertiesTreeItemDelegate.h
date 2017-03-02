#ifndef __PROPERTIESTREEITEMDELEGATE_H__
#define __PROPERTIESTREEITEMDELEGATE_H__

#include <QWidget>
#include <QVector2D>
#include <QLineEdit>
#include <QStyledItemDelegate>
#include "Model/ControlProperties/AbstractProperty.h"

class AbstractPropertyDelegate;
class QToolButton;
class Project;

class PropertiesContext
{
public:
    const Project* project = nullptr;
};

class PropertiesTreeItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    PropertiesTreeItemDelegate(QObject* parent = NULL);
    ~PropertiesTreeItemDelegate();

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;

    virtual AbstractPropertyDelegate* GetCustomItemDelegateForIndex(const QModelIndex& index) const;

    void SetProject(const Project* project);

    void emitCommitData(QWidget* editor);
    void emitCloseEditor(QWidget* editor, QAbstractItemDelegate::EndEditHint hint);

protected:
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

    QModelIndex GetSourceIndex(QModelIndex index, QAbstractItemModel* itemModel) const;

    PropertiesContext context;

    struct PropertyPath
    {
        QString sectionName;
        QString propertyName;

        PropertyPath(const QString& sectionName_, const QString& propertyName_)
            : sectionName(sectionName_)
            , propertyName(propertyName_)
        {
        }

        bool operator<(const PropertyPath& other) const
        {
            if (sectionName == other.sectionName)
            {
                return propertyName < other.propertyName;
            }
            return sectionName < other.sectionName;
        }
    };

    QMap<AbstractProperty::ePropertyType, AbstractPropertyDelegate*> propertyItemDelegates;
    QMap<const DAVA::Type*, AbstractPropertyDelegate*> anyItemDelegates;
    QMap<PropertyPath, AbstractPropertyDelegate*> propertyNameTypeItemDelegates;
};
class PropertyWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PropertyWidget(QWidget* parent = NULL);
    ~PropertyWidget(){};

    bool event(QEvent* e) override;

    void keyPressEvent(QKeyEvent* e) override;

    void mousePressEvent(QMouseEvent* e) override;

    void mouseReleaseEvent(QMouseEvent* e) override;

    void focusInEvent(QFocusEvent* e) override;

    void focusOutEvent(QFocusEvent* e) override;

public:
    QWidget* editWidget;
};
#endif // __PROPERTIESTREEITEMDELEGATE_H__
