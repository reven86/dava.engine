#include "TArc/Controls/SceneTabbar.h"
#include "TArc/Utils/CommonFieldNames.h"

#include <Debug/DVAssert.h>
#include <Base/BaseTypes.h>

#include <QVariant>
#include <QShortcut>

namespace DAVA
{
namespace TArc
{
SceneTabbar::SceneTabbar(ContextAccessor* accessor_, Reflection model_, QWidget* parent /* = nullptr */)
    : QTabBar(parent)
    , accessor(accessor_)
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
    static FastName fastname_contextsFieldName(ContextsFieldName);
    static FastName fastname_activeContextFieldName(ActiveContextFieldName);

    if (wrapper == modelWrapper)
    {
        DVASSERT(wrapper.HasData());
        bool tabsPropertyChanged = fields.empty();
        bool activeTabPropertyChanged = fields.empty();
        for (const Any& fieldName : fields)
        {
            if (fieldName.CanCast<FastName>())
            {
                FastName name = fieldName.Cast<FastName>();
                if (name == fastname_contextsFieldName)
                {
                    tabsPropertyChanged = true;
                }

                if (name == fastname_activeContextFieldName)
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
    else
    {
        uint64 id = GetIDByWrapper(wrapper);
        int tabIndex = GetTabIndexByID(id);
        Reflection ref = GetTabModel(id, nullptr);
        if (ref.IsValid()) // can be invalid if tab was just closed
        {
            Reflection title = ref.GetField(ContextNameFieldName);
            DVASSERT(title.IsValid());
            QString titleText = QString::fromStdString(title.GetValue().Cast<String>());

            Reflection isChangedField = ref.GetField(IsContextModifiedFieldName);
            DVASSERT(isChangedField.IsValid());
            bool isChanged = isChangedField.GetValue().Cast<bool>();

            if (isChanged == true)
            {
                titleText += "*";
            }
            setTabText(tabIndex, titleText);

            QString tooltipText;
            Reflection tooltip = ref.GetField(ContextToolTipFieldName);
            if (tooltip.IsValid())
            {
                Any tooltipValue = tooltip.GetValue();
                DVASSERT(tooltipValue.CanCast<String>());
                tooltipText = QString::fromStdString(tooltipValue.Cast<String>());
                setTabToolTip(tabIndex, tooltipText);
            }
        }
    }
}

void SceneTabbar::OnActiveTabChanged()
{
    Reflection ref = model.GetField(ActiveContextFieldName);
    DVASSERT(ref.IsValid());
    Reflection idField = ref.GetField(ContextIDFieldName);
    if (idField.IsValid()) // can be invalid when active context == nullptr
    {
        setEnabled(true);
        Any value = idField.GetValue();
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
    else
    {
        setEnabled(false);
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

        Reflection ref = model.GetField(ContextsFieldName);
        Vector<Reflection::Field> fields = ref.GetFields();
        setEnabled(!fields.empty());

        for (const Reflection::Field& field : fields)
        {
            Reflection idField = field.ref.GetField(ContextIDFieldName);
            DVASSERT(idField.IsValid());
            uint64 tabID = idField.GetValue().Cast<uint64>();

            if (existsIds.count(tabID) == 0)
            {
                int index = addTab("");
                setTabData(index, tabID);
                DataWrapper::DataAccessor fn(Bind(&SceneTabbar::GetTabModel, this, tabID, _1));
                DataWrapper tabWrapper = accessor->CreateWrapper(fn);
                tabWrapper.SetListener(this);
                tabWrappers.emplace(tabID, tabWrapper);
            }
            else
            {
                existsIds.erase(tabID);
            }
        }

        activeTabRemoved = existsIds.count(currentTabID) > 0;

        for (const auto& node : existsIds)
        {
            for (int i = 0; i < count(); ++i)
            {
                if (tabData(i).value<uint64>() == node.first)
                {
                    auto iter = tabWrappers.find(node.first);
                    DVASSERT(iter != tabWrappers.end());
                    iter->second.SetListener(nullptr);
                    tabWrappers.erase(iter);
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

Reflection SceneTabbar::GetSceneTabsModel(const DataContext* /*context*/)
{
    return model;
}

Reflection SceneTabbar::GetTabModel(uint64 id, const DataContext* /*context*/)
{
    Reflection tabsField = model.GetField(ContextsFieldName);
    Vector<Reflection::Field> fields = tabsField.GetFields();
    for (const Reflection::Field& field : fields)
    {
        if (field.ref.GetField(ContextIDFieldName).GetValue().Cast<uint64>() == id)
        {
            return field.ref;
        }
    }

    return Reflection();
}

void SceneTabbar::OnCurrentTabChanged(int currentTab)
{
    uint64 newActiveTabID = 0;
    if (currentTab != -1)
    {
        QVariant data = tabData(currentTab);
        DVASSERT(data.canConvert<uint64>());
        newActiveTabID = data.value<uint64>();

        Reflection r = GetTabModel(newActiveTabID, nullptr);
        modelWrapper.SetFieldValue(ActiveContextFieldName, r.GetValue());
    }
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

uint64 SceneTabbar::GetIDByWrapper(const DataWrapper& wrapper) const
{
    for (const auto& iter : tabWrappers)
    {
        if (iter.second == wrapper)
        {
            return iter.first;
        }
    }

    DVASSERT(false);
    return static_cast<uint64>(-1);
}

int SceneTabbar::GetTabIndexByID(uint64 id) const
{
    for (int i = 0; i < count(); ++i)
    {
        if (tabData(i).value<uint64>() == id)
        {
            return i;
        }
    }

    DVASSERT(false);
    return -1;
}

} // namespace TArc
} // namespace DAVA