#pragma once

#include <QObject>

namespace DAVA
{
namespace TArc
{
enum EDGE
{
    LEFT_EDGE = (1 << 0),
    TOP_EDGE = (1 << 1),
    RIGHT_EDGE = (1 << 2),
    BOTTOM_EDGE = (1 << 3),
};

Q_DECLARE_FLAGS(EDGES, EDGE)

Q_DECLARE_OPERATORS_FOR_FLAGS(EDGES)
}
}
