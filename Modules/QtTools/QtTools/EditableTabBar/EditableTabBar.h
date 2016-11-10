#ifndef __QTTOOLS_EDITABLETABBAR_H__
#define __QTTOOLS_EDITABLETABBAR_H__

#include <QTabBar>

class QLineEdit;
class QValidator;
class EditableTabBar : public QTabBar
{
    Q_OBJECT
public:
    EditableTabBar(QWidget* parent = nullptr);

    void setNameValidator(const QValidator* v);

    bool isEditable() const;
    void setEditable(bool isEditable);

    Q_SIGNAL void tabNameChanged(int index);

protected:
    bool eventFilter(QObject* object, QEvent* event) override;
    void tabInserted(int index) override;

private:
    Q_SLOT void onNameEditingFinished();
    Q_SLOT void onTabDoubleClicked(int index);

    void startEdit(int tabIndex);
    void finishEdit(bool commitChanges);

private:
    QLineEdit* nameEditor = nullptr;
    bool isTabsEditable = true;
};

#endif // __QTTOOLS_EDITABLETABBAR_H__