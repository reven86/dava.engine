#include "ShowMaterialAction.h"

#include "Classes/Qt/Main/mainwindow.h"
#include "Classes/Qt/MaterialEditor/MaterialEditor.h"

ShowMaterialAction::ShowMaterialAction(DAVA::NMaterial* material_)
    : CommandAction(CMDID_SHOW_MATERIAL, "ShowMaterial")
    , material(material_)
{
}

void ShowMaterialAction::Redo()
{
    QtMainWindow::Instance()->OnMaterialEditor();
    MaterialEditor::Instance()->SelectMaterial(material);
}
