#include "SearchCriteriaWidget.h"
#include "Logger/Logger.h"
#include "FindFilter.h"

#include <QCheckBox>
#include <QLineEdit>

using namespace DAVA;

class EmptyCriteriaEditor
: public CriteriaEditor
{
public:
    using FindFilterBuilder = Function<std::unique_ptr<FindFilter>()>;

    EmptyCriteriaEditor(QWidget* parent, const FindFilterBuilder& findFilterBuilder_)
        : CriteriaEditor(parent)
        , findFilterBuilder(findFilterBuilder_)
    {
    }

    std::unique_ptr<FindFilter> BuildFindFilter() override
    {
        return findFilterBuilder();
    }

private:
    FindFilterBuilder findFilterBuilder;
};

class StringCriteriaEditor
: public CriteriaEditor
{
public:
    using StringFindFilterBuilder = Function<std::unique_ptr<FindFilter>(const StringCriteriaEditor*)>;

    StringCriteriaEditor(QWidget* parent, const StringFindFilterBuilder& findFilterBuilder_)
        : CriteriaEditor(parent)
        , findFilterBuilder(findFilterBuilder_)
    {
        layout = new QHBoxLayout(this);

        matchesLabel = new QLabel(this);
        matchesLabel->setText(tr("matches"));

        caseSensitive = new QCheckBox();
        caseSensitive->setText(tr("case sensitive"));
        caseSensitive->setChecked(false);

        value = new QLineEdit();

        layout->addWidget(matchesLabel);
        layout->addSpacing(10);
        layout->addWidget(value);
        layout->addStretch();
        layout->addWidget(caseSensitive);

        layout->setMargin(0);
        layout->setSpacing(0);

        QObject::connect(value, SIGNAL(textChanged(const QString&)), this, SIGNAL(CriteriaChanged()));
        QObject::connect(caseSensitive, SIGNAL(stateChanged(int)), this, SIGNAL(CriteriaChanged()));

        setFocusProxy(value);
    }

    String GetString() const
    {
        return value->text().toStdString();
    }

    bool IsCaseSensitive() const
    {
        return caseSensitive->isChecked();
    }

    std::unique_ptr<FindFilter> BuildFindFilter() override
    {
        return findFilterBuilder(this);
    }

private:
    QHBoxLayout* layout = nullptr;
    QLabel* matchesLabel = nullptr;
    QLineEdit* value = nullptr;
    QCheckBox* caseSensitive = nullptr;

    StringFindFilterBuilder findFilterBuilder;
};

class EnumCriteriaEditor
: public CriteriaEditor
{
public:
    using EnumFindFilterBuilder = Function<std::unique_ptr<FindFilter>(const EnumCriteriaEditor*)>;

    EnumCriteriaEditor(QWidget* parent, const EnumMap* editedEnum, const EnumFindFilterBuilder& findFilterBuilder_)
        : CriteriaEditor(parent)
        , findFilterBuilder(findFilterBuilder_)
    {
        layout = new QHBoxLayout(this);

        enumCombobox = new QComboBox();

        for (int32 enumIndex = 0; enumIndex < editedEnum->GetCount(); ++enumIndex)
        {
            enumCombobox->addItem(editedEnum->ToString(enumIndex));
        }

        layout->addWidget(enumCombobox);
        layout->addStretch();

        layout->setMargin(0);
        layout->setSpacing(0);

        QObject::connect(enumCombobox, SIGNAL(currentIndexChanged(int)), this, SIGNAL(CriteriaChanged()));

        setFocusProxy(enumCombobox);
    }

    int32 GetValue() const
    {
        return enumCombobox->currentIndex();
    }

    std::unique_ptr<FindFilter> BuildFindFilter() override
    {
        return findFilterBuilder(this);
    }

private:
    QHBoxLayout* layout = nullptr;
    QComboBox* enumCombobox = nullptr;

    EnumFindFilterBuilder findFilterBuilder;
};

class AbstractSearchCriteria
{
public:
    virtual ~AbstractSearchCriteria() = default;

    virtual const char* GetName() = 0;
    virtual CriteriaEditor* CreateEditor(QWidget* parent) = 0;
};

class NameSearchCriteria
: public AbstractSearchCriteria
{
public:
    const char* GetName() override
    {
        return "Name";
    }

    CriteriaEditor* CreateEditor(QWidget* parent) override
    {
        return new StringCriteriaEditor(parent,
                                        [](const StringCriteriaEditor* editor)
                                        {
                                            return std::make_unique<ControlNameFilter>(editor->GetString(), editor->IsCaseSensitive());
                                        });
    }
};

class HasComponentSearchCriteria
: public AbstractSearchCriteria
{
public:
    const char* GetName() override
    {
        return "Has component";
    }

    CriteriaEditor* CreateEditor(QWidget* parent) override
    {
        return new EnumCriteriaEditor(parent,
                                      GlobalEnumMap<UIComponent::eType>::Instance(),
                                      [](const EnumCriteriaEditor* editor)
                                      {
                                          return std::make_unique<HasComponentFilter>(static_cast<UIComponent::eType>(editor->GetValue()));
                                      });
    }
};

const Vector<std::shared_ptr<AbstractSearchCriteria>> criterias
{ std::make_shared<NameSearchCriteria>(),
  std::make_shared<HasComponentSearchCriteria>() };

SearchCriteriaWidget::SearchCriteriaWidget(QWidget* parent)
    : QWidget(parent)
{
    layout = new QHBoxLayout(this);

    addCriteriaButton = new QToolButton();
    addCriteriaButton->setText("+");

    removeCriteriaButton = new QToolButton();
    removeCriteriaButton->setText("-");

    criteria = new QComboBox();
    criteria->setSizeAdjustPolicy(QComboBox::SizeAdjustPolicy::AdjustToContents);

    for (const std::shared_ptr<AbstractSearchCriteria>& c : criterias)
    {
        criteria->addItem(tr(c->GetName()));
    }

    layout->setSpacing(0);
    layout->setMargin(0);

    innerLayout = new QHBoxLayout();

    innerLayout->setSpacing(0);
    innerLayout->setMargin(0);

    layout->addWidget(addCriteriaButton);
    layout->addWidget(removeCriteriaButton);
    layout->addSpacing(10);
    layout->addWidget(criteria);
    layout->addSpacing(10);
    layout->addLayout(innerLayout);
    layout->addSpacing(10);

    QObject::connect(addCriteriaButton, SIGNAL(clicked()), this, SIGNAL(AddAnotherCriteria()));
    QObject::connect(removeCriteriaButton, SIGNAL(clicked()), this, SIGNAL(RemoveCriteria()));
    QObject::connect(criteria, SIGNAL(currentIndexChanged(int)), this, SLOT(OnCriteriaSelected(int)));

    OnCriteriaSelected(0);
}

SearchCriteriaWidget::~SearchCriteriaWidget()
{
}

std::shared_ptr<FindFilter> SearchCriteriaWidget::BuildFindFilter() const
{
    if (editor)
    {
        return editor->BuildFindFilter();
    }
    else
    {
        return nullptr;
    }
}

void SearchCriteriaWidget::OnCriteriaSelected(int index)
{
    innerLayout->removeWidget(editor);
    delete editor;

    editor = criterias[index]->CreateEditor(nullptr);
    innerLayout->addWidget(editor);
    QObject::connect(editor, SIGNAL(CriteriaChanged()), this, SIGNAL(CriteriaChanged()));

    setFocusProxy(editor);

    emit CriteriaChanged();
}
