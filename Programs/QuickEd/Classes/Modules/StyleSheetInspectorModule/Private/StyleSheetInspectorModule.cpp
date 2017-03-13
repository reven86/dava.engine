#include "Modules/StyleSheetInspectorModule/StyleSheetInspectorModule.h"
#include "Modules/DocumentsModule/DocumentData.h"
#include "Modules/PackageListenerModule/PackageListenerProxy.h"

#include "Application/QEGlobal.h"

#include "Model/PackageHierarchy/ControlNode.h"

#include "Utils/QtDavaConvertion.h"

#include <TArc/WindowSubSystem/UI.h>
#include <TArc/Utils/ModuleCollection.h>

#include <Functional/Function.h>
#include <UI/Styles/UIStyleSheet.h>
#include <UI/Styles/UIStyleSheetSystem.h>
#include <UI/UIControlSystem.h>
#include <Utils/StringFormat.h>
#include <Base/Any.h>

#include <QListWidget>

DAVA_VIRTUAL_REFLECTION_IMPL(StyleSheetInspectorModule)
{
    DAVA::ReflectionRegistrator<StyleSheetInspectorModule>::Begin()
        .ConstructorByPointer()
        .End();
}

StyleSheetInspectorModule::StyleSheetInspectorModule()
    : updater(DAVA::MakeFunction(this, &StyleSheetInspectorModule::Update), 300)
{
}

StyleSheetInspectorModule::~StyleSheetInspectorModule()
{
    updater.Abort();
}

void StyleSheetInspectorModule::PostInit()
{   
    InitFieldBinder();
    InitUI();
    AddListener();
}

void StyleSheetInspectorModule::InitFieldBinder()
{
    using namespace DAVA;
    using namespace DAVA::TArc;
    ContextAccessor* accessor = GetAccessor();
    selectionFieldBinder.reset(new FieldBinder(accessor));
    FieldDescriptor fieldDescr;
    fieldDescr.type = ReflectedTypeDB::Get<DocumentData>();
    fieldDescr.fieldName = DAVA::FastName(DocumentData::selectionPropertyName);
    selectionFieldBinder->BindField(fieldDescr, MakeFunction(this, &StyleSheetInspectorModule::OnSelectionChanged));
}

void StyleSheetInspectorModule::InitUI()
{
    using namespace DAVA::TArc;

    listWidget = new QListWidget();

    const char* title = "Style Sheet Inspector";
    DockPanelInfo panelInfo;
    panelInfo.title = title;
    panelInfo.area = Qt::RightDockWidgetArea;
    PanelKey panelKey(title, panelInfo);
    GetUI()->AddView(QEGlobal::windowKey, panelKey, listWidget);
}

void StyleSheetInspectorModule::AddListener()
{
    using namespace DAVA::TArc;

    ContextAccessor* accessor = GetAccessor();
    DataContext* globalContext = accessor->GetGlobalContext();
    PackageListenerProxy* proxy = globalContext->GetData<PackageListenerProxy>();
    DVASSERT(proxy != nullptr);
    proxy->AddListener(this);
}

void StyleSheetInspectorModule::ControlPropertyWasChanged(ControlNode* node, AbstractProperty* property)
{
    updater.Update();
}

void StyleSheetInspectorModule::StylePropertyWasChanged(StyleSheetNode* node, AbstractProperty* property)
{
    updater.Update();
}

void StyleSheetInspectorModule::StyleSheetsWereRebuilt()
{
    updater.Update();
}

void StyleSheetInspectorModule::OnSelectionChanged(const DAVA::Any& selectionValue)
{
    SelectedNodes selection = selectionValue.Cast<SelectedNodes>(SelectedNodes());
    for (const PackageBaseNode* node : selection)
    {
        const ControlNode* controlNode = dynamic_cast<const ControlNode*>(node);
        if (nullptr != controlNode && nullptr != controlNode->GetControl())
        {
            currentControl = controlNode->GetControl();
            break;
        }
    }
    updater.Update();
}

void StyleSheetInspectorModule::Update()
{
    using namespace DAVA;

    listWidget->clear();
    if (currentControl == nullptr)
    {
        return;
    }

    UIStyleSheetProcessDebugData debugData;

    UIControlSystem::Instance()->GetStyleSheetSystem()->DebugControl(currentControl.Get(), &debugData);

    QFont boldFont;
    boldFont.setBold(true);

    QFont strikeOutFont;
    strikeOutFont.setStrikeOut(true);

    UIStyleSheetPropertySet samePriorityPropertySet;
    std::tuple<int32, int32> prevStyleSheetPriority{ -1, -1 };

    for (auto styleSheetIter = debugData.styleSheets.rbegin();
        styleSheetIter != debugData.styleSheets.rend();
        ++styleSheetIter)
    {
        const UIStyleSheet* ss = styleSheetIter->GetStyleSheet();

        const std::tuple<int32, int32> styleSheetPriority{ styleSheetIter->GetPriority(), ss->GetScore() };

        if (prevStyleSheetPriority != styleSheetPriority)
        {
            samePriorityPropertySet.reset();
            prevStyleSheetPriority = styleSheetPriority;
        }

        const String& selector = Format("%s (score %i / %i)",
            ss->GetSelectorChain().ToString().c_str(),
            ss->GetScore(),
            styleSheetIter->GetPriority());

        QListWidgetItem* styleSheetItem = new QListWidgetItem(selector.c_str());
        styleSheetItem->setFont(boldFont);
        if (!ss->GetSourceInfo().file.IsEmpty())
        {
            styleSheetItem->setToolTip(ss->GetSourceInfo().file.GetFrameworkPath().c_str());
        }
        listWidget->addItem(styleSheetItem);

        const UIStyleSheetPropertyTable* propertyTable = ss->GetPropertyTable();
        for (const UIStyleSheetProperty& prop : propertyTable->GetProperties())
        {
            const UIStyleSheetPropertyDescriptor& descr =
                UIStyleSheetPropertyDataBase::Instance()->GetStyleSheetPropertyByIndex(prop.propertyIndex);

            String propertyStr = Format("  %s = %s",
                descr.GetFullName().c_str(),
                VariantToQString(prop.value, descr.memberInfo).toUtf8().data());

            QListWidgetItem* styleSheetPropertyItem = new QListWidgetItem(propertyStr.c_str());

            if (debugData.propertySources[prop.propertyIndex] != ss)
            {
                styleSheetPropertyItem->setFont(strikeOutFont);
            }

            if (samePriorityPropertySet[prop.propertyIndex])
            {
                styleSheetPropertyItem->setTextColor(Qt::red);
            }

            listWidget->addItem(styleSheetPropertyItem);
        }

        samePriorityPropertySet |= propertyTable->GetPropertySet();
    }
}

DECL_GUI_MODULE(StyleSheetInspectorModule);
