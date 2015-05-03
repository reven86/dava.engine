#ifndef QUICKED_BASECONTROLLER_H
#define QUICKED_BASECONTROLLER_H

#include <QObject>

class QAction;
class Document;
class DocumentGroup;
class Project;
class MainWindow;
class PackageNode;

class EditorCore : public QObject
{
    Q_OBJECT
public:
    explicit EditorCore(QObject *parent = nullptr);
    ~EditorCore();
    void Start();

    MainWindow *GetMainWindow() const;

protected slots:
    void OnCleanChanged(bool clean);
    void OnOpenPackageFile(const QString &path);

    bool CloseOneDocument(int index);
    void SaveDocument(int index);
    void SaveAllDocuments();

    void Exit();
    void RecentMenu(QAction *);
    void OnCurrentTabChanged(int index);
    
    void UpdateLanguage();
    void UpdateFontPreset();
    void NewFontPreset(const QString &oldPresetName, const QString &newPresetName);

protected:
    void OpenProject(const QString &path);
    bool CloseProject();
    int CreateDocument(PackageNode *package);
    void SaveDocument(Document *document);

private:
    bool eventFilter( QObject *obj, QEvent *event ) override;
    void CloseDocument(int index);
    int GetIndexByPackagePath(const QString &fileName) const;
    ///Return: pointer to currentDocument if exists, nullptr if not
    Project *project;
    QList<Document*> documents;
    DocumentGroup *documentGroup;
    MainWindow *mainWindow;
};

#endif // QUICKED_BASECONTROLLER_H
