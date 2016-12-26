import QtQuick 2.4
import QtQuick.Controls 1.4
import QtQuick.Controls.Private 1.0

AbstractCheckable
{
    id: checkBox
    objectName: "CheckBase"

    /*!
        \qmlproperty enumeration CheckBox::checkedState

        This property indicates the current checked state of the checkbox.

        Possible values:
        \c Qt.UnChecked - The checkbox is not checked (default).
        \c Qt.Checked - The checkbox is checked.
        \c Qt.PartiallyChecked - The checkbox is in a partially checked (or
        "mixed") state.

        The \l {AbstractCheckable::checked}{checked} property also determines whether
        this property is \c Qt.Checked or \c Qt.UnChecked, and vice versa.
    */
    property int checkedState: multipleValues ? Qt.PartiallyChecked : checked ? Qt.Checked : Qt.Unchecked

    /*!
        This property determines whether the \c Qt.PartiallyChecked state is
        available.

        A checkbox may be in a partially checked state when the regular checked
        state can not be determined.

        Setting \l checkedState to \c Qt.PartiallyChecked will implicitly set
        this property to \c true.

        If this property is \c true, \l {AbstractCheckable::checked}{checked} will be \c false.

        By default, this property is \c false.
    */
    property bool partiallyCheckedEnabled: false

    /*!
        \internal
        True if onCheckedChanged should be ignored because we were reacting
        to onCheckedStateChanged.
    */
    property bool __ignoreChecked: false

    /*!
        \internal
        True if onCheckedStateChanged should be ignored because we were reacting
        to onCheckedChanged.
    */
    property bool __ignoreCheckedState: false

    style: Settings.styleComponent(Settings.style, "CheckBoxStyle.qml", checkBox)

    activeFocusOnTab: true

    Accessible.role: Accessible.CheckBox
    Accessible.name: text

    __cycleStatesHandler: __cycleCheckBoxStates

    onCheckedChanged:
    {
        if (!__ignoreChecked)
        {
            __ignoreCheckedState = true;
            checkedState = checked ? Qt.Checked : Qt.Unchecked;
            __ignoreCheckedState = false;
        }
    }

    onCheckedStateChanged:
    {
        __ignoreChecked = true;
        if (checkedState === Qt.PartiallyChecked)
        {
            partiallyCheckedEnabled = true;
            checked = false;
        }
        else if (!__ignoreCheckedState)
        {
            checked = checkedState === Qt.Checked;
        }
        __ignoreChecked = false;
    }

    onPartiallyCheckedEnabledChanged:
    {
        if (exclusiveGroup && partiallyCheckedEnabled)
        {
            console.warn("Cannot have partially checked boxes in an ExclusiveGroup.");
        }
    }

    onExclusiveGroupChanged:
    {
        if (exclusiveGroup && partiallyCheckedEnabled)
        {
            console.warn("Cannot have partially checked boxes in an ExclusiveGroup.");
        }
    }

    /*! \internal */
    function __cycleCheckBoxStates()
    {
        if (!partiallyCheckedEnabled)
        {
            checked = !checked;
        }
        else
        {
            switch (checkedState)
            {
                case Qt.Unchecked: checkedState = Qt.Checked; break;
                case Qt.Checked: checkedState = Qt.PartiallyChecked; break;
                case Qt.PartiallyChecked: checkedState = Qt.Unchecked; break;
            }
        }
    }
}
