#include "TArc/Controls/SceneTabbar.h"
#include "TArc/Utils/ScopedValueGuard.h"

#include "Debug/DVAssert.h"
#include "Base/BaseTypes.h"

#include <QVariant>

namespace DAVA
{
namespace TArc
{
SceneTabbar::SceneTabbar(ContextAccessor* accessor, Reflection model_, QWidget* parent /* = nullptr */)
    : model(model_)
{
    DataWrapper::DataAccessor accessorFn(this, &SceneTabbar::GetSceneTabsModel);
    modelWrapperWrapper = accessor->CreateWrapper(accessorFn);
    modelWrapperWrapper.SetListener(this);

    QObject::connect(this, &QTabBar::currentChanged, this, &SceneTabbar::OnCurrentTabChanged);
    QObject::connect(this, &QTabBar::tabCloseRequested, this, &SceneTabbar::OnCloseTabRequest);
}

SceneTabbar::~SceneTabbar()
{
    modelWrapperWrapper.SetListener(nullptr);
}

void SceneTabbar::OnDataChanged(const DataWrapper& wrapper, const Vector<Any>& fields)
{
    DVASSERT(wrapper.HasData());
    if (fields.empty())
    {
        OnTabsCollectionChanged();
        OnActiveTabChanged();
    }

    bool tabsPropertyChanged = false;
    bool activeTabPropertyChanged = false;
    for (const Any& fieldName : fields)
    {
        if (fieldName.CanCast<String>())
        {
            String name = fieldName.Cast<String>();
            if (name == tabsPropertyName)
            {
                tabsPropertyChanged = true;
            }

            if (name == activeTabPropertyName)
            {
                activeTabPropertyChanged = true;
            }
        }
    }

    if (tabsPropertyChanged)
    {
        OnTabsCollectionChanged();
    }

    if (activeTabPropertyChanged)
    {
        OnActiveTabChanged();
    }
}

void SceneTabbar::OnActiveTabChanged()
{
    SCOPED_VALUE_GUARD(bool, inTabChanging, true, void());

    Reflection ref = model.GetField(Any(activeTabPropertyName)).ref;
    DVASSERT(ref.IsValid());
    Any value = ref.GetValue();
    DVASSERT(value.CanCast<uint64>());
    uint64 id = value.Cast<uint64>();

    {
        QVariant data = tabData(currentIndex());
        if (data.canConvert<uint64>() && data.value<uint64>() == id)
            return;
    }

    int tabCount = count();
    for (int i = 0; i < tabCount; ++i)
    {
        QVariant data = tabData(i);
        DVASSERT(data.canConvert<uint64>());
        if (data.value<uint64>() == id)
        {
            setCurrentIndex(i);
            return;
        }
    }
}

void SceneTabbar::OnTabsCollectionChanged()
{
    bool activeTabRemoved = false;
    {
        SCOPED_VALUE_GUARD(bool, inTabChanging, true, void());

        uint64 currentTabID = tabData(currentIndex()).value<uint64>();
        UnorderedMap<uint64, int> existsIds;
        int tabCount = count();
        for (int i = 0; i < tabCount; ++i)
        {
            QVariant data = tabData(i);
            DVASSERT(data.canConvert<uint64>());
            existsIds.emplace(data.value<uint64>(), i);
        }

        Reflection ref = model.GetField(tabsPropertyName).ref;
        Vector<Reflection::Field> fields = ref.GetFields();
        setEnabled(!fields.empty());

        for (const Reflection::Field& field : fields)
        {
            DVASSERT(field.key.CanCast<uint64>());
            uint64 id = field.key.Cast<uint64>();
            Reflection title = field.ref.GetField(tabTitlePropertyName).ref;
            DVASSERT(title.IsValid());
            Any titleValue = title.GetValue();
            DVASSERT(titleValue.CanCast<String>());
            QString titleText = QString::fromStdString(titleValue.Cast<String>());

            QString tooltipText;
            Reflection tooltip = field.ref.GetField(tabTooltipPropertyName).ref;
            if (tooltip.IsValid())
            {
                Any tooltipValue = tooltip.GetValue();
                DVASSERT(tooltipValue.CanCast<String>());
                tooltipText = QString::fromStdString(tooltipValue.Cast<String>());
            }

            if (existsIds.count(id) == 0)
            {
                int index = addTab(titleText);
                setTabToolTip(index, tooltipText);
                setTabData(index, QVariant::fromValue<uint64>(id));
            }
            else
            {
                setTabText(existsIds[id], titleText);
                setTabToolTip(existsIds[id], tooltipText);
                existsIds.erase(id);
            }
        }

        activeTabRemoved = existsIds.count(currentTabID) > 0;

        for (const auto& node : existsIds)
        {
            for (int i = 0; i < count(); ++i)
            {
                if (tabData(i).value<uint64>() == node.first)
                {
                    removeTab(i);
                    break;
                }
            }
        }
    }

    if (activeTabRemoved)
    {
        OnCurrentTabChanged(currentIndex());
    }
}

DAVA::Reflection SceneTabbar::GetSceneTabsModel(const DataContext* /*context*/)
{
    return model;
}

void SceneTabbar::OnCurrentTabChanged(int currentTab)
{
    SCOPED_VALUE_GUARD(bool, inTabChanging, true, void());

    if (currentTab == -1)
    {
        model.GetField(activeTabPropertyName).ref.SetValue(uint64(0));
        return;
    }

    QVariant data = tabData(currentTab);
    DVASSERT(data.canConvert<uint64>());
    model.GetField(activeTabPropertyName).ref.SetValue(data.value<uint64>());
}

void SceneTabbar::OnCloseTabRequest(int index)
{
    QVariant data = tabData(index);
    DVASSERT(data.canConvert<uint64>());
    uint64 id = data.value<uint64>();
    closeTab.Emit(id);
}

const char* SceneTabbar::activeTabPropertyName = "ActiveTabID";
const char* SceneTabbar::tabsPropertyName = "Tabs";
const char* SceneTabbar::tabTitlePropertyName = "Title";
const char* SceneTabbar::tabTooltipPropertyName = "Tooltip";

} // namespace TArc
} // namespace DAVA