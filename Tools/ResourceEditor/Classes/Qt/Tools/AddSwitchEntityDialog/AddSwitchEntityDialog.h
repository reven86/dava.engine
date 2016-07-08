#ifndef __RESOURCEEDITORQT__ADDSWITCHENTITYDIALOG__
#define __RESOURCEEDITORQT__ADDSWITCHENTITYDIALOG__

#include "../BaseAddEntityDialog/BaseAddEntityDialog.h"
#include "DAVAEngine.h"
#include "Qt/Scene/SceneEditor2.h"

class SelectEntityPathWidget;

class AddSwitchEntityDialog : public BaseAddEntityDialog
{
    Q_OBJECT

public:
    AddSwitchEntityDialog(QWidget* parent = 0);

    ~AddSwitchEntityDialog();

    void CleanupPathWidgets();

    const DAVA::Vector<SelectEntityPathWidget*>& GetPathWidgets()
    {
        return pathWidgets;
    }

    void GetPathEntities(DAVA::Vector<DAVA::Entity*>& entities, SceneEditor2* editor);

    void accept();

    void reject();

protected:
    virtual void FillPropertyEditorWithContent()
    {
    }

    DAVA::Vector<SelectEntityPathWidget*> pathWidgets;

    DAVA::Vector<QWidget*> additionalWidgets;
};

#endif /* defined(__RESOURCEEDITORQT__ADDSWITCHENTITYDIALOG__) */