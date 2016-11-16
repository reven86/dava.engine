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
    activeTabWrapper = accessor->CreateWrapper(Bind(&SceneTabbar::GetModelField, this, activeTabPropertyName));
    activeTabWrapper.SetListener(this);

    tabsCollectionWrapper = accessor->CreateWrapper(Bind(&SceneTabbar::GetModelField, this, tabsPropertyName));
    tabsCollectionWrapper.SetListener(this);

    QObject::connect(this, &QTabBar::currentChanged, this, &SceneTabbar::OnCurrentTabChanged);
    QObject::connect(this, &QTabBar::tabCloseRequested, this, &SceneTabbar::OnCloseTabRequest);
}

SceneTabbar::~SceneTabbar()
{
    activeTabWrapper.SetListener(nullptr);
}

void SceneTabbar::OnDataChanged(const DataWrapper& wrapper, const Vector<Any>& fields)
{
    if (wrapper == activeTabWrapper)
    {
        OnActiveTabChanged();
    }
    else
    {
        DVASSERT(wrapper == tabsCollectionWrapper);
        OnTabsCollectionChanged();
    }
}

void SceneTabbar::OnActiveTabChanged()
{
    SCOPED_VALUE_GUARD(bool, inTabChanging, true, void());

    Reflection ref = GetModelField(activeTabPropertyName);
    DVASSERT(ref.IsValid());
    Any value = ref.GetValue();
    DVASSERT(value.CanCast<uint64>());
    uint64 id = value.Cast<uint64>();

    {
        QVariant data = tabData(currentIndex());
        DVASSERT(data.canConvert<uint64>());
        if (data.value<uint64>() == id)
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
            break;
        }
    }
}

void SceneTabbar::OnTabsCollectionChanged()
{
    {
        SCOPED_VALUE_GUARD(bool, inTabChanging, true, void());

        UnorderedMap<uint64, int> existsIds;
        int tabCount = count();
        for (int i = 0; i < tabCount; ++i)
        {
            QVariant data = tabData(i);
            DVASSERT(data.canConvert<uint64>());
            existsIds.emplace(data.value<uint64>(), i);
        }

        Reflection ref = GetModelField(tabsPropertyName);
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

            if (existsIds.count(id) == 0)
            {
                int index = addTab(titleText);
                setTabData(index, QVariant::fromValue<uint64>(id));
            }
            else
            {
                setTabText(existsIds[id], titleText);
                existsIds.erase(id);
            }
        }

        for (const auto& node : existsIds)
        {
            removeTab(node.second);
        }
    }

    if (isEnabled())
    {
        OnCurrentTabChanged(currentIndex());
    }
}

DAVA::Reflection SceneTabbar::GetModelField(const char* fieldName)
{
    return model.GetField(Any(fieldName)).ref;
}

void SceneTabbar::OnCurrentTabChanged(int currentTab)
{
    SCOPED_VALUE_GUARD(bool, inTabChanging, true, void());

    QVariant data = tabData(currentTab);
    DVASSERT(data.canConvert<uint64>());
    GetModelField(activeTabPropertyName).SetValue(data.value<uint64>());
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

} // namespace TArc
} // namespace DAVA