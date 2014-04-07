#ifndef __PROPERTY_FLAG_SELECTOR_COMBO_H__
#define __PROPERTY_FLAG_SELECTOR_COMBO_H__


#include <QObject>
#include <QComboBox>
#include <QPointer>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QListView>


class EnumMap;
class QStandardItem;

class FlagSelectorCombo
    : public QComboBox
{
    Q_OBJECT

private:
    enum ItemRoles
    {
        ValueRole = Qt::UserRole + 1,
    };

public:
    FlagSelectorCombo(QWidget *parent = NULL);
    ~FlagSelectorCombo();

    void AddFlagItem(const int value, const QString& hint);
    void SetFlags(const int flags);
    int GetFlags() const;

private slots:
    void onItemChanged(QStandardItem *item);
    void updateText();

private:
    bool eventFilter(QObject *obj, QEvent *e);
    void paintEvent(QPaintEvent *event);

    QString text;
};


#endif // __PROPERTY_FLAG_SELECTOR_COMBO_H__
