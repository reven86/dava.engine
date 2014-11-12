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


#ifndef BASEPROPERTYGRIDWIDGET_H
#define BASEPROPERTYGRIDWIDGET_H

#include <QWidget>
#include <QMetaProperty>

#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include "QColorWidget.h"

#include "HierarchyTreeNode.h"
#include "BaseMetadata.h"
#include "PropertyGridWidgetData.h"

#include "UI/UIControl.h"

using namespace DAVA;

namespace Ui {
class BasePropertyGridWidget;
}

class BasePropertyGridWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit BasePropertyGridWidget(QWidget *parent = 0);
    ~BasePropertyGridWidget();
    
    // Setup the Property Block name.
    void SetPropertyBlockName(const QString& blockName);

    // Initialize with the metatada assigned.
    virtual void Initialize(BaseMetadata* metaData);
    virtual void Cleanup();

    typedef Map<QWidget*, PropertyGridWidgetData> PROPERTYGRIDWIDGETSMAP;
    typedef PROPERTYGRIDWIDGETSMAP::iterator PROPERTYGRIDWIDGETSITER;
    typedef PROPERTYGRIDWIDGETSMAP::const_iterator PROPERTYGRIDWIDGETSCONSTITER;
    
protected:
    // Active metadata.
    BaseMetadata* activeMetadata;

    // Map of the widget and properties added to them.
    PROPERTYGRIDWIDGETSMAP propertyGridWidgetsMap;
    Map<QWidget*, String> invokableMethodsMap;
    
    typedef Map<String, QMetaProperty> PROPERTIESMAP;
    typedef PROPERTIESMAP::iterator PROPERTIESMAPITER;
    typedef PROPERTIESMAP::const_iterator PROPERTIESMAPCONSTITER;

    // Build the properties map in the "name - value" way to make the search faster.
    BasePropertyGridWidget::PROPERTIESMAP BuildMetadataPropertiesMap();

    // Register/unregister grid control as state-aware. This method should be called by
    // any Property Grid Widged which contains state-aware data.
    void RegisterGridWidgetAsStateAware();
    void UnregisterGridWidgetAsStateAware();
    
    // Register different widgets types for the different properties.
    void RegisterLineEditWidgetForProperty(const PROPERTIESMAP& propertiesMap, const char* propertyName,
                                           QLineEdit* lineEditWidget,
                                           bool needUpdateTree = false, bool stateAware = false);
    void RegisterSpinBoxWidgetForProperty(const PROPERTIESMAP& propertiesMap, const char* propertyName,
                                           QSpinBox* spinBoxWidget,
                                          bool needUpdateTree = false, bool stateAware = false);
    void RegisterDoubleSpinBoxWidgetForProperty(const PROPERTIESMAP& propertiesMap, const char* propertyName,
                                           QDoubleSpinBox* doubleSpinBoxWidget,
                                          bool needUpdateTree = false, bool stateAware = false);
    void RegisterCheckBoxWidgetForProperty(const PROPERTIESMAP& propertiesMap, const char* propertyName,
                                           QCheckBox* checkBoxWidget,
                                           bool needUpdateTree = false, bool stateAware = false);
    void RegisterComboBoxWidgetForProperty(const PROPERTIESMAP& propertiesMap, const char* propertyName,
                                           QComboBox* comboBoxWidget,
                                           bool needUpdateTree = false, bool stateAware = false);
    void RegisterColorWidgetForProperty(const PROPERTIESMAP& propertiesMap, const char* propertyName,
                                            QColorWidget* colorWidget,
                                            bool needUpdateTree = false, bool stateAware = false);
    void RegisterPushButtonWidgetForProperty(const PROPERTIESMAP& propertiesMap, const char* propertyName,
                                             QPushButton* pushButtonWidget,
                                             bool needUpdateTree = false, bool stateAware = false);

    // We can also attach push buttons to the methods from metadata.
    void RegisterPushButtonWidgetForInvokeMethod(QPushButton *pushButton, const String& methodName);

    // Unregister the widgets.
    void UnregisterLineEditWidget(QLineEdit* lineEdit);
    void UnregisterSpinBoxWidget(QSpinBox* spinBoxWidget);
	void UnregisterDoubleSpinBoxWidget(QDoubleSpinBox* spinBoxWidget);
    void UnregisterCheckBoxWidget(QCheckBox* checkBoxWidget);
    void UnregisterComboBoxWidget(QComboBox* comboBoxWidget);
    void UnregisterColorWidget(QColorWidget* colorWidget);
    void UnregisterPushButtonWidget(QPushButton* pushButtonWidget);
    void UnregisterPushButtonWidgetForInvokeMethod(QPushButton* pushButtonWidget);

    // Process the data attached to each property when it is changed.
    void ProcessAttachedData(const PropertyGridWidgetData& attachedData);

    // Get the data from the property and update the particular widget types.
    void UpdateWidgetWithPropertyValue(const QString& propertyName);
    void UpdateWidgetWithPropertyValue(const PROPERTYGRIDWIDGETSITER& iter);

    virtual void UpdateLineEditWidgetWithPropertyValue(QLineEdit* lineEditWidget, const QMetaProperty& curProperty);
    //void UpdateCheckBoxWidgetWithPropertyValue(QCheckBox* checkBoxWidget, const QMetaProperty& curProperty);
    virtual void UpdateSpinBoxWidgetWithPropertyValue(QSpinBox* spinBoxWidget, const QMetaProperty& curProperty);
    virtual void UpdateColorWidgetWithPropertyValue(QColorWidget* colorWidget, const QMetaProperty& curProperty);

    // Override this method to get a notification about properties changed from external source.
    virtual void OnPropertiesChangedFromExternalSource();
	
	// For aligns property widget we need it's own update method
	virtual void UpdateCheckBoxWidgetWithPropertyValue(QCheckBox* checkBoxWidget, const QMetaProperty& curProperty);
	 
	// Double spinbox are used in slider widget - so we should track them separately
	virtual void UpdateDoubleSpinBoxWidgetWithPropertyValue(QDoubleSpinBox* spinBoxWidget, const QMetaProperty& curProperty);

    // Comboboxes are processed separately - depending on their content. That's why these methods are
    // virtual.
    virtual void UpdateComboBoxWidgetWithPropertyValue(QComboBox* comboBoxWidget, const QMetaProperty& curProperty);
    
    // PushButton are processed separately - depending on their content
    virtual void UpdatePushButtonWidgetWithPropertyValue(QPushButton *pushButtonWidget, const QMetaProperty& curProperty);

    // Process the Combobox Value Changed.
    virtual void ProcessComboboxValueChanged(QComboBox* senderWidget, const PROPERTYGRIDWIDGETSITER& iter,
                                             const QString& value);
	// Handle Double SpinBox value changes
	virtual void ProcessDoubleSpinBoxValueChanged(QDoubleSpinBox* senderWidget, const PROPERTYGRIDWIDGETSITER& iter,
												const double value);

    // Set the combobox selected item by the item text.
    void SetComboboxSelectedItem(QComboBox* comboBoxWidget, const QString& selectedItemText, bool byToolTip = false);

    // Handle Line Edit Editing finished.
    virtual void HandleLineEditEditingFinished(QLineEdit* senderWidget);
    
    //Handle Push Button Clicked.
    virtual void ProcessPushButtonClicked(QPushButton* senderWidget);

    // Handle Invoke Method is requested.
    void ProcessInvokeMethodRequested(QWidget* /*widget*/, const String& methodName);

    // Handle UI Control State is changed.
    virtual void HandleSelectedUIControlStatesChanged(const Vector<UIControl::eControlState>& newStates);

    // These methods are called when property change is succeeded/failed.
    virtual void HandleChangePropertySucceeded(const QString& propertyName);
    virtual void HandleChangePropertyFailed(const QString& propertyName);

    // Verify whether particular widget is bound to the property.
    bool IsWidgetBoundToProperty(QWidget* widget, const QString& propertyName) const;
    
    // Whether the property is dirty for current UI Control State and for particular state?
    bool IsActiveStatePropertyDirty(const QString& propertyName) const;
    bool IsPropertyDirty(const QString& propertyName, UIControl::eControlState state) const;
	
	// Get int value for property
	int GetPropertyIntValue(const QString& propertyName);
	// Get boolean property value by name
	bool GetPropertyBooleanValue(const QString& propertyName) const;

    // Update the widget palette.
    void UpdateWidgetPalette(QWidget* widget, const QString& propertyName);

    // Prepare the "clean" and "dirty" palettes for furhter use.
    void PreparePalettes();

    // Get the palette for "dirty" and "clear" properties.
    virtual const QPalette& GetWidgetPaletteForDirtyProperty() const;
    virtual const QPalette& GetWidgetPaletteForClearProperty() const;
	
	virtual void InstallEventFiltersForWidgets(QWidget *widget);
	// Event filter to block wheel events for comboboxes and spinbox
	virtual bool eventFilter(QObject *obj, QEvent *event);

	// Is the control a subcontrol? (extended UIEditor's version).
	bool IsSubcontrolInUIEditor(UIControl* uiControl);

	// Whether the active control IS subcontrol?
	bool ActiveControlIsSubcontrol();

    // Pre-process the sprite name.
    QString PreprocessSpriteName(const QString& rawSpriteName);

