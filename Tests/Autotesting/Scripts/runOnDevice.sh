# !/bin/bash

# $1 - target product name

echo "get udid device"

UDID_device=`~/AIRSDK_Compiler/bin/adt -devices -platform iOS | grep "$2" | awk '{print $3}'`
HANDLE=`~/AIRSDK_Compiler/bin/adt -devices -platform iOS | grep "$2" | awk '{print $1}'`
echo "udid is ${UDID_device}, handle is ${HANDLE}"

echo "remove old build from device"
~/AIRSDK_Compiler/bin/adt -uninstallApp -device ${HANDLE} -platform iOS -appid com.davainc.${1}

echo "install app on device"
~/AIRSDK_Compiler/bin/adt -installApp -device ${HANDLE} -platform iOS -package ${1}.ipa

echo "run $1 on device"
instruments -w ${UDID_device} -t $PATH_TO_AUTO_TEMPL/Automation.tracetemplate "$1" -e UIASCRIPT testRun.js