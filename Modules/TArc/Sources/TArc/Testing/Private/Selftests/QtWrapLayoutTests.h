#include "Base/BaseTypes.h"
#include "TArc/Testing/TArcTestClass.h"
#include "TArc/Testing/TArcUnitTests.h"
#include "TArc/Testing/MockDefine.h"
#include "TArc/Controls/QtWrapLayout.h"
#include "TArc/Controls/QtBoxLayouts.h"
#include "TArc/Controls/LineEdit.h"

#include <Reflection/ReflectionRegistrator.h>

#include <QWidget>
#include <QLabel>
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

// clang-format off
DAVA_TARC_TESTCLASS(QtWrapLayoutTests)
{
    DAVA_TEST (ResizeLayoutTest)
    {
        QWidget* w = new QWidget();
        DAVA::TArc::QtWrapLayout* layout = new DAVA::TArc::QtWrapLayout(w);
        DAVA::int32 margin = 8;
        DAVA::int32 hSpacing = 4;
        DAVA::int32 vSpacing = 3;
        layout->setMargin(margin);
        layout->SetHorizontalSpacing(hSpacing);
        layout->SetVerticalSpacing(vSpacing);

        QtWrapLayoutTestDetail::LineEditData data;
        DAVA::Reflection r = DAVA::Reflection::Create(&data);

        using namespace DAVA::TArc;

        {
            DAVA::TArc::QtHBoxLayout* box = new DAVA::TArc::QtHBoxLayout();
            QLabel* l = new QLabel("X:");
            box->addWidget(l);

            DAVA::TArc::LineEdit::Params params(GetAccessor(), GetUI(), DAVA::TArc::mainWindowKey);
            params.fields[DAVA::TArc::LineEdit::Fields::Text] = "text";
            DAVA::TArc::LineEdit* lineEdit = new DAVA::TArc::LineEdit(params, GetAccessor(), r);
            box->AddControl(lineEdit);
            box->setObjectName("boxX");

            layout->AddLayout(box);
        }

        {
            DAVA::TArc::QtHBoxLayout* box = new DAVA::TArc::QtHBoxLayout();
            QLabel* l = new QLabel("Y:");
            box->addWidget(l);

            DAVA::TArc::LineEdit::Params params(GetAccessor(), GetUI(), DAVA::TArc::mainWindowKey);
            params.fields[DAVA::TArc::LineEdit::Fields::Text] = "text";
            DAVA::TArc::LineEdit* lineEdit = new DAVA::TArc::LineEdit(params, GetAccessor(), r);
            box->AddControl(lineEdit);
            box->setObjectName("boxY");

            layout->AddLayout(box);
        }

        {
            DAVA::TArc::QtHBoxLayout* box = new DAVA::TArc::QtHBoxLayout();
            QLabel* l = new QLabel("Z:");
            box->addWidget(l);

            DAVA::TArc::LineEdit::Params params(GetAccessor(), GetUI(), DAVA::TArc::mainWindowKey);
            params.fields[DAVA::TArc::LineEdit::Fields::Text] = "text";
            DAVA::TArc::LineEdit* lineEdit = new DAVA::TArc::LineEdit(params, GetAccessor(), r);
            box->AddControl(lineEdit);
            box->setObjectName("boxZ");

            layout->AddLayout(box);
        }

        {
            DAVA::TArc::QtHBoxLayout* box = new DAVA::TArc::QtHBoxLayout();
            QLabel* l = new QLabel("W:");
            box->addWidget(l);

            DAVA::TArc::LineEdit::Params params(GetAccessor(), GetUI(), DAVA::TArc::mainWindowKey);
            params.fields[DAVA::TArc::LineEdit::Fields::Text] = "text";
            DAVA::TArc::LineEdit* lineEdit = new DAVA::TArc::LineEdit(params, GetAccessor(), r);
            box->AddControl(lineEdit);
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

        DAVA::int32 minRowWidth = 2 * margin + 3 * hSpacing +
                                  boxXLayout->minimumSize().width() +
                                  boxYLayout->minimumSize().width() +
                                  boxZLayout->minimumSize().width() +
                                  boxWLayout->minimumSize().width();

        w->resize(minRowWidth, 100);
        w->show();

        {
            DAVA::int32 layoutOffset = margin; // first layout has offset 8 because of margin

            TEST_VERIFY(boxXLayout->geometry() == QRect(QPoint(layoutOffset, margin), boxXLayout->minimumSize()));
            layoutOffset += boxXLayout->geometry().width() + hSpacing;
            TEST_VERIFY(boxYLayout->geometry() == QRect(QPoint(layoutOffset, margin), boxYLayout->minimumSize()));
            layoutOffset += boxYLayout->geometry().width() + hSpacing;
            TEST_VERIFY(boxZLayout->geometry() == QRect(QPoint(layoutOffset, margin), boxZLayout->minimumSize()));
            layoutOffset += boxZLayout->geometry().width() + hSpacing;
            TEST_VERIFY(boxWLayout->geometry() == QRect(QPoint(layoutOffset, margin), boxWLayout->minimumSize()));
        }

        w->resize(minRowWidth - 60, 100);

        {
            DAVA::int32 layoutWOffset = margin; // first layout has offset 8 because of margin

            TEST_VERIFY(boxXLayout->geometry() == QRect(QPoint(layoutWOffset, margin), boxXLayout->geometry().size()));
            layoutWOffset += boxXLayout->geometry().width() + hSpacing;
            TEST_VERIFY(boxYLayout->geometry() == QRect(QPoint(layoutWOffset, margin), boxYLayout->geometry().size()));
            layoutWOffset = margin;
            DAVA::int32 layoutHOffset = DAVA::Max(boxXLayout->geometry().height(), boxYLayout->geometry().height()) + margin + vSpacing; // 8 - margin, 3 - spacing
            TEST_VERIFY(boxZLayout->geometry() == QRect(QPoint(layoutWOffset, layoutHOffset), boxZLayout->geometry().size()));
            layoutWOffset += boxZLayout->geometry().width() + hSpacing;
            TEST_VERIFY(boxWLayout->geometry() == QRect(QPoint(layoutWOffset, layoutHOffset), boxWLayout->geometry().size()));
        }

        w->resize(boxXLayout->minimumSize().width() + 10, 100);

        {
            DAVA::int32 expectedLayoutWidth = w->width() - 2 * margin;
            DAVA::int32 layoutHOffset = margin;

            TEST_VERIFY(boxXLayout->geometry() == QRect(QPoint(margin, layoutHOffset), QSize(expectedLayoutWidth, boxXLayout->geometry().height())));
            layoutHOffset += (boxXLayout->geometry().height() + vSpacing);
            TEST_VERIFY(boxYLayout->geometry() == QRect(QPoint(margin, layoutHOffset), QSize(expectedLayoutWidth, boxYLayout->geometry().height())));
            layoutHOffset += (boxYLayout->geometry().height() + vSpacing);
            TEST_VERIFY(boxZLayout->geometry() == QRect(QPoint(margin, layoutHOffset), QSize(expectedLayoutWidth, boxZLayout->geometry().height())));
            layoutHOffset += (boxZLayout->geometry().height() + vSpacing);
            TEST_VERIFY(boxWLayout->geometry() == QRect(QPoint(margin, layoutHOffset), QSize(expectedLayoutWidth, boxWLayout->geometry().height())));
        }

        delete w;
    }
};
// clang-format on
