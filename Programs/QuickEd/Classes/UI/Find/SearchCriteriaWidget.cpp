#include "SearchCriteriaWidget.h"
#include "Logger/Logger.h"

#include <QTextEdit>

using namespace DAVA;

class StringCriteriaEditor
: public QWidget
{
public:
    StringCriteriaEditor(QWidget* parent)
        : QWidget(parent)
    {
        operationCombobox = new QComboBox(this);

        operationCombobox->addItem("matches");
        operationCombobox->addItem("contains");
        operationCombobox->addItem("begins with");
        operationCombobox->addItem("ends with");
        operationCombobox->addItem("is");
        operationCombobox->addItem("is not");
        operationCombobox->setSizeAdjustPolicy(QComboBox::SizeAdjustPolicy::AdjustToContents);

        value = new QTextEdit(this);
    }

private:
    QComboBox* operationCombobox = nullptr;
    QTextEdit* value = nullptr;
};

class AbstractSearchCriteria
{
public:
    virtual ~AbstractSearchCriteria() = default;

    virtual QString GetName() = 0;
    virtual QWidget* CreateEditor(QWidget* parent) = 0;
};

class NameSearchCriteria
: public AbstractSearchCriteria
{
public:
    QString GetName() override
    {
        return "Name";
    }

    QWidget* CreateEditor(QWidget* parent) override
    {
        return new StringCriteriaEditor(parent);
    }
};

class HasSoundSearchCriteria
: public AbstractSearchCriteria
{
public:
    QString GetName() override
    {
        return "Has sound";
    }

    QWidget* CreateEditor(QWidget* parent) override
    {
        return new QWidget(parent);
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
        criteria->addItem(c->GetName());
    }

    layout->setSpacing(0);
    layout->setMargin(0);

    innerLayout = new QHBoxLayout();

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
}

SearchCriteriaWidget::~SearchCriteriaWidget()
{
    DAVA::Logger::Debug("%s", __FUNCTION__);
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
    editor = new QWidget();
    editor->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);
    criterias[index]->CreateEditor(editor);

    //QLabel* l = new QLabel(editor);
    //l->setText("asdf");
    innerLayout->addWidget(editor);
}
