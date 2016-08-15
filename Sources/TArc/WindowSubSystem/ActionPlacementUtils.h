#pragma once

#include <QString>
#include <QUrl>

namespace tarc
{
static const QString menuScheme = QStringLiteral("menu");
static const QString toolbarScheme = QStringLiteral("toolbar");

QUrl CreateMenuPoint(const QString& path);

}