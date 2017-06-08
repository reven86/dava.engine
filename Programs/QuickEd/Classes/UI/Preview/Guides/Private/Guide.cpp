#include "UI/Preview/Guides/Guide.h"

#include <QWidget>

void Guide::Show()
{
    line->show();
    text->show();
}

void Guide::Hide()
{
    line->hide();
    text->hide();
}
