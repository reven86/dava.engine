#include "Base/BaseTypes.h"
#include "TArc/Testing/TArcTestClass.h"
#include "TArc/Testing/TArcUnitTests.h"
#include "TArc/Testing/MockDefine.h"
#include "TArc/Controls/QtWrapLayout.h"
#include "TArc/Controls/QtBoxLayouts.h"
#include "TArc/Controls/LineEdit.h"

#include <QWidget>
#include <QLabel>

//#include "TArc/Core/ClientModule.h"
//
#include <Reflection/ReflectionRegistrator.h>

#include <QResizeEvent>

namespace QtWrapLayoutTestDetail
{
struct LineEditData
{
    DAVA::String text;

    DAVA_REFLECTION(LineEditData);
};

DAVA_REFLECTION_IMPL(LineEditData)
{
    DAVA::ReflectionRegistrator<LineEditData>::Begin()
    .Field("text", &LineEditData::text)
    .End();
}
}

DAVA_TARC_TESTCLASS(QtWrapLayoutTests)
{
  DAVA_TEST (ResizeLayoutTest)
  {
  QWidget* w = new QWidget();
DAVA::TArc::QtWrapLayout* layout = new DAVA::TArc::QtWrapLayout(w);
layout->setMargin(8);
layout->SetHorizontalSpacing(4);
layout->SetVerticalSpacing(3);

QtWrapLayoutTestDetail::LineEditData data;
DAVA::Reflection r = DAVA::Reflection::Create(&data);

{
    DAVA::TArc::QtHBoxLayout* box = new DAVA::TArc::QtHBoxLayout();
    QLabel* l = new QLabel("X:");
    box->addWidget(l);

    DAVA::TArc::ControlDescriptorBuilder<DAVA::TArc::LineEdit::Fields> descriptor;
    descriptor[DAVA::TArc::LineEdit::Fields::Text] = "text";
    DAVA::TArc::LineEdit* lineEdit = new DAVA::TArc::LineEdit(descriptor, GetAccessor(), r);
    box->AddWidget(lineEdit);
    box->setObjectName("boxX");

    layout->AddLayout(box);
}

{
    DAVA::TArc::QtHBoxLayout* box = new DAVA::TArc::QtHBoxLayout();
    QLabel* l = new QLabel("Y:");
    box->addWidget(l);

    DAVA::TArc::ControlDescriptorBuilder<DAVA::TArc::LineEdit::Fields> descriptor;
    descriptor[DAVA::TArc::LineEdit::Fields::Text] = "text";
    DAVA::TArc::LineEdit* lineEdit = new DAVA::TArc::LineEdit(descriptor, GetAccessor(), r);
    box->AddWidget(lineEdit);
    box->setObjectName("boxY");

    layout->AddLayout(box);
}

{
    DAVA::TArc::QtHBoxLayout* box = new DAVA::TArc::QtHBoxLayout();
    QLabel* l = new QLabel("Z:");
    box->addWidget(l);

    DAVA::TArc::ControlDescriptorBuilder<DAVA::TArc::LineEdit::Fields> descriptor;
    descriptor[DAVA::TArc::LineEdit::Fields::Text] = "text";
    DAVA::TArc::LineEdit* lineEdit = new DAVA::TArc::LineEdit(descriptor, GetAccessor(), r);
    box->AddWidget(lineEdit);
    box->setObjectName("boxZ");

    layout->AddLayout(box);
}

{
    DAVA::TArc::QtHBoxLayout* box = new DAVA::TArc::QtHBoxLayout();
    QLabel* l = new QLabel("W:");
    box->addWidget(l);

    DAVA::TArc::ControlDescriptorBuilder<DAVA::TArc::LineEdit::Fields> descriptor;
    descriptor[DAVA::TArc::LineEdit::Fields::Text] = "text";
    DAVA::TArc::LineEdit* lineEdit = new DAVA::TArc::LineEdit(descriptor, GetAccessor(), r);
    box->AddWidget(lineEdit);
    box->setObjectName("boxW");

    layout->AddLayout(box);
}

QLayout* boxXLayout = w->findChild<QLayout*>("boxX");
QLayout* boxYLayout = w->findChild<QLayout*>("boxY");
QLayout* boxZLayout = w->findChild<QLayout*>("boxZ");
QLayout* boxWLayout = w->findChild<QLayout*>("boxW");

TEST_VERIFY(boxXLayout != nullptr);
TEST_VERIFY(boxYLayout != nullptr);
TEST_VERIFY(boxZLayout != nullptr);
TEST_VERIFY(boxWLayout != nullptr);

DAVA::int32 minRowWidth = 2 * 8 + // margin
3 * 4 + // spacing
boxXLayout->minimumSize().width() +
boxYLayout->minimumSize().width() +
boxZLayout->minimumSize().width() +
boxWLayout->minimumSize().width();

w->resize(minRowWidth, 100);
w->show();

{
    DAVA::int32 layoutOffset = 8; // first layout has offset 8 because of margin

    TEST_VERIFY(boxXLayout->geometry() == QRect(QPoint(layoutOffset, 8), boxXLayout->minimumSize()));
    layoutOffset += boxXLayout->geometry().width() + 4;
    TEST_VERIFY(boxYLayout->geometry() == QRect(QPoint(layoutOffset, 8), boxYLayout->minimumSize()));
    layoutOffset += boxYLayout->geometry().width() + 4;
    TEST_VERIFY(boxZLayout->geometry() == QRect(QPoint(layoutOffset, 8), boxZLayout->minimumSize()));
    layoutOffset += boxZLayout->geometry().width() + 4;
    TEST_VERIFY(boxWLayout->geometry() == QRect(QPoint(layoutOffset, 8), boxWLayout->minimumSize()));
}

w->resize(minRowWidth - 60, 100);

{
    DAVA::int32 layoutWOffset = 8; // first layout has offset 8 because of margin

    TEST_VERIFY(boxXLayout->geometry() == QRect(QPoint(layoutWOffset, 8), boxXLayout->geometry().size()));
    layoutWOffset += boxXLayout->geometry().width() + 4;
    TEST_VERIFY(boxYLayout->geometry() == QRect(QPoint(layoutWOffset, 8), boxYLayout->geometry().size()));
    layoutWOffset = 8;
    DAVA::int32 layoutHOffset = DAVA::Max(boxXLayout->geometry().height(), boxYLayout->geometry().height()) + 8 + 3; // 8 - margin, 3 - spacing
    TEST_VERIFY(boxZLayout->geometry() == QRect(QPoint(layoutWOffset, layoutHOffset), boxZLayout->geometry().size()));
    layoutWOffset += boxZLayout->geometry().width() + 4;
    TEST_VERIFY(boxWLayout->geometry() == QRect(QPoint(layoutWOffset, layoutHOffset), boxWLayout->geometry().size()));
}

w->resize(boxXLayout->minimumSize().width() + 10, 100);

{
    DAVA::int32 expectedLayoutWidth = w->width() - 2 * 8;
    DAVA::int32 layoutHOffset = 8; // first layout has offset 8 because of margin

    TEST_VERIFY(boxXLayout->geometry() == QRect(QPoint(8, layoutHOffset), QSize(expectedLayoutWidth, boxXLayout->geometry().height())));
    layoutHOffset += (boxXLayout->geometry().height() + 3);
    TEST_VERIFY(boxYLayout->geometry() == QRect(QPoint(8, layoutHOffset), QSize(expectedLayoutWidth, boxYLayout->geometry().height())));
    layoutHOffset += (boxYLayout->geometry().height() + 3);
    TEST_VERIFY(boxZLayout->geometry() == QRect(QPoint(8, layoutHOffset), QSize(expectedLayoutWidth, boxZLayout->geometry().height())));
    layoutHOffset += (boxZLayout->geometry().height() + 3);
    TEST_VERIFY(boxWLayout->geometry() == QRect(QPoint(8, layoutHOffset), QSize(expectedLayoutWidth, boxWLayout->geometry().height())));
}

delete w;
}
}
;
