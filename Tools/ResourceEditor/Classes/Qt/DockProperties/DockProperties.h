#ifndef __RESOURCEEDITORQT__DOCKPROPERTIES__
#define __RESOURCEEDITORQT__DOCKPROPERTIES__

#include <QDockWidget>
#include <QPointer>
#include <memory>

namespace Ui
{
class MainWindow;
}

class QMenu;
class QToolBar;
class PropertyEditor;
class GlobalOperations;

class DockProperties : public QDockWidget
{
    Q_OBJECT

public:
    DockProperties(QWidget* parent = NULL);
    ~DockProperties();

    void Init(Ui::MainWindow* mainwindowUi, const std::shared_ptr<GlobalOperations>& globalOperations);

protected slots:
    void ActionFavoritesEdit();
    void ViewModeSelected(int index);
    void OnAddAction();

private:
    QPointer<QMenu> addComponentMenu;
    QPointer<QToolBar> toolbar;
    QPointer<PropertyEditor> propertiesEditor;
};

#endif // __RESOURCEEDITORQT__DOCKPROPERTIES__
