#include "TArc/Controls/PropertyPanel/BaseComponentValue.h"
#include "TArc/Controls/PropertyPanel/Private/ReflectedPropertyModel.h"
#include "TArc/Controls/PropertyPanel/PropertyPanelMeta.h"
#include "TArc/Controls/QtBoxLayouts.h"

#include "TArc/DataProcessing/DataWrappersProcessor.h"
#include "TArc/Utils/ScopedValueGuard.h"
#include "TArc/Utils/ReflectionHelpers.h"
#include "TArc/Qt/QtSize.h"

#include <Engine/PlatformApi.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Logger/Logger.h>

#include <QApplication>
#include <QtEvents>
#include <QStyle>
#include <QStyleOption>
#include <QPainter>
#include <QPainter>
#include <QDebug>

namespace DAVA
{
namespace TArc
{
class BaseComponentValue::ButtonModel
{
public:
    ButtonModel(BaseComponentValue* valueComponent, std::shared_ptr<M::CommandProducer> cmd_)
        : value(valueComponent)
        , cmd(cmd_)
    {
        commandInfo = cmd->GetInfo();
    }

private:
    bool IsEnabled() const
    {
        return cmd->OnlyForSingleSelection() == false || value->nodes.size() == 1;
    }

    bool IsVisible() const
    {
        for (const std::shared_ptr<PropertyNode>& node : value->nodes)
        {
            if (cmd->IsApplyable(node))
            {
                return true;
            }
        }

        return false;
    }

    const QIcon& GetIcon() const
    {
        return commandInfo.icon;
    }

    QSize GetIconSize() const
    {
        return BaseComponentValue::toolButtonIconSize;
    }

    const QString& GetToolTip() const
    {
        return commandInfo.tooltip;
    }

    bool IsAutoRise() const
    {
        return false;
    }

    void Invoke()
    {
        value->CallButtonAction(cmd);
    }

private:
    BaseComponentValue* value;
    std::shared_ptr<M::CommandProducer> cmd;
    M::CommandProducer::Info commandInfo;

