#pragma once

#include "Commands2/Base/Command2.h"
#include "Functional/Signal.h"
#include "QtTools/WarningGuard/QtWarningsHandler.h"

#include <core_data_model/reflection/property_model_extensions.hpp>
#include <core_dependency_system/depends.hpp>

#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>

struct DAVAProperiesEnum
{
    enum Type
    {
        EntityRoot = wgt::PropertyNode::DomainSpecificProperty,
    };
};

class PropertyPanelGetExtension : public wgt::SetterGetterExtension
{
public:
    wgt::Variant getValue(const wgt::RefPropertyItem* item, int column, size_t roleId, wgt::IDefinitionManager& definitionManager) const override;
};

class EntityChildCreatorExtension : public wgt::ChildCreatorExtension
{
public:
    void exposeChildren(const std::shared_ptr<const wgt::PropertyNode>& node, std::vector<std::shared_ptr<const wgt::PropertyNode>>& children, wgt::IDefinitionManager& defMng) const override;
};

class EntityMergeValueExtension : public wgt::MergeValuesExtension
{
public:
    wgt::RefPropertyItem* lookUpItem(const std::shared_ptr<const wgt::PropertyNode>& node, const std::vector<std::unique_ptr<wgt::RefPropertyItem>>& items,
                                     wgt::IDefinitionManager& definitionManager) const override;
};

class AddCustomPropertyWidget : public QWidget
{
    PUSH_QT_WARNING_SUPRESSOR
    Q_OBJECT;
    POP_QT_WARNING_SUPRESSOR

public:
    AddCustomPropertyWidget(int defaultType = DAVA::VariantType::TYPE_STRING, QWidget* parent = NULL);

    DAVA::Signal<const DAVA::String&, const DAVA::VariantType&> ValueReady;

    void showEvent(QShowEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

protected slots:
    void OkKeyPressed();
    void PreSetSelected(int index);

private:
    QLineEdit* keyWidget;
    QComboBox* valueWidget;
    QComboBox* presetWidget;
    QPushButton* defaultBtn;
};

class EntityInjectDataExtension : public wgt::InjectDataExtension
{
public:
    class Delegate
    {
    public:
        virtual void BeginBatch(const DAVA::String& name, DAVA::uint32 commandCount) = 0;
        virtual void Exec(Command2::Pointer&& command) = 0;
        virtual void EndBatch() = 0;
    };

    EntityInjectDataExtension(Delegate& delegateObj, wgt::IComponentContext& context);

    void inject(wgt::RefPropertyItem* item) override;
    void updateInjection(wgt::RefPropertyItem* item) override;

private:
    void RemoveComponent(const wgt::RefPropertyItem* item);

    void RemoveRenderBatch(const wgt::RefPropertyItem* item);
    void ConvertBatchToShadow(const wgt::RefPropertyItem* item);
    void RebuildTangentSpace(const wgt::RefPropertyItem* item);

    void AddCustomProperty(const wgt::RefPropertyItem* item);
    void OpenMaterials(const wgt::RefPropertyItem* item);

private:
    Delegate& delegateObj;
    wgt::Depends<wgt::IDefinitionManager> defManagerHolder;
};
