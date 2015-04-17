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


#ifndef EDITFONTDIALOG_H
#define EDITFONTDIALOG_H

#include "ui_editfontdialog.h"
#include "Base/BaseTypes.h"

class ControlNode;
namespace DAVA {
    class Font;
}

class EditFontDialog : public QDialog, public Ui::EditFontDialog
{
    Q_OBJECT

public:
    explicit EditFontDialog(QWidget *parent = nullptr);
    ~EditFontDialog() = default;
    void OnPropjectOpened();
    //! \brief update fields for existing dialog
    //! \param[in] control node, which contains font property
    void UpdateFontPreset(ControlNode *selectedControlNode);
    //! \brief functions return font preset from given node
    DAVA::String findFont(ControlNode *node);
private slots:
    //! \brief update locale button and locale spinbox for current language
    void UpdateLocaleFontWidgets();
    //! \brief set default font preset for current locale
    void ResetCurrentLocale();
    //! \brief set default font preset for all locales
    void ResetAllLocales();
    //! \brief apply changes
    void OnApplyToAll();
    void OnCreateNew();
private:
    void ResetFontForLocale(const QString &locale);
    DAVA::Font* GetLocaleFont(const DAVA::String &locale) const;
    //! \brief update current locale
    void UpdateCurrentLocale();
    //! \brief update 
    void UpdateDefaultFontWidgets();
    DAVA::String presetName;
};

#endif // EDITFONTDIALOG_H
