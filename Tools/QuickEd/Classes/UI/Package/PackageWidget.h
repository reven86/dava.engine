#ifndef __UI_EDITOR_UI_PACKAGE_WIDGET__
#define __UI_EDITOR_UI_PACKAGE_WIDGET__

#include <QWidget>
#include <QDockWidget>
#include <QPointer>
#include <QItemSelectionModel>
#include "UI/Package/FilteredPackageModel.h"
#include "UI/Package/PackageModel.h"
#include "DAVAEngine.h"
#include "ui_PackageWidget.h"

namespace Ui {
    class PackageWidget;
}

class ControlNode;
class SharedData;

class PackageWidget : public QDockWidget, public Ui::PackageWidget
{
    Q_OBJECT
public:
    explicit PackageWidget(QWidget *parent = 0);
    ~PackageWidget() = default;

public slots:
    void OnDocumentChanged(SharedData *context);
    void OnDataChanged(const QByteArray &role);
private:
    void LoadContext();
    void SaveContext();
private:
    void OnControlSelectedInEditor(const QList<ControlNode *> &node);

    void RefreshActions(const QModelIndexList &indexList);
    void RefreshAction(QAction *action, bool enabled, bool visible);
    void CollectSelectedNodes(DAVA::Vector<ControlNode*> &nodes);
    void CopyNodesToClipboard(const DAVA::Vector<ControlNode*> &nodes);
    void RemoveNodes(const DAVA::Vector<ControlNode*> &nodes);
    QList<QPersistentModelIndex> GetExpandedIndexes() const;
    
private slots:
    void OnRowsInserted(const QModelIndex &parent, int first, int last);
    void OnRowsAboutToBeRemoved(const QModelIndex &parent, int first, int last);
    void OnSelectionChanged(const QItemSelection &proxySelected, const QItemSelection &proxyDeselected);
    void filterTextChanged(const QString &);
    void OnCopy();
    void OnPaste();
    void OnCut();
    void OnDelete();

private:
    SharedData *sharedData;
    QAction *importPackageAction;
    QAction *copyAction;
    QAction *pasteAction;
    QAction *cutAction;
    QAction *delAction;

    QPointer<FilteredPackageModel> filteredPackageModel;
    QPointer<PackageModel> packageModel;
};

#endif // __UI_EDITOR_UI_PACKAGE_WIDGET__
