#include "Modules/StyleSheetInspectorModule/StyleSheetInspectorWidget.h"
#include "Modules/DocumentsModule/DocumentData.h"

#include "Model/PackageHierarchy/ControlNode.h"

#include "Utils/QtDavaConvertion.h"
#include "Utils/PackageListenerProxy.h"

#include <TArc/Core/ContextAccessor.h>

#include <UI/UIControl.h>
#include <UI/Styles/UIStyleSheet.h>
#include <UI/Styles/UIStyleSheetSystem.h>
#include <UI/UIControlSystem.h>

#include <Functional/Function.h>
#include <Utils/StringFormat.h>
#include <Base/Any.h>

StyleSheetInspectorWidget::StyleSheetInspectorWidget(DAVA::TArc::ContextAccessor* accessor_)
    : QListWidget(nullptr)
    , accessor(accessor_)
    , updater(300)
    , packageListenerProxy(this, accessor)
{
    updater.SetUpdater(DAVA::MakeFunction(this, &StyleSheetInspectorWidget::Update));
    updater.SetStopper([this]() {
        return currentControl.Valid() == false;
    });

    InitFieldBinder();
}

StyleSheetInspectorWidget::~StyleSheetInspectorWidget() = default;

void StyleSheetInspectorWidget::InitFieldBinder()
{
    using namespace DAVA;
    using namespace DAVA::TArc;

    selectionFieldBinder.reset(new FieldBinder(accessor));
    FieldDescriptor fieldDescr;
    fieldDescr.type = ReflectedTypeDB::Get<DocumentData>();
    fieldDescr.fieldName = FastName(DocumentData::selectionPropertyName);
    selectionFieldBinder->BindField(fieldDescr, MakeFunction(this, &StyleSheetInspectorWidget::OnSelectionChanged));
}

void StyleSheetInspectorWidget::ControlPropertyWasChanged(ControlNode* /*node*/, AbstractProperty* /*property*/)
{
    updater.Update();
}

void StyleSheetInspectorWidget::StylePropertyWasChanged(StyleSheetNode* /*node*/, AbstractProperty* /*property*/)
{
    updater.Update();
}

void StyleSheetInspectorWidget::StyleSheetsWereRebuilt()
{
    updater.Update();
}

void StyleSheetInspectorWidget::OnSelectionChanged(const DAVA::Any& selectionValue)
{
    currentControl = nullptr;
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

void StyleSheetInspectorWidget::Update()
{
    using namespace DAVA;

    clear();
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
        addItem(styleSheetItem);

        const UIStyleSheetPropertyTable* propertyTable = ss->GetPropertyTable();
        for (const UIStyleSheetProperty& prop : propertyTable->GetProperties())
        {
            const UIStyleSheetPropertyDescriptor& descr =
            UIStyleSheetPropertyDataBase::Instance()->GetStyleSheetPropertyByIndex(prop.propertyIndex);

            String propertyStr = Format("  %s = %s",
                                        descr.GetFullName().c_str(),
                                        AnyToQString(prop.value, descr.field).toUtf8().data());

            QListWidgetItem* styleSheetPropertyItem = new QListWidgetItem(propertyStr.c_str());

            if (debugData.propertySources[prop.propertyIndex] != ss)
            {
                styleSheetPropertyItem->setFont(strikeOutFont);
            }

            if (samePriorityPropertySet[prop.propertyIndex])
            {
                styleSheetPropertyItem->setTextColor(Qt::red);
            }

            addItem(styleSheetPropertyItem);
        }

        samePriorityPropertySet |= propertyTable->GetPropertySet();
    }
}
