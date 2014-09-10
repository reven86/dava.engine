#ifndef __RUNACTIONWIDGET_H__
#define __RUNACTIONWIDGET_H__


#include <QWidget>
#include <QScopedPointer>
#include <QVariant>
#include <QMap>
#include <QPointer>
#include <QStringListModel>


namespace Ui {
    class RunActionEventWidget;
}


class SceneEditor2;
class EntityGroup;

class RunActionEventWidget
    : public QWidget
{
    Q_OBJECT

public:
    explicit RunActionEventWidget(QWidget *parent = NULL);
    ~RunActionEventWidget();

private:
    QScopedPointer<Ui::RunActionEventWidget> ui;
    QMap< int, int > editorIdMap;
    QPointer<QStringListModel> autocompleteModel;
    SceneEditor2 *scene;

private slots:
    void OnTypeChanged();
    void OnInvoke();
    void sceneActivated(SceneEditor2 *scene);
    void sceneSelectionChanged(SceneEditor2 *scene, const EntityGroup *selected, const EntityGroup *deselected);
};



#endif // __RUNACTIONWIDGET_H__
