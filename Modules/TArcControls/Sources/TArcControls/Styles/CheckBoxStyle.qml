import QtQuick 2.4
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.3
import QtQuick.Window 2.2
import QtQuick.Controls.Private 1.0

Style {
    id: checkboxStyle
    objectName: "WGCheckBoxStyle"
    WGComponent { type: "WGCheckBoxStyle" }

    /*! The \l CheckBox this style is attached to. */
    readonly property WGCheckBase control: __control

    /*! This defines the text label. */
    property Component label: Item {
        implicitWidth: text.implicitWidth + 2
        implicitHeight: text.implicitHeight
        baselineOffset: text.baselineOffset
        Rectangle {
            anchors.fill: text
            anchors.margins: -1
            anchors.leftMargin: -3
            anchors.rightMargin: -3
            visible: control.activeFocus
            height: 6
            radius: 3
            color: "transparent"
            border.color: palette.lighterShade
        }
        Text {
            id: text
            text: StyleHelpers.stylizeMnemonics(control.text)
            anchors.centerIn: parent
            color: SystemPaletteSingleton.text(control.enabled)
            renderType: globalSettings.wgNativeRendering ? Text.NativeRendering : Text.QtRendering
        }
    }
    /*! The background under indicator and label. */
    property Component background

    /*! The spacing between indicator and label. */
    property int spacing: Math.round(TextSingleton.implicitHeight/4)

    /*! This defines the indicator button. */
    property Component indicator:  Item {
        implicitWidth: Math.round(TextSingleton.implicitHeight)
        height: width
        Rectangle {
            anchors.fill: parent
            anchors.bottomMargin: -1
            color: "#44ffffff"
            radius: baserect.radius
        }
        Rectangle {
            id: baserect
            gradient: Gradient {
                GradientStop {color: "#eee" ; position: 0}
                GradientStop {color: control.pressed ? "#eee" : "#fff" ; position: 0.1}
                GradientStop {color: "#fff" ; position: 1}
            }
            radius: TextSingleton.implicitHeight * 0.16
            anchors.fill: parent
            border.color: control.activeFocus ? palette.lighterShade : "#999"
        }

        Image {
            source: "../images/check.png"
            opacity: control.checkedState === Qt.Checked ? control.enabled ? 1 : 0.5 : 0
            anchors.centerIn: parent
            anchors.verticalCenterOffset: 1
            Behavior on opacity {NumberAnimation {duration: 80}}
        }

        Rectangle {
            anchors.fill: parent
            anchors.margins: Math.round(baserect.radius)
            antialiasing: true
            gradient: Gradient {
                GradientStop {color: control.pressed ? "#555" : "#999" ; position: 0}
                GradientStop {color: "#555" ; position: 1}
            }
            radius: baserect.radius - 1
            anchors.centerIn: parent
            anchors.alignWhenCentered: true
            border.color: "#222"
            Behavior on opacity {NumberAnimation {duration: 80}}
            opacity: control.checkedState === Qt.PartiallyChecked ? control.enabled ? 1 : 0.5 : 0
        }
    }

    /*! \internal */
    property Component panel: Item {
        implicitWidth: Math.max(backgroundLoader.implicitWidth, row.implicitWidth + padding.left + padding.right)
        implicitHeight: Math.max(backgroundLoader.implicitHeight, labelLoader.implicitHeight + padding.top + padding.bottom,indicatorLoader.implicitHeight + padding.top + padding.bottom)
        baselineOffset: labelLoader.item ? padding.top + labelLoader.item.baselineOffset : 0

        Loader {
            id: backgroundLoader
            sourceComponent: background
            anchors.fill: parent
        }
        RowLayout {
            id: row
            anchors.fill: parent

            anchors.leftMargin: padding.left
            anchors.rightMargin: padding.right
            anchors.topMargin: padding.top
            anchors.bottomMargin: padding.bottom

            spacing: checkboxStyle.spacing
            Loader {
                id: indicatorLoader
                sourceComponent: indicator
                anchors.verticalCenter: parent.verticalCenter
            }
            Loader {
                id: labelLoader
                Layout.fillWidth: true
                sourceComponent: label
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }
}
