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


#ifndef __QUICKED_PROPERTIES_MODEL_H__
#define __QUICKED_PROPERTIES_MODEL_H__

#include <QAbstractItemModel>

#include "FileSystem/VariantType.h"
#include "Model/ControlProperties/PropertyListener.h"

namespace DAVA {
    class InspInfo;
    enum ItemDataRole
    {
        ResetRole = Qt::UserRole +1,
    };
}

class AbstractProperty;
class ControlNode;
class StyleSheetNode;
class QtModelPackageCommandExecutor;
class ComponentPropertiesSection;

class PropertiesModel : public QAbstractItemModel, private PropertyListener
{
    Q_OBJECT
    
public:
    PropertiesModel(ControlNode *controlNode, QtModelPackageCommandExecutor *_commandExecutor, QObject *parent = nullptr);
    PropertiesModel(StyleSheetNode *styleSheet, QtModelPackageCommandExecutor *_commandExecutor, QObject *parent = nullptr);
    virtual ~PropertiesModel();
    
    ControlNode *GetControlNode() const {return controlNode; }
    
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    virtual QModelIndex parent(const QModelIndex &child) const override;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const  override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const  override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const override;

private: // PropertyListener
    void PropertyChanged(AbstractProperty *property) override;

    void ComponentPropertiesWillBeAdded(RootProperty *root, ComponentPropertiesSection *section, int index) override;
    void ComponentPropertiesWasAdded(RootProperty *root, ComponentPropertiesSection *section, int index) override;
    
    void ComponentPropertiesWillBeRemoved(RootProperty *root, ComponentPropertiesSection *section, int index) override;
    void ComponentPropertiesWasRemoved(RootProperty *root, ComponentPropertiesSection *section, int index) override;

    void StylePropertyWillBeAdded(StyleSheetPropertiesSection *section, StyleSheetProperty *property, int index) override;
    void StylePropertyWasAdded(StyleSheetPropertiesSection *section, StyleSheetProperty *property, int index) override;
    
    void StylePropertyWillBeRemoved(StyleSheetPropertiesSection *section, StyleSheetProperty *property, int index) override;
    void StylePropertyWasRemoved(StyleSheetPropertiesSection *section, StyleSheetProperty *property, int index) override;

    void StyleSelectorWillBeAdded(StyleSheetSelectorsSection *section, StyleSheetSelectorProperty *property, int index) override;
    void StyleSelectorWasAdded(StyleSheetSelectorsSection *section, StyleSheetSelectorProperty *property, int index) override;
    
    void StyleSelectorWillBeRemoved(StyleSheetSelectorsSection *section, StyleSheetSelectorProperty *property, int index) override;
    void StyleSelectorWasRemoved(StyleSheetSelectorsSection *section, StyleSheetSelectorProperty *property, int index) override;

private:
    void ChangeProperty(AbstractProperty *property, const DAVA::VariantType &value);
    void ResetProperty(AbstractProperty *property);
    
private:
    QModelIndex indexByProperty(AbstractProperty *property, int column = 0);
    QString makeQVariant(const AbstractProperty *property) const;
    void initVariantType(DAVA::VariantType &var, const QVariant &val) const;
    
private:
    ControlNode *controlNode = nullptr;
    StyleSheetNode *styleSheet = nullptr;
    AbstractProperty *rootProperty = nullptr;
    QtModelPackageCommandExecutor *commandExecutor = nullptr;
};

#endif // __QUICKED_PROPERTIES_MODEL_H__
