#pragma once

#include <QDialog>
#include <QMap>
#include <memory>

namespace Ui
{
class FindFileDialog;
}

class ProjectStructure;
class QCompleter;
class QAction;

class FindFileDialog : public QDialog
{
public:
    static QString GetFilePath(const ProjectStructure* projectStructure, const QString& extension, QWidget* parent);
    static QAction* CreateFindInFilesAction(QWidget* parent);

private:
    explicit FindFileDialog(const ProjectStructure* projectStructure, const QString& extension, QWidget* parent = nullptr);

    void Init(const QStringList& files);

    bool eventFilter(QObject* obj, QEvent* event);

    QString GetCommonParent(const QString& path1, const QString& path2);

    QString ToShortName(const QString& name) const;
    QString FromShortName(const QString& name) const;

    std::unique_ptr<Ui::FindFileDialog> ui;

    QString prefix;
    QCompleter* completer = nullptr;
};
