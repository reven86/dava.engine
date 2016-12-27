#ifndef __RESOURCEEDITORQT__ADDSWITCHENTITYDIALOG__
#define __RESOURCEEDITORQT__ADDSWITCHENTITYDIALOG__

#include "DAVAEngine.h"
#include "Qt/Tools/BaseAddEntityDialog/BaseAddEntityDialog.h"
#include "Qt/Scene/ActiveSceneHolder.h"

class SelectEntityPathWidget;

class AddSwitchEntityDialog : public BaseAddEntityDialog
{
    Q_OBJECT

public:
    AddSwitchEntityDialog(QWidget* parent = 0);
    ~AddSwitchEntityDialog();

    void accept() override;
    void reject() override;

protected:
    void GetPathEntities(DAVA::Vector<DAVA::Entity*>& entities, SceneEditor2* editor);
    void FillPropertyEditorWithContent() override;

private:
    void CleanupPathWidgets();

    ActiveSceneHolder sceneHolder;
    DAVA::Vector<SelectEntityPathWidget*> pathWidgets;

    DAVA::Vector<QWidget*> additionalWidgets;
};

#endif /* defined(__RESOURCEEDITORQT__ADDSWITCHENTITYDIALOG__) */
