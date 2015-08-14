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


#ifndef __UI_EDITOR_UI_PACKAGE_MIME_DATA_H__
#define __UI_EDITOR_UI_PACKAGE_MIME_DATA_H__

#include <QMimeData>
#include <QModelIndex>
#include <QPersistentModelIndex>
#include <QStringList>

#include "Base/BaseTypes.h"

class ControlNode;
class StyleSheetNode;

class PackageMimeData: public QMimeData
{
    Q_OBJECT
    
public:
    static const QString MIME_TYPE;

public:
    PackageMimeData();
    virtual ~PackageMimeData();
    
    void AddControl(ControlNode *node);
    void AddStyle(StyleSheetNode *node);

    const DAVA::Vector<ControlNode*> &GetControls() const;
    const DAVA::Vector<StyleSheetNode*> &GetStyles() const;
    
    virtual bool hasFormat(const QString &mimetype) const override;
    virtual QStringList formats() const override;
    
protected:
    virtual QVariant retrieveData(const QString &mimetype, QVariant::Type preferredType) const override;
    
private:
    DAVA::Vector<ControlNode*> controls;
    DAVA::Vector<StyleSheetNode*> styles;
};


#endif // __UI_EDITOR_UI_PACKAGE_MIME_DATA_H__
