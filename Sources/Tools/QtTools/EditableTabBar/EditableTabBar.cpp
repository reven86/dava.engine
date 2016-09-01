#include "EditableTabBar.h"
#include "Debug/DVAssert.h"

#include <QLineEdit>
#include <QEvent>
#include <QMouseEvent>
#include <QAction>

EditableTabBar::EditableTabBar(QWidget* parent)
    : QTabBar(parent)
    , nameEditor(new QLineEdit(this))
{
    nameEditor->setWindowFlags(Qt::Popup);
    nameEditor->installEventFilter(this);

    QMetaObject::Connection connectionResult = QObject::connect(nameEditor, &QLineEdit::editingFinished, this, &EditableTabBar::onNameEditingFinished);
    DVASSERT(connectionResult);

    connectionResult = QObject::connect(this, &QTabBar::tabBarDoubleClicked, this, &EditableTabBar::onTabDoubleClicked);
    DVASSERT(connectionResult);
}

void EditableTabBar::setNameValidator(const QValidator* v)
{
    DVASSERT(nameEditor != nullptr);
    nameEditor->setValidator(v);
}

bool EditableTabBar::isEditable() const
{
    return isTabsEditable;
}

void EditableTabBar::setEditable(bool isEditable_)
{
    isTabsEditable = isEditable_;
}

bool EditableTabBar::eventFilter(QObject* object, QEvent* event)
{
    if (nameEditor->isVisible())
    {
        DVASSERT(isEditable());
        bool hideEditor = false;
        switch (event->type())
        {
        case QEvent::MouseButtonPress:
            hideEditor = !nameEditor->geometry().contains(static_cast<QMouseEvent*>(event)->globalPos());
            break;
        case QEvent::KeyPress:
            hideEditor = static_cast<QKeyEvent*>(event)->key() == Qt::Key_Escape;
            break;
        default:
            break;
        }

        if (hideEditor)
        {
            finishEdit(false);
            return true;
        }
    }
    return QTabBar::eventFilter(object, event);
}

void EditableTabBar::tabInserted(int index)
{
    QTabBar::tabInserted(index);
}

void EditableTabBar::onNameEditingFinished()
{
    finishEdit(true);
}

void EditableTabBar::onTabDoubleClicked(int index)
{
    startEdit(index);
}

void EditableTabBar::startEdit(int tabIndex)
{
    if (isTabsEditable == false)
    {
        return;
    }

    QRect rect = tabRect(tabIndex);
    nameEditor->setFixedSize(rect.size());
    nameEditor->move(mapToGlobal(rect.topLeft()));
    QString text = tabText(tabIndex);
    nameEditor->setText(text);
    nameEditor->setSelection(0, text.size());
    if (!nameEditor->isVisible())
    {
        nameEditor->show();
    }
}

void EditableTabBar::finishEdit(bool commitChanges)
{
    DVASSERT(isEditable());

    nameEditor->hide();
    if (commitChanges)
    {
        QString newName = nameEditor->text();
        int editedTab = currentIndex();
        if (editedTab >= 0)
        {
            setTabText(editedTab, newName);
            emit tabNameChanged(editedTab);
        }
    }
}
