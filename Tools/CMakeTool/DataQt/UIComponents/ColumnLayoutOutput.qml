import QtQuick 2.2
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.0
import Cpp.Utils 1.0

Item {
    id: wrapper
    property alias outputText: textField_output.text
    Layout.minimumHeight: label.height +  textField_output.height + rowLayout.minimumHeight + rowLayout_output.spacing * 3 + openProjectButton.height
    Layout.minimumWidth: rowLayout.minimumWidth
    property var outputComplete;
    signal cmakeLaunched();
    signal buildStarted();
    property alias needClean: checkBox_clean.checked

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

        RowLayout {
            id: rowLayout
            property int minimumHeight: Math.max(button_runCmake.height, checkBox_clean.height)
            property int minimumWidth: checkBox_clean.width + button_runCmake.width + button_runBuild.width + stopButton.width + spacing * 4
            CheckBox {
                id: checkBox_clean
                text: qsTr("clean build folder");
            }
            Button {
                id: button_runCmake
                iconSource: "qrc:///Icons/run.png"
                text: qsTr("run cmake")
                enabled: textField_output.text.length !== 0 && outputComplete && !processWrapper.running
                onClicked: {
                    cmakeLaunched();
                    processWrapper.LaunchCmake(textField_output.text, checkBox_clean.checked, fileSystemHelper.NormalizePath(rowLayout_buildFolder.path))
                }
            }
            Button {
                id: button_runBuild
                iconSource: "qrc:///Icons/build.png"
                text: qsTr("run build")
                enabled: !processWrapper.running
                onClicked: {
                    buildStarted()
                    var buildPath = fileSystemHelper.NormalizePath(rowLayout_buildFolder.path)
                    var cmakePath = fileSystemHelper.NormalizePath(rowLayout_cmakeFolder.path)
                    processWrapper.LaunchCmake(cmakePath + " --build " + buildPath, false, "")
                }
            }
            Button {
                id: stopButton
                iconSource: "qrc:///Icons/stop.png"
                tooltip: qsTr("stop process");
                enabled: processWrapper.running
                onClicked: {
                    processWrapper.KillProcess();
                }
            }
        }
        Button {
            id: openProjectButton
            iconSource: "qrc:///Icons/openfolder.png"
            tooltip: qsTr("open project file")
            enabled: rowLayout_buildFolder.pathIsValid
            text: qsTr("Open project file");
            onClicked:  {
                processWrapper.FindAndOpenProjectFile(rowLayout_buildFolder.path);
            }
        }


        FileSystemHelper {
            id: fileSystemHelper
        }
    }
}
