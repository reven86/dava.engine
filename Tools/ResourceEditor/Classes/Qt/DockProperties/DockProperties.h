#ifndef __RESOURCEEDITORQT__DOCKPROPERTIES__
#define __RESOURCEEDITORQT__DOCKPROPERTIES__

#include <QDockWidget>
#include <QPointer>
#include <QMenu>

class DockProperties : public QDockWidget
{
    Q_OBJECT

public:
    DockProperties(QWidget* parent = NULL);
    ~DockProperties();

    void Init();

protected slots:
    void ActionFavoritesEdit();
    void ViewModeSelected(int index);
    void OnAddAction();

private:
    QPointer<QMenu> addComponentMenu;
};

#endif // __RESOURCEEDITORQT__DOCKPROPERTIES__
