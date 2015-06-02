#include "SpyWidget.h"


SpyWidget::SpyWidget( QWidget* parent )
    : QWidget( parent )
{
    setupUi( this );

    connect( keepOnTop, &QCheckBox::stateChanged, this, &SpyWidget::onKeepOnTopChanged );

    onKeepOnTopChanged();
}

SpyWidget::~SpyWidget()
{
}

void SpyWidget::onKeepOnTopChanged()
{
    const auto keepOnTopFlag = keepOnTop->isChecked() ? Qt::WindowStaysOnTopHint : Qt::Widget;
    const auto flags = windowFlags() & ~Qt::WindowStaysOnTopHint;
    setWindowFlags( flags | keepOnTopFlag );

    show();
}