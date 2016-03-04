function createOutput() {
    if(!mainObject) {
        return qsTr("create output called before programm being loaded")
    }

	outputComplete = true;
    var outputText = "";
    if(mainObject["currentPlatform"] === undefined) {
        outputComplete = false;
        return qsTr("current platform must be specified!");
    }
    var index = mainObject["currentPlatform"];
    var platformObject = mainObject["platforms"][index];
    outputText += platformObject.value;
    outputText += " -B" + rowLayout_buildFolder.path;

    var substrings = [];
    var defaults = platformObject.defaults;
    var localOptionsObject = mainObject["currentOptions"];
    if(defaults && Array.isArray(defaults)) {
        if(defaults.length > localOptionsObject.count) {
            errorDialog.informativeText = qsTr("Internal error: local options size less than default substrings size!");
            errorDialog.open();
            return "";
        }
        for(var i = 0, length = defaults.length; i < length; ++i) {
            substrings[i] = defaults[i];
        }

        for(var i = 0, length = localOptionsObject.length; i < length; ++i) {
            var localOption = localOptionsObject[i];
            substrings[localOption.index] = localOption.value;
        }
    }
    for(var i = 0, length = substrings.length; i < length; ++i) {
        outputText = outputText.arg(substrings[i])
    }


    if(outputText.indexOf("$CMAKE_PATH") !== -1) {
        var cmakePath = rowLayout_cmakeFolder.path;
        if(cmakePath.length === 0 || !fileSystemHelper.IsFileExists(cmakePath)) {
            outputComplete = false;
            return qsTr("cmake path required");
        } else {
            outputText = outputText.replace("$CMAKE_PATH", cmakePath);
        }
    }

    if(outputText.indexOf("$BUILD_FOLDER_PATH") !== -1) {
        var buildFolder = rowLayout_buildFolder.path;
        if(buildFolder.length === 0 || !fileSystemHelper.IsDirExists(buildFolder)) {
            outputComplete = false;
            return qsTr("build folder path required")
        } else {
            if(buildFolder[buildFolder.length - 1] === '/') {
                buildFolder = buildFolder.slice(0, -1);
            }
            outputText = outputText.replace("$BUILD_FOLDER_PATH", buildFolder)

        }
    }
    if(outputText.indexOf("$DAVA_FRAMEWORK_PATH") !== -1) {
        var davaFolder = rowLayout_davaFolder.path;
        if(davaFolder.length === 0 || !fileSystemHelper.IsDirExists(davaFolder)) {
            outputComplete = false;
            return qsTr("DAVA folder path required");
        } else {
            if(davaFolder[davaFolder.length - 1] === '/') {
                davaFolder = davaFolder.slice(0, -1);
            }
            outputText = outputText.replace("$DAVA_FRAMEWORK_PATH", davaFolder)
        }
        return outputText;
    }
    return outputText;
}
