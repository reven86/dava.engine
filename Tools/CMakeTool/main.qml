import QtQuick 2.2
import QtQuick.Controls 1.3
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.0
import Cpp.Utils 1.0
import Qt.labs.settings 1.0

ApplicationWindow {
    id: applicationWindow
    visible: true
    width: 800
    height: 600

    Settings {
        id: settings
        property int mainWrapperWidth: 400
        property alias x: applicationWindow.x
        property alias y: applicationWindow.y
        property alias width: applicationWindow.width
        property alias height: applicationWindow.height
    }

    title: qsTr("CMake tool")
    ListModel {
        id: listModel_platforms
    }

    ListModel {
        id: listModel_localOptions
    }

    ListModel {
        id: listModel_globalOptions
    }

    ListModel {
        id: listModel_recentBuildsModel
    }

    ProcessWrapper {
        id: processWrapper;
    }

    ConfigStorage {
        id: configStorage
    }

    FileSystemHelper {
        id: fileSystemHelper;
    }

    property string platformsOptionName: "platforms";
    property string globalOptionsOptionName: "options";
    property bool outputComplete: false
    property var mainObject; //main JS object, contained in config file

    function syncConfig() {
        configStorage.SaveToConfigFile(JSON.stringify(mainObject, null, 4));
    }

    Timer {
        id: timer;
        interval: 10
        repeat: false
        onTriggered: updateOutputStringImpl();
    }

    function extractDavaPathFromPath(path) {
        var davaFolderName = "dava.framework";
        var index = path.indexOf(davaFolderName);
        if(index !== -1) {
            textField_DAVAFolder.text = path.substring(0, index + davaFolderName.length);
        }
    }

    function updateOutputString() {
        timer.start();
    }


    function updateOutputStringImpl() {
        outputComplete = true;
        var outputText = "cmake ";
        for(var i = 0, length = listModel_platforms.count; i < length; ++i) {
            var platformObject = mainObject[platformsOptionName][i];
            if(!platformObject.checked) {
                continue;
            }
            outputText += platformObject.value;
            var substrings = [];
            var defaults = platformObject.defaults;
            var localOptionsObject = platformObject.options;
            if(defaults && Array.isArray(defaults)) {
                if(defaults.length > localOptionsObject.count) {
                    errorDialog.informativeText = qsTr("Internal error: local options size less than default substrings size!");
                    errorDialog.open();
                    return;
                }
                for(var i = 0, length = defaults.length; i < length; ++i) {
                    substrings[i] = defaults[i];
                }

                for(var i = 0, length = localOptionsObject.length; i < length; ++i) {
                    var localOption = localOptionsObject[i];
                    if(localOption.checked) {
                        substrings[localOption["substring number"] - 1] = localOption.value;
                    }
                }
            }
            for(var i = 0, length = substrings.length; i < length; ++i) {
                outputText = outputText.arg(substrings[i])
            }
        }
        var globalOptionsObject = mainObject[globalOptionsOptionName];
        if(globalOptionsObject && Array.isArray(globalOptionsObject)) {
            for(var i = 0, length = globalOptionsObject.length; i < length; ++i) {
                var globalOption = globalOptionsObject[i];
                if(globalOption.checked) {
                    outputText += " " + globalOption.value
                }
            }
        }
        outputText += " -B " + comboBox_buildFolder.editText;

        if(outputText.indexOf("$BUILD_FOLDER_PATH") !== -1) {
            var buildFolder = comboBox_buildFolder.editText;
            if(buildFolder.length === 0 || !fileSystemHelper.isDirExists(buildFolder)) {
                outputText = qsTr("build folder path required")
                outputComplete = false;
            } else {
                if(buildFolder[buildFolder.length - 1] === '/') {
                    buildFolder = buildFolder.slice(0, -1);
                }
                outputText = outputText.replace("$BUILD_FOLDER_PATH", buildFolder)

            }
        }
        if(outputText.indexOf("$DAVA_FRAMEWORK_PATH") !== -1) {
            var davaFolder = textField_DAVAFolder.text;
            if(davaFolder.length === 0 || !fileSystemHelper.isDirExists(davaFolder)) {
                outputText = qsTr("DAVA folder path required")
                outputComplete = false;
            } else {
                if(davaFolder[davaFolder.length - 1] === '/') {
                    davaFolder = davaFolder.slice(0, -1);
                }
                outputText = outputText.replace("$DAVA_FRAMEWORK_PATH", davaFolder)
            }
        }
        textField_output.text = outputText;
    }

    Component.onCompleted: {
        try {
            var configuration = configStorage.GetJSONTextFromConfigFile()
            mainObject = JSON.parse(configuration);

            var arrayPlatforms = mainObject[platformsOptionName];
            if(arrayPlatforms && Array.isArray(arrayPlatforms)) {
                for(var i = 0, length = arrayPlatforms.length; i < length; ++i) {
                    listModel_platforms.append(arrayPlatforms[i]);
                }
            }

            var arrayGlobalOptions = mainObject[globalOptionsOptionName];
            if(arrayGlobalOptions && Array.isArray(arrayGlobalOptions)) {
                for(var i = 0, length = arrayGlobalOptions.length; i < length; ++i) {
                    listModel_globalOptions.append(arrayGlobalOptions[i]);
                }
            }
            var davaFolderPath = mainObject["davaFolder"];
            if(davaFolderPath) {
                textField_DAVAFolder.text = davaFolderPath
            } else if(applicationDirPath) {
                extractDavaPathFromPath(applicationDirPath);
            }

            var buildHistory = mainObject["buildFolderHistory"];
            if(buildHistory && Array.isArray(buildHistory)) {
                comboBox_buildFolder.blockRebuild = true;
                for(var i = 0, length = buildHistory.length; i < length; ++i) {
                    listModel_recentBuildsModel.append({text: buildHistory[i]});
                }
                comboBox_buildFolder.blockRebuild = false;
            }
        }
        catch(error) {
            errorDialog.informativeText = error.message;
            errorDialog.critical = true;
            errorDialog.open();
        }
    }

    MessageDialog {
        id: errorDialog;
        text: qsTr("error occurred!")
        icon: StandardIcon.Warning
        property bool critical: false
        onVisibleChanged: {
            if(!visible && critical) {
                Qt.quit()
            }
        }
    }

    FileDialog {
        id: fileDialog_buidFolder
        title: qsTr("select build folder")
        selectFolder: true
        onAccepted: {
            var url = fileDialog_buidFolder.fileUrls[0].toString()
            url = fileSystemHelper.resolveUrl(url);
            comboBox_buildFolder.editText = url;
        }
    }

    FileDialog {
        id: fileDialog_DAVAFolder
        title: qsTr("select DAVA folder");
        selectFolder: true
        onAccepted: {
            var url = fileDialog_DAVAFolder.fileUrls[0].toString()
            url = fileSystemHelper.resolveUrl(url);
            textField_DAVAFolder.text = url;
        }
    }
    SplitView {

        id: splitView;
        anchors.fill: parent
        anchors.margins: 10
        Item {
            width: settings.mainWrapperWidth;
            Component.onDestruction: {
                settings.mainWrapperWidth = width
            }

            ColumnLayout {
                id: columnLayout_main
                anchors.fill: parent
                anchors.rightMargin: 10
                RowLayout {
                    id: rowLayout_buldFolder

                    Label {
                        id: label_comboBox_buildFolder
                        text: qsTr("Build folder")
                    }

                    ComboBox {
                        id: comboBox_buildFolder
                        Layout.fillWidth: true
                        editable: true
                        model: listModel_recentBuildsModel
                        property bool blockRebuild: false;
                        onEditTextChanged: {
                            if(blockRebuild) {
                                return;
                            }

                            if(fileSystemHelper.isDirExists(editText)) {
                                var history = mainObject["buildFolderHistory"];
                                var curIndex = currentIndex
                                var newItem = editText;
                                if(history && Array.isArray(history)) {

                                    var maxLength = mainObject["maxHistoryCount"];
                                    if(!maxLength) {
                                        maxLength = 5;
                                    }
                                    var count = Math.min(history.length, maxLength);
                                    var found = false;
                                    for(var i = history.length; i >= 0 && !found; --i) {
                                        var currentHistory = history[i];
                                        if(currentHistory === newItem
                                                || currentHistory + "/" === newItem
                                                || newItem + "/" === currentHistory) {
                                            found = true;
                                            newItem = currentHistory;
                                            history.splice(i, 1);
                                        }
                                    }
                                    if(history.length < maxLength) {
                                        history.push("");
                                    }
                                    for(var i = history.length - 2; i >= 0; --i) {
                                        history[i + 1] = history[i];
                                    }
                                    history[0] = newItem;
                                    if(history.length > maxLength) {
                                        console.log("got max length", history.length);
                                        history = history.slice(0, maxLength);
                                        console.log(history.length);
                                    }
                                } else {
                                    history = [newItem];
                                }
                                mainObject["buildFolderHistory"] = history;
                                syncConfig();
                                blockRebuild = true;
                                if(!found) {
                                    listModel_recentBuildsModel.clear();
                                    for(var i = 0, length = history.length; i < length; ++i) {
                                        listModel_recentBuildsModel.append({text: history[i]});
                                    }
                                }
                                blockRebuild = false;
                            }


                            syncConfig();
                            extractDavaPathFromPath(editText);
                            updateOutputString();
                        }
                    }

                    Button {
                        id: button_getcomboBox_buildFolder
                        iconSource: "qrc:///Resources/openfolder.png"
                        onClicked: {
                            fileDialog_buidFolder.folder = comboBox_buildFolder.editText;
                            fileDialog_buidFolder.open();
                        }
                    }

                    Image {
                        id: image_comboBox_buildFolderStatus
                        width: height
                        height: rowLayout_buldFolder.height
                        source: "qrc:///Resources/" + (fileSystemHelper.isDirExists(comboBox_buildFolder.editText) ? "ok" : "error") + ".png"
                    }
                }

                RowLayout {
                    id: rowLayout_davaFolder
                    Label {
                        id: label_DAVAFolder
                        text: qsTr("DAVA folder")
                    }

                    TextField {
                        id: textField_DAVAFolder
                        Layout.fillWidth: true
                        placeholderText: qsTr("path to dava folder")
                        onTextChanged: {
                            mainObject["davaFolder"] = text
                            syncConfig();
                            updateOutputString()
                        }
                    }

                    Button {
                        id: button_getDAVAFolder
                        iconSource: "qrc:///Resources/openfolder.png"
                        onClicked: {
                            fileDialog_DAVAFolder.folder = textField_DAVAFolder.text;
                            fileDialog_DAVAFolder.open();
                        }
                    }

                    Image {
                        id: image_DAVAFolderStatus
                        width: height
                        height: rowLayout_davaFolder.height
                        source: "qrc:///Resources/" + (fileSystemHelper.isDirExists(textField_DAVAFolder.text) ? "ok" : "error") + ".png"
                    }
                }
                Item
                {
                    id: wrapperItem
                    Layout.fillHeight: true;
                    Layout.fillWidth: true;

                    RowLayout {
                        id: rowLayout_platformsAndOptions
                        height: parent.height / 3 * 2 -5
                        anchors.top: parent.top
                        anchors.topMargin: 0
                        anchors.right: parent.right
                        anchors.rightMargin: 0
                        anchors.left: parent.left
                        anchors.leftMargin: 0

                        ColumnLayout {
                            id: columnLayout_platforms
                            anchors.bottom: parent.bottom
                            anchors.bottomMargin: 0
                            anchors.top: parent.top
                            anchors.topMargin: 0
                            anchors.right: parent.horizontalCenter
                            anchors.rightMargin: 5
                            anchors.left: parent.left
                            anchors.leftMargin: 0

                            Label {
                                id: label_platforms
                                text: qsTr("Platforms")
                                anchors.left: parent.left
                                anchors.leftMargin: 0
                                anchors.top: parent.top
                                anchors.topMargin: 0
                            }

                            ListView {
                                id: listView_platforms
                                orientation: Qt.Vertical
                                boundsBehavior: Flickable.StopAtBounds
                                model: listModel_platforms
                                ExclusiveGroup {
                                    id: exclusiveGroup_platforms
                                }
                                delegate: RadioButton {
                                    text: model.name
                                    checked: model.checked
                                    exclusiveGroup: exclusiveGroup_platforms
                                    onCheckedChanged: {
                                        mainObject[platformsOptionName][index]["checked"] = checked;
                                        syncConfig();
                                        if(checked) {
                                            listModel_localOptions.clear();
                                            var localObject = mainObject[platformsOptionName][index];
                                            var options = localObject["options"];
                                            if(options && Array.isArray(options)) {
                                                for(var i = 0, length = options.length; i < length; ++i) {
                                                    options[i]["parentIndex"] = index
                                                    listModel_localOptions.append(options[i]);
                                                }
                                            }
                                        }
                                        updateOutputString();
                                    }
                                }

                                anchors.top: label_platforms.bottom
                                anchors.topMargin: 5
                                anchors.bottom: parent.bottom
                                anchors.bottomMargin: 0
                                anchors.left: parent.left
                                anchors.leftMargin: 0
                                anchors.right: parent.right
                                anchors.rightMargin: 0
                                spacing: 10
                            }
                        }

                        ColumnLayout {
                            id: columnLayout_localOptions
                            anchors.bottom: parent.bottom
                            anchors.bottomMargin: 0
                            anchors.top: parent.top
                            anchors.topMargin: 0
                            anchors.left: parent.horizontalCenter
                            anchors.leftMargin: 5
                            anchors.right: parent.right
                            anchors.rightMargin: 0

                            Label {
                                id: label_options
                                text: qsTr("Options")
                                anchors.left: parent.left
                                anchors.leftMargin: 0
                                anchors.top: parent.top
                                anchors.topMargin: 0
                            }

                            ListView {
                                id: listView_localOptions
                                    Component {
                                    id: loaderDelegate
                                    Loader {
                                        sourceComponent: type == "checkbox" ? checkboxDelegate : radioDelegate;
                                        property variant modelData: listModel_localOptions.get(model.index);
                                        property int index : model.index;
                                    }
                                }
                                ExclusiveGroup {
                                    id: exclusiveGroup_localOptions
                                }

                                Component {
                                    id: radioDelegate
                                    RadioButton {
                                        text: modelData ? modelData.name : ""
                                        checked: modelData ? modelData.checked : false
                                        onCheckedChanged: {
                                            if(!modelData) {
                                                return;
                                            }

                                            mainObject[platformsOptionName][modelData.parentIndex]["options"][index]["checked"] = checked;
                                            syncConfig();
                                            updateOutputString();
                                        }

                                        exclusiveGroup: exclusiveGroup_localOptions
                                    }
                                }
                                Component {
                                    id: checkboxDelegate
                                    CheckBox {
                                        text: modelData ? modelData.name : ""
                                        checked: modelData ? modelData.checked : ""
                                        onCheckedChanged: {
                                            if(!modelData) {
                                                return;
                                            }
                                            mainObject[platformsOptionName][modelData.parentIndex]["options"][index]["checked"] = checked;
                                            syncConfig();
                                            updateOutputString();
                                        }
                                    }
                                }
                                orientation: Qt.Vertical
                                boundsBehavior: Flickable.StopAtBounds
                                model: listModel_localOptions
                                delegate: loaderDelegate

                                anchors.top: label_options.bottom
                                anchors.topMargin: 5
                                anchors.bottom: parent.bottom
                                anchors.bottomMargin: 0
                                anchors.left: parent.left
                                anchors.leftMargin: 0
                                anchors.right: parent.right
                                anchors.rightMargin: 0
                                spacing: 10
                            }
                        }
                    }

                    ColumnLayout {
                        id: columnLayout_globalOptions
                        height: (parent.height / 3) - 5
                        anchors.left: parent.left
                        anchors.leftMargin: 0
                        anchors.right: parent.right
                        anchors.rightMargin: 0
                        anchors.bottom: parent.bottom
                        anchors.bottomMargin: 0

                        Label {
                            id: label_globalOptions
                            text: qsTr("global options")
                        }

                        GridView {
                            id: gridView_globalOptions
                            Layout.fillWidth: true;
                            boundsBehavior: Flickable.StopAtBounds
                            width: 100
                            height: 100
                            anchors.bottom: parent.bottom
                            anchors.bottomMargin: 0
                            anchors.top: label_globalOptions.bottom
                            anchors.topMargin: 5

                            delegate: CheckBox {
                                text: model.name
                                checked: model.checked
                                onCheckedChanged: {
                                    mainObject[globalOptionsOptionName][index]["checked"] = checked;
                                    syncConfig();
                                    updateOutputString();
                                }
                            }

                            model: listModel_globalOptions
                        }
                    }
                }

                RowLayout {
                    id: rowLayout_output
                    width: 395
                    height: 100

                    Label {
                        id: label_output
                        text: qsTr("Output:")
                    }

                    TextArea {
                        id: textField_output
                        Layout.fillWidth: true;
                        Layout.minimumHeight: contentHeight + 5
                        Layout.maximumHeight: contentHeight + 5
                        textColor: outputComplete ? "black" : "darkred"
                    }

                    Button {
                        id: button_runCmake
                        iconSource: "qrc:///Resources/run.png"
                        text: qsTr("run cmake")
                        enabled: textField_output.text.length !== 0 && outputComplete
                        onClicked: {
                            textArea_processText.append("");
                            processWrapper.LaunchCmake(textField_output.text)
                        }
                    }
                }
            }
        }
        TextArea {
            id: textArea_processText
            textFormat: TextEdit.RichText
            readOnly: true
            Connections {
                target: processWrapper
                onProcessStateChanged: textArea_processText.append("<font color=\"DarkGreen\">" + text + "</font>");
                onProcessErrorChanged: textArea_processText.append("<font color=\"DarkRed\">" + text + "</font>");
                onProcessStandardOutput: textArea_processText.append(text);
                onProcessStandardError: textArea_processText.append("<font color=\"DarkRed\">" + text + "</font>");
            }
        }
    }

}

