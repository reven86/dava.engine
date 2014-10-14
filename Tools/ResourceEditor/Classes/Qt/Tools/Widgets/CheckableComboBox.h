#ifndef CHECKABLECOMBOBOX_H
#define CHECKABLECOMBOBOX_H


#include <QComboBox>
#include <QStandardItemModel>
#include <QPointer>
#include <QVariant>


class CheckableComboBox
    : public QComboBox
{
    Q_OBJECT

    signals:
    void done();

public:
    explicit CheckableComboBox(QWidget* parent = NULL);
    ~CheckableComboBox();

    QStringList GetSelectedItems() const;
    QList<QVariant> GetSelectedUserData() const;
    void SelectUserData(const QList<QVariant>& dataList);

private slots:
    void OnRowsInserted(const QModelIndex& parent, int start, int end);
    void UpdateTextHints();

private:
    QModelIndexList GetCheckedIndexes() const;

    bool eventFilter(QObject* obj, QEvent* e);
    void paintEvent(QPaintEvent* event);

    QString textHint;
};


#endif // CHECKABLECOMBOBOX_H
