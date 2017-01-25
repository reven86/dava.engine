#include "TArc/Controls/SceneTabbar.h"

#include "Debug/DVAssert.h"
#include "Base/BaseTypes.h"

#include <QVariant>
#include <QShortcut>

namespace DAVA
{
namespace TArc
{
SceneTabbar::SceneTabbar(ContextAccessor* accessor, Reflection model_, QWidget* parent /* = nullptr */)
    : QTabBar(parent)
    , model(model_)
{
    DataWrapper::DataAccessor accessorFn(this, &SceneTabbar::GetSceneTabsModel);
    modelWrapper = accessor->CreateWrapper(accessorFn);
    modelWrapper.SetListener(this);

    QObject::connect(this, &QTabBar::currentChanged, this, &SceneTabbar::OnCurrentTabChanged);
    QObject::connect(this, &QTabBar::tabCloseRequested, this, &SceneTabbar::OnCloseTabRequest);
    QObject::connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_W), this), &QShortcut::activated, DAVA::MakeFunction(this, &SceneTabbar::OnCloseCurrentTab));
#if defined(__DAVAENGINE_WIN32__)
    QObject::connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_F4), this), &QShortcut::activated, DAVA::MakeFunction(this, &SceneTabbar::OnCloseCurrentTab));
#endif
}

void SceneTabbar::OnDataChanged(const DataWrapper& wrapper, const Vector<Any>& fields)
{
    DVASSERT(wrapper.HasData());
    bool tabsPropertyChanged = fields.empty();
    bool activeTabPropertyChanged = fields.empty();
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
    Reflection ref = model.GetField(Any(activeTabPropertyName));
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
        QSignalBlocker blocker(this);
        uint64 currentTabID = tabData(currentIndex()).value<uint64>();
        UnorderedMap<uint64, int> existsIds;
        int tabCount = count();
        for (int i = 0; i < tabCount; ++i)
        {
            QVariant data = tabData(i);
            DVASSERT(data.canConvert<uint64>());
            existsIds.emplace(data.value<uint64>(), i);
        }

        Reflection ref = model.GetField(tabsPropertyName);
        Vector<Reflection::Field> fields = ref.GetFields();
        setEnabled(!fields.empty());

        for (const Reflection::Field& field : fields)
        {
            DVASSERT(field.key.CanCast<uint64>());
            uint64 id = field.key.Cast<uint64>();
            Reflection title = field.ref.GetField(tabTitlePropertyName);
            DVASSERT(title.IsValid());
            Any titleValue = title.GetValue();
            DVASSERT(titleValue.CanCast<String>());
            QString titleText = QString::fromStdString(titleValue.Cast<String>());

            QString tooltipText;
            Reflection tooltip = field.ref.GetField(tabTooltipPropertyName);
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
    uint64 newActiveTabID = 0;
    if (currentTab != -1)
    {
        QVariant data = tabData(currentTab);
        DVASSERT(data.canConvert<uint64>());
        newActiveTabID = data.value<uint64>();
    }

    modelWrapper.SetFieldValue(activeTabPropertyName, newActiveTabID);
}

void SceneTabbar::OnCloseTabRequest(int index)
{
    QVariant data = tabData(index);
    DVASSERT(data.canConvert<uint64>());
    uint64 id = data.value<uint64>();
    closeTab.Emit(id);
}

void SceneTabbar::OnCloseCurrentTab()
{
    int currentTab = currentIndex();
    if (currentTab != -1)
    {
        OnCloseTabRequest(currentTab);
    }
}

const char* SceneTabbar::activeTabPropertyName = "ActiveTabID";
const char* SceneTabbar::tabsPropertyName = "Tabs";
const char* SceneTabbar::tabTitlePropertyName = "Title";
const char* SceneTabbar::tabTooltipPropertyName = "Tooltip";

} // namespace TArc
} // namespace DAVA