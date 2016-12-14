#include "UI/StyleSheetInspector/StyleSheetInspector.h"
#include "UI/Styles/UIStyleSheet.h"
#include "UI/Styles/UIStyleSheetSystem.h"
#include "Utils/QtDavaConvertion.h"

using namespace DAVA;

StyleSheetInspector::StyleSheetInspector(QWidget* parent /* = 0*/)
    : QDockWidget(parent)
{
    setupUi(this);
}

void StyleSheetInspector::OnDocumentChanged(Document* context)
{
    if (packageNode)
    {
        packageNode->RemoveListener(this);
    }

    packageNode = context ? context->GetPackage() : nullptr;

    if (packageNode)
    {
        packageNode->AddListener(this);
    }

    currentControl = nullptr;
    Update();
}

void StyleSheetInspector::OnSelectionChanged(const SelectedNodes& selected, const SelectedNodes& deselected)
{
    for (auto node : selected)
    {
        ControlNode* controlNode = dynamic_cast<ControlNode*>(node);
        if (controlNode != nullptr)
        {
            if (nullptr != controlNode && nullptr != controlNode->GetControl())
            {
                currentControl = controlNode->GetControl();

                break;
            }
        }
    }

    Update();
}

void StyleSheetInspector::StyleSheetsWereRebuilt()
{
    Update();
}

void StyleSheetInspector::Update()
{
    listWidget->clear();

    if (currentControl)
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
            styleSheetItem->setToolTip(ss->GetSourceInfo().file.GetStringValue().c_str());
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
}
