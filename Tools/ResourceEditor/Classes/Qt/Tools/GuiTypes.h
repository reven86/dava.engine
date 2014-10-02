#ifndef __GUI_TYPES_H__
#define __GUI_TYPES_H__

#include <QObject>


enum EDGE
{
    LEFT_EDGE =     (1 << 0),
    TOP_EDGE =      (1 << 1),
    RIGHT_EDGE =    (1 << 2),
    BOTTOM_EDGE =   (1 << 3),
};


Q_DECLARE_FLAGS( EDGES, EDGE )

Q_DECLARE_OPERATORS_FOR_FLAGS( EDGES )


#endif // __GUI_TYPES_H__
