#include "LineEditStyle.h"

#include "LineEditEx.h"


LineEditStyle::LineEditStyle(QStyle* style)
    : QProxyStyle(style)
{
}

LineEditStyle::~LineEditStyle()
{
}

QRect LineEditStyle::subElementRect(SubElement element, QStyleOption const* option, const QWidget * widget) const
{
    QRect rc = QProxyStyle::subElementRect( element, option, widget );

    if (element == QStyle::SE_LineEditContents)
    {
        const LineEditEx *le = qobject_cast<const LineEditEx *>(widget);
        if ( le != NULL )
        {
            rc.adjust(0, 0, -le->ButtonsWidth(), 0);
        }
    }

    return rc;
}
