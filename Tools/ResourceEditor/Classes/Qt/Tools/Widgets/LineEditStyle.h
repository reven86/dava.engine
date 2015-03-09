#ifndef LINEEDITSTYLE_H
#define LINEEDITSTYLE_H


#include <QProxyStyle>


class LineEditStyle
    : public QProxyStyle
{
public:
    explicit LineEditStyle(QStyle *style = NULL);
    ~LineEditStyle();

    QRect subElementRect( SubElement element, const QStyleOption * option, const QWidget * widget = 0 ) const;
};


#endif // LINEEDITSTYLE_H
