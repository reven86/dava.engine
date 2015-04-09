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
    explicit CheckableComboBox(QWidget* parent = nullptr);
    ~CheckableComboBox();

    QStringList selectedItems() const;
    QList<QVariant> selectedUserData() const;
    void selectUserData(const QList<QVariant>& dataList);

    QModelIndexList checkedIndexes() const;

private slots:
    void onRowsInserted(const QModelIndex& parent, int start, int end);
    void updateTextHints();

private:
    bool eventFilter(QObject* obj, QEvent* e) override;
    void paintEvent(QPaintEvent* event) override;

    QString textHint;
};


#endif // CHECKABLECOMBOBOX_H
