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


#ifndef PREFERENCES_MODEL
#define PREFERENCES_MODEL

#include "Base/Introspection.h"
#include <QStandardItemModel>

class PreferencesModel : public QStandardItemModel
{
    Q_OBJECT

public:
    enum eType
    {
        TYPE_INSP,
        TYPE_MEMBER
    };

    enum eRoles
    {
        ROLE_TYPE = Qt::UserRole + 1,
        ROLE_POINTER
    };

    PreferencesModel(QObject* parent);
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

private:
    void BuildTree();
    static QString MakeDataFromVariant(const DAVA::InspMember* member);
};

Q_DECLARE_METATYPE(PreferencesModel::eType);
Q_DECLARE_METATYPE(PreferencesModel::eRoles);
Q_DECLARE_METATYPE(const DAVA::InspInfo*);
Q_DECLARE_METATYPE(const DAVA::InspMember*);

#endif //PREFERENCES_MODEL
