function Component()
{
    // default constructor
}

Component.prototype.createOperations = function()
{
    // call default implementation to actually install applications!
    component.createOperations();

    // Create shortcut
    if (installer.value("os") === "win") {



        component.addOperation("CreateShortcut",
                               installer.value("TargetDir") + "/cmake.org.html",
                               installer.value("StartMenuDir") + "/CMake Web Site.lnk");

        component.addOperation("CreateShortcut",
                               installer.value("TargetDir") + "/uninstall.exe",
                               installer.value("StartMenuDir") + "/Uninstall.lnk");
    }
}
