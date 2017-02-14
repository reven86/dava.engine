#pragma once

#include <Base/BaseTypes.h>
#include <QLayout>

class QWidget;
namespace DAVA
{
namespace TArc
{
class QtWrapLayoutPrivate;
class QtWrapLayout : public QLayout
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QtWrapLayout);

public:
    QtWrapLayout(QWidget* parent = nullptr);

    int32 GetHorizontalSpacing() const;
    void SetHorizontalSpacing(int32 spacing);

    int32 GetVerticalSpacing() const;
    void SetVerticalSpacing(int32 spacing);

    void AddLayout(QLayout* layout);

    // reimplemented from QLayout
    void addItem(QLayoutItem* item) override;
    QLayoutItem* itemAt(int index) const override;
    QLayoutItem* takeAt(int index) override;

    void setGeometry(const QRect& rect) override;
    QSize minimumSize() const override;
    QSize sizeHint() const override;
    void invalidate() override;

    bool hasHeightForWidth() const override;
    int heightForWidth(int width) const override;
    Qt::Orientations expandingDirections() const override;
    int count() const override;

private:
    QtWrapLayoutPrivate* GetNonConstPrivate() const;
};
} // namespace TArc
} // namespace DAVA