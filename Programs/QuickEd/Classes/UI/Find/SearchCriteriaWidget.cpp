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

        operationCombobox = new QComboBox();

        operationCombobox->addItem(tr("matches"));
        operationCombobox->addItem(tr("contains"));
        operationCombobox->addItem(tr("begins with"));
        operationCombobox->addItem(tr("ends with"));
        operationCombobox->addItem(tr("is"));
        operationCombobox->addItem(tr("is not"));
        operationCombobox->setSizeAdjustPolicy(QComboBox::SizeAdjustPolicy::AdjustToContents);

        value = new QLineEdit();

        layout->addWidget(operationCombobox);
        layout->addSpacing(10);
        layout->addWidget(value);

        layout->setMargin(0);
        layout->setSpacing(0);

        setFocusProxy(value);
    }

    String GetString() const
    {
        return value->text().toStdString();
    }

    std::unique_ptr<FindFilter> BuildFindFilter() override
    {
        return findFilterBuilder(this);
    }

private:
    QHBoxLayout* layout = nullptr;
    QComboBox* operationCombobox = nullptr;
    QLineEdit* value = nullptr;

    StringFindFilterBuilder findFilterBuilder;
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
                                            return std::make_unique<ControlNameFilter>(FastName(editor->GetString()));
                                        });
    }
};

class HasSoundSearchCriteria
: public AbstractSearchCriteria
{
public:
    const char* GetName() override
    {
        return "Has sound";
    }

    CriteriaEditor* CreateEditor(QWidget* parent) override
    {
        return new EmptyCriteriaEditor(parent,
                                       []()
                                       {
                                           return std::make_unique<ControlNameFilter>(FastName("sound"));
                                       });
    }
};

const Vector<std::shared_ptr<AbstractSearchCriteria>> criterias
{ std::make_shared<NameSearchCriteria>(),
  std::make_shared<HasSoundSearchCriteria>() };

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

    layout->addStretch();

    QObject::connect(addCriteriaButton, SIGNAL(clicked()), this, SLOT(OnAddAnotherCriteriaPressed()));
    QObject::connect(removeCriteriaButton, SIGNAL(clicked()), this, SLOT(OnRemoveCriteriaPressed()));
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

void SearchCriteriaWidget::OnAddAnotherCriteriaPressed()
{
    emit AddAnotherCriteria();
}

void SearchCriteriaWidget::OnRemoveCriteriaPressed()
{
    emit RemoveCriteria();
}

void SearchCriteriaWidget::OnCriteriaSelected(int index)
{
    innerLayout->removeWidget(editor);
    delete editor;

    editor = criterias[index]->CreateEditor(nullptr);
    innerLayout->addWidget(editor);

    setFocusProxy(editor);
}
