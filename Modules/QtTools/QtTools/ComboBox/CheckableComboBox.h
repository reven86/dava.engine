#ifndef CHECKABLECOMBOBOX_H
#define CHECKABLECOMBOBOX_H

#include "QtTools/WarningGuard/QtWarningsHandler.h"
PUSH_QT_WARNING_SUPRESSOR
#include <QComboBox>
#include <QStandardItemModel>
POP_QT_WARNING_SUPRESSOR

class ComboBoxModel : public QStandardItemModel
{
    PUSH_QT_WARNING_SUPRESSOR
    Q_OBJECT
    POP_QT_WARNING_SUPRESSOR

public:
    ComboBoxModel(QObject* parent = nullptr);
    Qt::ItemFlags flags(const QModelIndex& index) const override;
};

class CheckableComboBox
: public QComboBox
{
    PUSH_QT_WARNING_SUPRESSOR
    Q_OBJECT
    POP_QT_WARNING_SUPRESSOR

signals:
    void done();

public:
    explicit CheckableComboBox(QWidget* parent = nullptr);
    ~CheckableComboBox();

    QVariantList selectedUserData() const;
    void selectUserData(const QVariantList& data);
signals:
    void selectedUserDataChanged(const QVariantList& data);
private slots:
    void onDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles);
    void updateTextHints();

private:
    QModelIndexList checkedIndexes() const;

    bool eventFilter(QObject* obj, QEvent* e) override;
    void paintEvent(QPaintEvent* event) override;

    QString textHint;
};

#endif // CHECKABLECOMBOBOX_H
