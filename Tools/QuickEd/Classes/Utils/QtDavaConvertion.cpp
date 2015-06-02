/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


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
