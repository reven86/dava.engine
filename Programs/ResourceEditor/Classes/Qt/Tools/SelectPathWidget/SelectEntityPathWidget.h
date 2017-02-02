#ifndef __RESOURCEEDITORQT__SELECENTITYTPATHWIDGET__
#define __RESOURCEEDITORQT__SELECENTITYTPATHWIDGET__

#include <QWidget>
#include <QMimeData>
#include <qlineedit.h>
#include <qtoolbutton.h>
#include "SelectPathWidgetBase.h"

#include "DAVAEngine.h"
class SceneEditor2;

class SelectEntityPathWidget : public SelectPathWidgetBase
{
    Q_OBJECT

public:
    explicit SelectEntityPathWidget(QWidget* parent = 0, DAVA::String openDialoDefualtPath = "", DAVA::String relativPath = "");

    ~SelectEntityPathWidget();

    DAVA::Entity* GetOutputEntity(SceneEditor2*);

protected:
    void dragEnterEvent(QDragEnterEvent* event);

    void ConvertQMimeDataFromSceneTree(const QMimeData* mimeData, DAVA::List<DAVA::Entity*>&);

    void ConvertQMimeDataFromFilePath(const QMimeData* mimeData, DAVA::List<DAVA::Entity*>&,
                                      SceneEditor2* sceneEditor = NULL);

    void ConvertFromMimeData(const QMimeData* mimeData, DAVA::List<DAVA::Entity*>& retList, SceneEditor2* sceneEditor);

    void SetEntities(const DAVA::List<DAVA::Entity*>& list, bool perfromRertain);

    DAVA::List<DAVA::Entity*> entitiesToHold;
};

#endif /* defined(__RESOURCEEDITORQT__SELECENTITYTPATHWIDGET__) */