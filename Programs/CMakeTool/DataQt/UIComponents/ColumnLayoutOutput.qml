import QtQuick 2.6
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.0
import Cpp.Utils 1.0

GroupBox {
    id: wrapper
	flat: true
    style: GroupBoxStyle {
        padding.left: 0 
		padding.right: 0
		padding.top: 0
		
    }

    property alias outputText: textField_output.text
    property bool outputComplete;
    signal cmakeWillBeLaunched();
    signal cmakeWasLaunched();
    signal buildStarted();
	Layout.fillWidth: true;
	Layout.fillHeight: true;
	function loadState(state) {
		if(state == undefined) {
			return;
		}
        checkBox_clean.checked = state.needClean;
		radioButton_openInIDE.checked = state.openInIDE;
		radioButton_buildDebug.checked = state.buildDebug;
		radioButton_buildRelease.checked = state.buildRelease;
		groupBox_postConfigure.checked = state.postConfigureEnabled;
    }

    function saveState() {
        var state = {};
        state.needClean = checkBox_clean.checked;
		state.openInIDE = radioButton_openInIDE.checked;
		state.buildDebug = radioButton_buildDebug.checked;
		state.buildRelease = radioButton_buildRelease.checked;
		state.postConfigureEnabled = groupBox_postConfigure.checked
        //make a deep copy
        return JSON.parse(JSON.stringify(state));
    }
	PlatformHelper {
		id: platformHelper
	}
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
			Layout.fillHeight: true
        }
		GroupBox {
			title: qsTr("pre-configure actions");
			Layout.fillWidth: true
			RowLayout {
				Layout.fillWidth: true
				CheckBox {
					id: checkBox_clean
					text: qsTr("clean build folder");
				}
			}
		}
		GroupBox {
			id: groupBox_postConfigure
			title: qsTr("post-configure actions");
			Layout.fillWidth: true
			checkable: true
			RowLayout {
				enabled: groupBox_postConfigure.checked
				id: rowLayout
				Layout.fillWidth: true
				ExclusiveGroup {
					id: exclusiveGroup_postConfigure
				}
				RadioButton {
					id: radioButton_openInIDE
					exclusiveGroup: exclusiveGroup_postConfigure
					text: qsTr("open in ") + (platformHelper.CurrentPlatform() == PlatformHelper.Windows ? "Visual Studio" : "XCode") ;
				}
				RadioButton {
					id: radioButton_buildDebug
					exclusiveGroup: exclusiveGroup_postConfigure
					text: qsTr("build debug");
				}
				RadioButton {
					id: radioButton_buildRelease
					exclusiveGroup: exclusiveGroup_postConfigure
					text: qsTr("build release");
				}
			}
		}

		RowLayout {
			Layout.fillWidth: true
            Button {
                id: button_runCmake
				Layout.fillWidth: true
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
			Layout.fillWidth: true
			Button {
                id: button_runBuildDebug
                text: qsTr("build debug")
                enabled: !processWrapper.running
				anchors.top: openProjectButton.top
				anchors.bottom: openProjectButton.bottom
                onClicked: {
                    buildStarted()
                    var buildPath = fileSystemHelper.NormalizePath(rowLayout_buildFolder.path)
                    var cmakePath = fileSystemHelper.NormalizePath(rowLayout_cmakeFolder.path)
                    processWrapper.LaunchCmake(cmakePath + " --build " + buildPath + " --config Debug", false, "")
                }
            }
            Button {
                id: button_runBuildRelease
                text: qsTr("build release")
                enabled: !processWrapper.running
				height: openProjectButton.height
				anchors.top: openProjectButton.top
				anchors.bottom: openProjectButton.bottom
                onClicked: {
                    buildStarted()
                    var buildPath = fileSystemHelper.NormalizePath(rowLayout_buildFolder.path)
                    var cmakePath = fileSystemHelper.NormalizePath(rowLayout_cmakeFolder.path)
                    processWrapper.LaunchCmake(cmakePath + " --build " + buildPath + " --config Release", false, "")
                }
            }
			Item {
				Layout.fillWidth: true
			}
            Button {
                id: openProjectButton
                iconSource: "qrc:///Icons/" + (platformHelper.CurrentPlatform() == PlatformHelper.Windows ? "msvs.ico" : "xcode.png")
                tooltip: qsTr("open project file")
                enabled: rowLayout_buildFolder.path.length !== 0
                text: qsTr("open project file");
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
