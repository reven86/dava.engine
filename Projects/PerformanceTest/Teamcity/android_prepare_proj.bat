cd ..\android
if NOT exist build.xml call android update project -t 2 -p .
if NOT exist ant.properties copy ..\Teamcity\android_keystore\*.* .\