protected slots:
    // Properties are updated from the external source and needs to be re-drawn.
    void OnPropertiesChangedExternally();

    // Slot for UI Control State Changed notification.
	void OnSelectedUIControlStatesChanged(const Vector<UIControl::eControlState>& newStates);
    
    // Slots for different widget types.
    void OnSpinBoxValueChanged(int value);
	void OnDoubleSpinBoxValueChanged(double value);
    void OnLineEditEditingFinished();
    void OnCheckBoxStateChanged(int state);
    void OnComboBoxValueChanged(QString value);

    void OnColorChanged(const QColor& color);
    void OnPushButtonClicked();

    // Invoke method is requested
    void OnInvokeMethodRequested();

    // These methods are called when property change is succeeded/failed.
    void OnChangePropertySucceeded(const QString& propertyName);
    void OnChangePropertyFailed(const QString& propertyName);

protected:
    // Get the "dirty states" indexes for the particular property.
    QList<int> GetDirtyStateIndexesList(const QString& propertyName);
    static const QString MULTIPLE_VALUE;
private:
    // Palettes.
    QPalette clearPropertyPalette;
    QPalette dirtyPropertyPalette;

private:
    Ui::BasePropertyGridWidget *ui;
};

#endif // BASEPROPERTYGRIDWIDGET_H
