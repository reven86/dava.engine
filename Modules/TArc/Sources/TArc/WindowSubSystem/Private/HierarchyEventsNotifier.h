#pragma once

#include <QObject>
namespace DAVA
{
namespace TArc
{
class HierarchyEventsNotifier : public QObject
{
public:
    HierarchyEventsNotifier(QObject* parent);

protected:
    bool eventFilter(QObject* obj, QEvent* e) override;

private:
    bool isInFiltering = false;
};
} // namespace TArc
} // namespace DAVA