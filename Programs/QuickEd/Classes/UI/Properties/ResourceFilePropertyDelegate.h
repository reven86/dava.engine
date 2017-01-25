#ifndef __RESOURCE_FILE_PROPERTY_DELEGATE_H__
#define __RESOURCE_FILE_PROPERTY_DELEGATE_H__

#include "BasePropertyDelegate.h"

class Project;

class ResourceFilePropertyDelegate : public BasePropertyDelegate
{
    Q_OBJECT
public:
    explicit ResourceFilePropertyDelegate(const QString& extension, const QString& resourceSubDir, PropertiesTreeItemDelegate* delegate);
    ~ResourceFilePropertyDelegate();

    QWidget* createEditor(QWidget* parent, const PropertiesContext& context, const QStyleOptionViewItem& option, const QModelIndex& index) override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    bool setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
    void enumEditorActions(QWidget* parent, const QModelIndex& index, QList<QAction*>& actions) override;

private slots:
    void selectFileClicked();
    void clearFileClicked();
    void OnEditingFinished();
    void OnTextChanged(const QString& text);

private:
    bool IsPathValid(const QString& path);
    QPointer<QLineEdit> lineEdit = nullptr;
    const Project* project = nullptr;

    QString resourceExtension;
    QString projectResourceDir;
    QString resourceSubDir;
};

#endif // __RESOURCE_FILE_PROPERTY_DELEGATE_H__
