#pragma once

#include <TArc/Controls/PropertyPanel/BaseComponentValue.h>
#include <TArc/Utils/QtConnections.h>

#include <FileSystem/VariantType.h>
#include <Reflection/Reflection.h>
#include <Base/Any.h>
#include <Base/BaseTypes.h>

namespace PropertyPanel
{
class AddKeyedArchiveItemWidget;
class KeyedArchiveEditor : public DAVA::TArc::BaseComponentValue
{
public:
    ~KeyedArchiveEditor() override;

protected:
    DAVA::Any GetMultipleValue() const override;
    bool IsValidValueToSet(const DAVA::Any& newValue, const DAVA::Any& currentValue) const override;

    DAVA::TArc::ControlProxy* CreateEditorWidget(QWidget* parent, const DAVA::Reflection& model, DAVA::TArc::DataWrappersProcessor* wrappersProcessor) override;

    void OnButtonClicked();
    void AddProperty(const DAVA::String& key, const DAVA::VariantType& value);

private:
    DAVA::TArc::QtConnections connections;
    static int lastAddedType;
    QPointer<AddKeyedArchiveItemWidget> widget;
};

class KeyedArchiveComboPresetEditor : public DAVA::TArc::BaseComponentValue
{
public:
    KeyedArchiveComboPresetEditor(const DAVA::Vector<DAVA::Any>& values);

protected:
    DAVA::Any GetMultipleValue() const override;
    bool IsValidValueToSet(const DAVA::Any& newValue, const DAVA::Any& currentValue) const override;
    DAVA::TArc::ControlProxy* CreateEditorWidget(QWidget* parent, const DAVA::Reflection& model, DAVA::TArc::DataWrappersProcessor* wrappersProcessor) override;

private:
    DAVA::Any GetValueAny() const;
    void SetValueAny(const DAVA::Any& newValue);

    DAVA::Vector<DAVA::Any> allowedValues;

    DAVA_VIRTUAL_REFLECTION(KeyedArchiveComboPresetEditor, DAVA::TArc::BaseComponentValue);
};

} // namespace PropertyPanel
