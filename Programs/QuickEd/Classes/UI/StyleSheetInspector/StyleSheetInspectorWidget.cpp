#include "UI/StyleSheetInspector/StyleSheetInspectorWidget.h"
#include "Modules/DocumentsModule/Document.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "UI/Styles/UIStyleSheet.h"
#include "UI/Styles/UIStyleSheetSystem.h"
#include "UI/UIControlSystem.h"
#include "Utils/QtDavaConvertion.h"
#include "Utils/StringFormat.h"

#include "ui_StyleSheetInspectorWidget.h"

using namespace DAVA;

StyleSheetInspectorWidget::StyleSheetInspectorWidget(QWidget* parent /* = nullptr*/)
    : QDockWidget(parent)
    , ui(new Ui::StyleSheetInspectorWidget())
{
    ui->setupUi(this);
}

StyleSheetInspectorWidget::~StyleSheetInspectorWidget() = default;

void StyleSheetInspectorWidget::OnDocumentChanged(Document* context)
{
    if (packageNode != nullptr)
    {
        packageNode->RemoveListener(this);
    }

    packageNode = context ? context->GetPackage() : nullptr;

    if (packageNode != nullptr)
    {
        packageNode->AddListener(this);
    }

    currentControl = nullptr;
    Update();
}

void StyleSheetInspectorWidget::OnSelectionChanged(const SelectedNodes& selected, const SelectedNodes& deselected)
{
    for (const PackageBaseNode* node : selected)
    {
        const ControlNode* controlNode = dynamic_cast<const ControlNode*>(node);
        if (nullptr != controlNode && nullptr != controlNode->GetControl())
        {
            currentControl = controlNode->GetControl();

            break;
        }
    }

    Update();
}

void StyleSheetInspectorWidget::ControlPropertyWasChanged(ControlNode* node, AbstractProperty* property)
{
    Update();
}

void StyleSheetInspectorWidget::StylePropertyWasChanged(StyleSheetNode* node, AbstractProperty* property)
{
    Update();
}

void StyleSheetInspectorWidget::StyleSheetsWereRebuilt()
{
    Update();
}

void StyleSheetInspectorWidget::Update()
{
    ui->listWidget->clear();

    if (currentControl != nullptr)
    {
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
            ui->listWidget->addItem(styleSheetItem);

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

                ui->listWidget->addItem(styleSheetPropertyItem);
            }

            samePriorityPropertySet |= propertyTable->GetPropertySet();
        }
    }
}
