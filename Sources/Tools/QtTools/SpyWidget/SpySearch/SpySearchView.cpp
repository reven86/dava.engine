#include "SpySearchView.h"

PUSH_QT_WARNING_SUPRESSOR
#include <QAbstractItemModel>
POP_QT_WARNING_SUPRESSOR

SpySearchView::SpySearchView(QWidget* parent)
    : QWidget(parent)
{
    setupUi(this);

    connect(dragSelector, &SpyDragWidget::mouseReleased, this, &SpySearchView::triggered);
    connect(dragSelector, &SpyDragWidget::mousePressed, this, &SpySearchView::OnSelectionStarted);
    connect(dragSelector, &SpyDragWidget::mouseReleased, this, &SpySearchView::OnSelectionDone);

    connect(classNameFilter, &QCheckBox::toggled, classNameText, &QWidget::setEnabled);
    connect(objectNameFilter, &QCheckBox::toggled, objectNameText, &QWidget::setEnabled);
}

SpySearchView::~SpySearchView()
{
}

void SpySearchView::OnSelectionStarted()
{
    if (autoHide->isChecked())
        setWindowOpacity(0.1);
}

void SpySearchView::OnSelectionDone()
{
    setWindowOpacity(1.0);
}
