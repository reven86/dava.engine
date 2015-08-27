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


#include "PackageMimeData.h"

#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/PackageHierarchy/StyleSheetNode.h"

using namespace DAVA;

const QString PackageMimeData::MIME_TYPE = "application/packageModel";

PackageMimeData::PackageMimeData()
{
}

PackageMimeData::~PackageMimeData()
{
    for (ControlNode *control : controls)
        control->Release();
    controls.clear();

    for (StyleSheetNode *style : styles)
        style->Release();
    
    styles.clear();
}

void PackageMimeData::AddControl(ControlNode *node)
{
    controls.push_back(SafeRetain(node));
}

void PackageMimeData::AddStyle(StyleSheetNode *node)
{
    styles.push_back(SafeRetain(node));
}

const Vector<ControlNode*> &PackageMimeData::GetControls() const
{
    return controls;
}

const Vector<StyleSheetNode*> &PackageMimeData::GetStyles() const
{
    return styles;
}

bool PackageMimeData::hasFormat(const QString &mimetype) const
{
    if (mimetype == MIME_TYPE)
        return true;
    return QMimeData::hasFormat(mimetype);
}

QStringList PackageMimeData::formats() const
{
    QStringList types;
    types << "text/plain";
    types << MIME_TYPE;
    return types;
}

QVariant PackageMimeData::retrieveData(const QString &mimetype, QVariant::Type preferredType) const
{
    if (mimetype == MIME_TYPE)
        return QVariant(QVariant::UserType);
    
    return QMimeData::retrieveData(mimetype, preferredType);
}
