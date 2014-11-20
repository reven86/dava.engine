//
//  LineEditExt.cpp
//  UIEditor
//
//  Created by Alexey Strokachuk on 9/20/14.
//
//

#include "LineEditExt.h"
#include <QCommonStyle>
#include <QHBoxLayout>
#include <QToolButton>

LineEditExt::~LineEditExt()
{
    delete clearButton;
}

LineEditExt::LineEditExt(QWidget *parent)
    : QLineEdit(parent)
    , buttonEnabled(true)
{
    clearButton = new QToolButton(this);
    clearButton->setFixedSize(18, 18);
    clearButton->setCursor(Qt::ArrowCursor);
    clearButton->setIcon(QIcon(":/Icons/editclear.png"));
    clearButton->setStyleSheet(buttonStyleSheetForCurrentState());
    
    // Some stylesheet and size corrections for the text box
    setStyleSheet(this->styleSheetForCurrentState());
    
    int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    QSize minSizeHint = minimumSizeHint();
    setMinimumSize(qMax(minSizeHint.width(), clearButton->sizeHint().width() + frameWidth * 2 + 2),
                   qMax(minSizeHint.height(), clearButton->sizeHint().height() + frameWidth * 2 + 2));
    setSizePolicy(QSizePolicy::Expanding, sizePolicy().verticalPolicy());
    
    updateClearButton(text());
    
    // Update the clear button when the text changes
    QObject::connect(this, SIGNAL(textChanged(QString)), SLOT(updateClearButton(QString)));
}

bool LineEditExt::clearButtonEnabled() const
{
    return buttonEnabled;
}

void LineEditExt::setClearButtonEnabled(bool enable)
{
    if (buttonEnabled == enable)
        return;
    
    buttonEnabled = enable;
    updateClearButton(text());
}

void LineEditExt::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    QSize size = clearButton->sizeHint();
    int frameWidth = this->style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    clearButton->move(rect().right() - frameWidth - size.width() - 2, (rect().bottom() + 2 - size.height()) / 2);
}

void LineEditExt::updateClearButton(const QString &text)
{
    if (!text.isEmpty())
    {
        clearButton->show();
        connect(clearButton, SIGNAL(clicked()), SLOT(clear()));
        
    }
    else
    {
        disconnect(clearButton, SIGNAL(clicked()), this, SLOT(clear()));
        clearButton->hide();
        
    }
    
    setStyleSheet(styleSheetForCurrentState());
}

QString LineEditExt::styleSheetForCurrentState() const
{
    int frameWidth = this->style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    
    QString style;
    style += "QLineEdit {";
    if (this->text().isEmpty())
    {
        style += "font-family: 'MS Sans Serif';";
        style += "font-style: italic;";
    }
    
    style += "padding-left: 3px;";
    style += QString("padding-right: %1px;").arg(clearButton->sizeHint().width() + frameWidth + 1);
    style += "border-width: 3px;";
    style += "background-color: rgba(255, 255, 255, 204);";
    style += "}";
    style += "QLineEdit:hover, QLineEdit:focus {";
    style += "background-color: rgba(255, 255, 255, 255);";
    style += "}";
    return style;
}

QString LineEditExt::buttonStyleSheetForCurrentState() const
{
    QString style;
    
    style += "QToolButton {";
    style += "border: none; margin: 0; padding: 0;";
    style += "}";
    
    return style;
}