    DAVA_REFLECTION(ButtonModel);
};

DAVA_REFLECTION_IMPL(BaseComponentValue::ButtonModel)
{
    ReflectionRegistrator<ButtonModel>::Begin()
    .Field("isEnabled", &ButtonModel::IsEnabled, nullptr)
    .Field("isVisible", &ButtonModel::IsVisible, nullptr)
    .Field("icon", &ButtonModel::GetIcon, nullptr)
    .Field("iconSize", &ButtonModel::GetIconSize, nullptr)
    .Field("toolTip", &ButtonModel::GetToolTip, nullptr)
    .Field("autoRise", &ButtonModel::IsAutoRise, nullptr)
    .Method("invoke", &ButtonModel::Invoke)
    .End();
}

BaseComponentValue::BaseComponentValue()
{
    thisValue = this;
}

BaseComponentValue::~BaseComponentValue()
{
    if (editorWidget != nullptr)
    {
        if (buttonsLayout != nullptr)
        {
            buttonsLayout->TearDown();
        }
        editorWidget->TearDown();
        editorWidget = nullptr;

        realWidget->deleteLater();
        realWidget = nullptr;
    }
    else
    {
        DVASSERT(realWidget == nullptr);
    }
}

void BaseComponentValue::Init(ReflectedPropertyModel* model_)
{
    model = model_;
}

void BaseComponentValue::Draw(QPainter* painter, const QStyleOptionViewItem& opt)
{
    UpdateEditorGeometry(opt.rect);
    bool isOpacue = realWidget->testAttribute(Qt::WA_NoSystemBackground);
    realWidget->setAttribute(Qt::WA_NoSystemBackground, true);
    QPixmap pxmap = realWidget->grab();
    realWidget->setAttribute(Qt::WA_NoSystemBackground, isOpacue);
    painter->drawPixmap(opt.rect, pxmap);
}

void BaseComponentValue::UpdateGeometry(const QStyleOptionViewItem& opt)
{
    UpdateEditorGeometry(opt.rect);
}

bool BaseComponentValue::HasHeightForWidth() const
{
    DVASSERT(realWidget != nullptr);
    return realWidget->hasHeightForWidth();
}

int BaseComponentValue::GetHeightForWidth(int width) const
{
    DVASSERT(realWidget != nullptr);
    return realWidget->heightForWidth(width);
}

int BaseComponentValue::GetHeight() const
{
    DVASSERT(realWidget != nullptr);
    return realWidget->sizeHint().height();
}

QWidget* BaseComponentValue::AcquireEditorWidget(const QStyleOptionViewItem& option)
{
    UpdateEditorGeometry(option.rect);
    return realWidget;
}

QString BaseComponentValue::GetPropertyName() const
{
    std::shared_ptr<PropertyNode> node = nodes.front();
    const Reflection& r = node->field.ref;

    if (node->propertyType != PropertyNode::GroupProperty)
    {
        const M::DisplayName* displayName = r.GetMeta<M::DisplayName>();
        if (displayName != nullptr)
        {
            return QString::fromStdString(displayName->displayName);
        }
    }

    return node->field.key.Cast<QString>();
}

FastName BaseComponentValue::GetID() const
{
    return itemID;
}

int32 BaseComponentValue::GetPropertiesNodeCount() const
{
    return static_cast<int32>(nodes.size());
}

std::shared_ptr<PropertyNode> BaseComponentValue::GetPropertyNode(int32 index) const
{
    DVASSERT(static_cast<size_t>(index) < nodes.size());
    return nodes[static_cast<size_t>(index)];
}

void BaseComponentValue::ForceUpdate()
{
    if (editorWidget != nullptr)
    {
        editorWidget->ForceUpdate();
    }
}

bool BaseComponentValue::IsReadOnly() const
{
    Reflection r = nodes.front()->field.ref;
    return r.IsReadonly() || (nullptr != r.GetMeta<M::ReadOnly>());
}

bool BaseComponentValue::IsSpannedControl() const
{
    return false;
}

bool BaseComponentValue::RepaintOnUpdateRequire() const
{
    return false;
}

bool BaseComponentValue::IsVisible() const
{
    return realWidget != nullptr && realWidget->isVisible();
}

const BaseComponentValue::Style& BaseComponentValue::GetStyle() const
{
    return style;
}

void BaseComponentValue::SetStyle(const Style& style_)
{
    style = style_;
}

DAVA::Any BaseComponentValue::GetValue() const
{
    Any value = nodes.front()->cachedValue;
    for (const std::shared_ptr<const PropertyNode>& node : nodes)
    {
        if (value != node->cachedValue)
        {
            return GetMultipleValue();
        }
    }

    return value;
}

void BaseComponentValue::SetValue(const Any& value)
{
    Any currentValue = nodes.front()->field.ref.GetValue();
    for (const std::shared_ptr<const PropertyNode>& node : nodes)
    {
        if (currentValue != node->field.ref.GetValue())
        {
            currentValue = GetMultipleValue();
            break;
        }
    }

    if (IsValidValueToSet(value, currentValue))
    {
        GetModifyInterface()->ModifyPropertyValue(nodes, value);
    }
}

std::shared_ptr<ModifyExtension> BaseComponentValue::GetModifyInterface()
{
    return model->GetExtensionChain<ModifyExtension>();
}

void BaseComponentValue::AddPropertyNode(const std::shared_ptr<PropertyNode>& node, const FastName& id)
{
    FastName resolvedId = id;
    if (resolvedId.IsValid() == false)
    {
        resolvedId = FastName(node->BuildID());
    }
    if (nodes.empty() == true)
    {
        itemID = resolvedId;
    }
#if defined(__DAVAENGINE_DEBUG__)
    else
    {
        DVASSERT(itemID == resolvedId);
        DVASSERT(nodes.front()->cachedValue.GetType() == node->cachedValue.GetType());
    }
#endif

    nodes.push_back(node);
}

void BaseComponentValue::RemovePropertyNode(const std::shared_ptr<PropertyNode>& node)
{
    auto iter = std::find(nodes.begin(), nodes.end(), node);
    if (iter == nodes.end())
    {
        DVASSERT(false);
        return;
    }

    nodes.erase(iter);
}

void BaseComponentValue::RemovePropertyNodes()
{
    nodes.clear();
}

ContextAccessor* BaseComponentValue::GetAccessor() const
{
    return model->accessor;
}

UI* BaseComponentValue::GetUI() const
{
    return model->ui;
}

const WindowKey& BaseComponentValue::GetWindowKey() const
{
    return model->wndKey;
}

DAVA::TArc::DataWrappersProcessor* BaseComponentValue::GetDataProcessor() const
{
    return model->GetWrappersProcessor(nodes.front());
}

void BaseComponentValue::EnsureEditorCreated(QWidget* parent)
{
    if (editorWidget != nullptr)
    {
        return;
    }

    DataWrappersProcessor* processor = GetDataProcessor();
    processor->SetDebugName(GetPropertyName().toStdString());
    editorWidget = CreateEditorWidget(parent, Reflection::Create(&thisValue), processor);
    processor->SetDebugName("");
    editorWidget->ForceUpdate();
    realWidget = editorWidget->ToWidgetCast();

    const M::CommandProducerHolder* typeProducer = GetTypeMeta<M::CommandProducerHolder>(nodes.front()->cachedValue);
    const M::CommandProducerHolder* fieldProducer = nodes.front()->field.ref.GetMeta<M::CommandProducerHolder>();
    if (typeProducer == fieldProducer)
    {
        typeProducer = nullptr;
    }

    bool realProperty = nodes.front()->propertyType == PropertyNode::RealProperty;
    if (realProperty == true && (fieldProducer != nullptr || typeProducer != nullptr))
    {
        QtHBoxLayout* layout = new QtHBoxLayout();
        buttonsLayout = new Widget(parent);
        buttonsLayout->SetLayout(layout);
        layout->setMargin(0);
        layout->setSpacing(1);

        CreateButtons(buttonsLayout, typeProducer);
        CreateButtons(buttonsLayout, fieldProducer);
        layout->addWidget(realWidget);
        QWidget* buttonsLayoutWidget = buttonsLayout->ToWidgetCast();
        buttonsLayoutWidget->setFocusProxy(realWidget);
        buttonsLayoutWidget->setFocusPolicy(realWidget->focusPolicy());
        realWidget = buttonsLayoutWidget;
    }
}

void BaseComponentValue::UpdateEditorGeometry(const QRect& geometry) const
{
    DVASSERT(realWidget != nullptr);
    if (realWidget->geometry() != geometry)
    {
        realWidget->setGeometry(geometry);
        QLayout* layout = realWidget->layout();
        if (layout != nullptr)
        {
            // force to layout items even if widget isn't visible
            layout->activate();
        }
    }
}

void BaseComponentValue::CreateButtons(Widget* widget, const M::CommandProducerHolder* holder)
{
    if (holder == nullptr)
    {
        return;
    }

    const Vector<std::shared_ptr<M::CommandProducer>>& commands = holder->GetCommandProducers();
    for (size_t i = 0; i < commands.size(); ++i)
    {
        std::unique_ptr<ButtonModel> modelObject = std::make_unique<ButtonModel>(this, commands[i]);
        Reflection model = Reflection::Create(ReflectedObject(modelObject.get()));
        buttonModels.push_back(std::move(modelObject));

        ReflectedButton::Params p(GetAccessor(), GetUI(), GetWindowKey());
        p.fields[ReflectedButton::Fields::Enabled] = "isEnabled";
        p.fields[ReflectedButton::Fields::Visible] = "isVisible";
        p.fields[ReflectedButton::Fields::Icon] = "icon";
        p.fields[ReflectedButton::Fields::IconSize] = "iconSize";
        p.fields[ReflectedButton::Fields::Tooltip] = "toolTip";
        p.fields[ReflectedButton::Fields::AutoRaise] = "autoRise"; // false
        p.fields[ReflectedButton::Fields::Clicked] = "invoke";
        ReflectedButton* button = new ReflectedButton(p, GetDataProcessor(), model, widget->ToWidgetCast());
        button->ForceUpdate();
        button->ToWidgetCast()->setFocusPolicy(Qt::StrongFocus);
        widget->AddControl(button);
    }
}

void BaseComponentValue::CallButtonAction(std::shared_ptr<M::CommandProducer> producer)
{
    M::CommandProducer::Info info = producer->GetInfo();
    producer->CreateCache(model->accessor);

    ModifyExtension::MultiCommandInterface cmdInterface = GetModifyInterface()->GetMultiCommandInterface(info.description, static_cast<uint32>(nodes.size()));
    for (std::shared_ptr<PropertyNode>& node : nodes)
    {
        if (producer->IsApplyable(node))
        {
            M::CommandProducer::Params params;
            params.accessor = model->accessor;
            params.invoker = model->invoker;
            params.ui = model->ui;
            cmdInterface.Exec(producer->CreateCommand(node, params));
        }
    }
    producer->ClearCache();
}

QSize BaseComponentValue::toolButtonIconSize = QSize(12, 12);

const char* BaseComponentValue::readOnlyFieldName = "isReadOnly";

DAVA_VIRTUAL_REFLECTION_IMPL(BaseComponentValue)
{
    ReflectionRegistrator<BaseComponentValue>::Begin()
    .Field(readOnlyFieldName, &BaseComponentValue::IsReadOnly, nullptr)
    .End();
}
}
}
