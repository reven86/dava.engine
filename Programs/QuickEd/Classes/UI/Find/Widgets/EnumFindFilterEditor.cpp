#include "UI/Find/Widgets/EnumFindFilterEditor.h"
#include "UI/Find/Filters/FindFilter.h"

using namespace DAVA;

EnumFindFilterEditor::EnumFindFilterEditor(QWidget* parent, const EnumMap* editedEnum, const EnumFindFilterBuilder& findFilterBuilder_)
    : FindFilterEditor(parent)
    , findFilterBuilder(findFilterBuilder_)
{
    layout = new QHBoxLayout(this);

    enumCombobox = new QComboBox(this);

    for (int32 enumIndex = 0; enumIndex < editedEnum->GetCount(); ++enumIndex)
    {
        enumCombobox->addItem(editedEnum->ToString(enumIndex));
    }

    layout->addWidget(enumCombobox);
    layout->addStretch();

    layout->setMargin(0);
    layout->setSpacing(0);

    QObject::connect(enumCombobox, SIGNAL(currentIndexChanged(int)), this, SIGNAL(FilterChanged()));

    setFocusProxy(enumCombobox);
}

int32 EnumFindFilterEditor::GetValue() const
{
    return enumCombobox->currentIndex();
}

std::unique_ptr<FindFilter> EnumFindFilterEditor::BuildFindFilter()
{
    return findFilterBuilder(this);
}
