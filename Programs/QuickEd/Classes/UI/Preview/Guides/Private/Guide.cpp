#include "UI/Preview/Guides/Guide.h"
#include "UI/Preview/Guides/GuideLabel.h"

#include <QWidget>

void Guide::Show()
{
    line->show();
    text->show();
}

void Guide::Raise()
{
    line->raise();
    text->raise();
}

void Guide::Hide()
{
    line->hide();
    text->hide();
}
