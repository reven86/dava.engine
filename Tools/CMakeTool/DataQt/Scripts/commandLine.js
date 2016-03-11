function createOutput(configuration, fileSystemHelper, buildPath, cmakePath, davaPath) { 
    var outputText = ""; //pass by reference

    if(configuration["currentPlatform"] === undefined) {
        throw qsTr("current platform must be specified!");
    }
    var index = configuration["currentPlatform"];
    var platformObject = configuration["platforms"][index];
    outputText += platformObject.value;
    outputText += " -B" + buildPath;

    var substrings = [];
    var defaults = platformObject.defaults;
    var localOptionsObject = configuration["currentOptions"];
    if(defaults && Array.isArray(defaults)) {
        if(defaults.length > localOptionsObject.count) {
            throw qsTr("Internal error: local options size less than default substrings size!");
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
    
    var globalOptions = configuration.globalOptions;
    if(globalOptions && Array.isArray(globalOptions)) {
        for(var i = 0, length = globalOptions.length; i < length; ++i) {
            outputText += " " + globalOptions[i].value;
        }
    }
    
    if(outputText.indexOf("$CMAKE_PATH") !== -1) {
        if(cmakePath.length === 0 || !fileSystemHelper.IsFileExists(cmakePath)) {
            throw qsTr("cmake path required");
        } else {
            outputText = outputText.replace("$CMAKE_PATH", cmakePath);
        }
    }

    if(outputText.indexOf("$BUILD_FOLDER_PATH") !== -1) {
        if(buildPath.length === 0 || !fileSystemHelper.IsDirExists(buildPath)) {
            throw qsTr("build folder path required")
        } else {
            outputText = outputText.replace("$BUILD_FOLDER_PATH", buildPath)
        }
    }
    if(outputText.indexOf("$DAVA_FRAMEWORK_PATH") !== -1) {
        if(davaPath.length === 0 || !fileSystemHelper.IsDirExists(davaPath)) {
            throw qsTr("DAVA folder path required");
        } else {
            outputText = outputText.replace("$DAVA_FRAMEWORK_PATH", davaPath)
        }
    }
    
    var customOptions = configuration["customOpstions"];
    if(typeof customOptions === "string") {
        outputText += " " + customOptions;
    }
    return outputText;
}
