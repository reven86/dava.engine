#include "QtDavaConvertion.h"
#include <QString>
#include <QColor>
#include <QVector2D>


DAVA::String QStringToString(const QString &str)
{
    return DAVA::String(str.toStdString());
}

QString StringToQString(const DAVA::String &str)
{
    return QString::fromStdString(str);
}

DAVA::WideString QStringToWideString(const QString& str)
{
#ifdef __DAVAENGINE_MACOS__
    return str.toStdWString();
#else
    return DAVA::WideString((wchar_t*)str.unicode(), str.length());
#endif
}

QString WideStringToQString(const DAVA::WideString& str)
{
#ifdef __DAVAENGINE_MACOS__
    return QString::fromStdWString(str);
#else
    return QString((const QChar*)str.c_str(), str.length());
#endif
}

DAVA::Color QColorToColor( const QColor& qtColor )
{
    return DAVA::Color(qtColor.redF(), qtColor.greenF(), qtColor.blueF(), qtColor.alphaF());
}

QColor ColorToQColor( const DAVA::Color& davaColor )
{
    return QColor((int)DAVA::Round(davaColor.r * 255.0f), (int)DAVA::Round(davaColor.g * 255.0f), (int)DAVA::Round(davaColor.b * 255.0f), (int)DAVA::Round(davaColor.a * 255.0f));
}

DAVA::Vector2 QVector2DToVector2( const QVector2D &vector )
{
    return DAVA::Vector2(vector.x(), vector.y());
}

QVector2D Vector2ToQVector2D( const DAVA::Vector2 &vector )
{
    return QVector2D(vector.x, vector.y);
}

QColor HexToQColor(const QString &str)
{
    QColor color;
    if (!str.startsWith("#"))
        return color;
    int len = str.length() - 1;
    switch (len)
    {
    case 6://RGB
        {
            int r, g, b;
            if(3 == sscanf(str.toStdString().c_str(), "#%02x%02x%02x", &r, &g, &b))
            {
                color.setRgb(qRgb(r, g, b));
            }
        }
        break;
    case 8://RGBA
        {
            int r, g, b, a;
            if(4 == sscanf(str.toStdString().c_str(), "#%02x%02x%02x%02x", &r, &g, &b, &a))
            {
                color.setRgba(qRgba(r, g, b, a));
            }
        }
        break;
    default:
        break;
    }

    return color;
}

QString QColorToHex( const QColor &color )
{
    QString str;
    str.sprintf("#%02x%02x%02x%02x", color.red(), color.green(), color.blue(), color.alpha());

    return str;
}
