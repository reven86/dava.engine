import QtQuick 2.2
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.0
import Cpp.Utils 1.0
import Qt.labs.settings 1.0

Item {
    id: wrapper
    property alias outputText: textField_output.text
    Layout.minimumHeight: label.height +  textField_output.height + rowLayout.height + rowLayout_output.spacing * 2
    property var outputComplete;
    ColumnLayout {
        id: rowLayout_output
        anchors.fill: parent

        Label {
            id: label
            text: qsTr("Output:")
        }

        TextArea {
            id: textField_output
            textColor: outputComplete ? "black" : "darkred"
            Layout.fillWidth: true
        }

        Settings {
            property alias mustClean: checkBox_clean.checked
        }

        RowLayout {
            id: rowLayout
            Button {
                id: button_runCmake
                iconSource: "qrc:///Icons/run.png"
                text: qsTr("run cmake")
                enabled: textField_output.text.length !== 0 && outputComplete
                onClicked: {
                    processWrapper.LaunchCmake(textField_output.text, checkBox_clean.checked, fileSystemHelper.NormalizePath(rowLayout_buildFolder.path))
                }
            }
            CheckBox {
                id: checkBox_clean
                text: qsTr("clean build folder");
            }
        }
        FileSystemHelper {
            id: fileSystemHelper
        }
    }
}
