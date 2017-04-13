#pragma once

#include "UI/Find/Widgets/FindFilterEditor.h"

#include <Base/BaseTypes.h>
#include <Base/EnumMap.h>
#include <Functional/Functional.h>
#include <QHBoxLayout>
#include <QComboBox>

class EnumFindFilterEditor
: public FindFilterEditor
{
public:
    using EnumFindFilterBuilder = DAVA::Function<std::unique_ptr<FindFilter>(const EnumFindFilterEditor*)>;

    EnumFindFilterEditor(QWidget* parent, const EnumMap* editedEnum, const EnumFindFilterBuilder& findFilterBuilder);

    DAVA::int32 GetValue() const;
    std::unique_ptr<FindFilter> BuildFindFilter() override;

private:
    QHBoxLayout* layout = nullptr;
    QComboBox* enumCombobox = nullptr;

    EnumFindFilterBuilder findFilterBuilder;
};
