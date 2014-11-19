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


#include "SizeDialog.h"

#include <QLabel>

SizeDialog::SizeDialog(QWidget *parent) :
QDialog(parent)
{
    verticalLayout = new QVBoxLayout(this);
    
    messageLbl = new QLabel(this);
    messageLbl->setText("Set new size for image:");
    verticalLayout->addWidget(messageLbl);
    
    horLayout = new QHBoxLayout(this);
    
    widthLbl = new QLabel(this);
    widthLbl->setText("Width:");
    
    widthSpinBox = new QSpinBox(this);
    widthSpinBox->setMinimum(1);
    widthSpinBox->setMaximum(99999);
    widthSpinBox->setSingleStep(1);
    
    heightLbl = new QLabel(this);
    heightLbl->setText("Height:");
    
    heightSpinBox = new QSpinBox(this);
    heightSpinBox->setMinimum(1);
    heightSpinBox->setMaximum(99999);
    heightSpinBox->setSingleStep(1);
    
    horLayout->addWidget(widthLbl);
    horLayout->addWidget(widthSpinBox);
    horLayout->addWidget(heightLbl);
    horLayout->addWidget(heightSpinBox);
    
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    
    verticalLayout->addLayout(horLayout);
    verticalLayout->addWidget(buttonBox);
}

SizeDialog::~SizeDialog()
{
    delete horLayout;
    delete verticalLayout;
    delete messageLbl;
    delete widthLbl;
    delete widthSpinBox;
    delete heightLbl;
    delete heightSpinBox;
    delete buttonBox;
}

DAVA::Vector2 SizeDialog::GetSize() const
{
    return DAVA::Vector2(widthSpinBox->value(), heightSpinBox->value());
}
