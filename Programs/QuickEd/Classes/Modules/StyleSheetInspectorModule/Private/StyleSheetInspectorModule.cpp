#include "Modules/StyleSheetInspectorModule/StyleSheetInspectorModule.h"
#include "Modules/StyleSheetInspectorModule/StyleSheetInspectorWidget.h"
#include "Application/QEGlobal.h"

#include <TArc/WindowSubSystem/UI.h>
#include <TArc/Utils/ModuleCollection.h>

DAVA_VIRTUAL_REFLECTION_IMPL(StyleSheetInspectorModule)
{
    DAVA::ReflectionRegistrator<StyleSheetInspectorModule>::Begin()
    .ConstructorByPointer()
    .End();
}

void StyleSheetInspectorModule::PostInit()
{
    using namespace DAVA::TArc;

    const char* title = "Style Sheet Inspector";
    DockPanelInfo panelInfo;
    panelInfo.title = title;
    panelInfo.area = Qt::RightDockWidgetArea;
    PanelKey panelKey(title, panelInfo);

    ContextAccessor* accessor = GetAccessor();
    StyleSheetInspectorWidget* widget = new StyleSheetInspectorWidget(accessor);
    GetUI()->AddView(DAVA::TArc::mainWindowKey, panelKey, widget);
}

DECL_GUI_MODULE(StyleSheetInspectorModule);
