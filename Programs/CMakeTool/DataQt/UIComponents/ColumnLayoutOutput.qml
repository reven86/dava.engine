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
    signal cmakeWillBeLaunched();
    signal cmakeWasLaunched();
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
            property int minimumHeight: Math.max(button_runCmake.height, checkBox_clean.height, button_runBuildDebug.height)
            property int minimumWidth: checkBox_clean.width + button_runCmake.width + button_runBuildDebug.width + button_runBuildRelease.width + stopButton.width + spacing * 5
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
                    cmakeWillBeLaunched();
                    processWrapper.LaunchCmake(textField_output.text, checkBox_clean.checked, fileSystemHelper.NormalizePath(rowLayout_buildFolder.path))
                    cmakeWasLaunched();
                }
            }
            Button {
                id: button_runBuildDebug
                iconSource: "qrc:///Icons/build.png"
                text: qsTr("build debug")
                enabled: !processWrapper.running
                onClicked: {
                    buildStarted()
                    var buildPath = fileSystemHelper.NormalizePath(rowLayout_buildFolder.path)
                    var cmakePath = fileSystemHelper.NormalizePath(rowLayout_cmakeFolder.path)
                    processWrapper.LaunchCmake(cmakePath + " --build " + buildPath + " --config Debug", false, "")
                }
            }
            Button {
                id: button_runBuildRelease
                iconSource: "qrc:///Icons/build.png"
                text: qsTr("build release")
                enabled: !processWrapper.running
                onClicked: {
                    buildStarted()
                    var buildPath = fileSystemHelper.NormalizePath(rowLayout_buildFolder.path)
                    var cmakePath = fileSystemHelper.NormalizePath(rowLayout_cmakeFolder.path)
                    processWrapper.LaunchCmake(cmakePath + " --build " + buildPath + " --config Release", false, "")
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

        RowLayout {
            id: rowLayout_openFolders
            property int minimumHeight: Math.max(openProjectButton.height, openBuildFolderButton.height)
            property int minimumWidth: openProjectButton.width + openBuildFolderButton.width + spacing * 2

            Button {
                id: openProjectButton
                iconSource: "qrc:///Icons/openfolder.png"
                tooltip: qsTr("open project file")
                enabled: rowLayout_buildFolder.path.length !== 0
                text: qsTr("Open project file");
                onClicked:  {
                    processWrapper.FindAndOpenProjectFile(rowLayout_buildFolder.path);
                }
            }
            Button {
                id: openBuildFolderButton
                iconSource: "qrc:///Icons/openfolder.png"
                tooltip: qsTr("open build folder")
                enabled: rowLayout_buildFolder.path.length !== 0
                text: qsTr("Open build folder");
                onClicked:  {
                    processWrapper.OpenFolderInExplorer(rowLayout_buildFolder.path);
                }
            }
        }


        FileSystemHelper {
            id: fileSystemHelper
        }
    }
}
