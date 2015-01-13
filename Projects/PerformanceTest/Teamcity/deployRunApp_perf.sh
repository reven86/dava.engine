# !/bin/bash
echo "copy PerformanceTest.app"
APP=PerformanceTest
DIR_APP="../DerivedData/TemplateProjectiPhone/Build/Products/Release-iphoneos"

if [ ! -d $DIR_APP/$APP.app ]; then
 echo "directory $DIR_APP/$APP.app doesn't exist"
 exit 1
fi
cp -Rf $DIR_APP/$APP.app "./"

echo "sign and create PerfomanceTest.ipa"
sh floatsign.sh $APP.app 4L7VSNH4R3 $APP.ipa
sh floatsign.sh $APP.app 4L7VSNH4R2 $DIR_APP/$APP.ipa

if [ ! -f $APP.ipa ]; then
  echo "$APP.ipa wasn't created"
  exit 1
fi


echo "deploy ipa on device"
./transporter_chief.rb $APP.ipa

echo "get udid device" 
UDID_device=`system_profiler SPUSBDataType | sed -n -e '/iPad/,/Serial/p' -e '/iPhone/,/Serial/p' -e '/iPod/,/Serial/p' | grep "Serial Number:" | awk -F ": " '{print $2}'`
echo "udid is ${UDID_device}"

echo "run app on device"
#instruments -w ${UDID_device} -t /Developer/Platforms/iPhoneOS.platform/Developer/Library/Instruments/PlugIns/AutomationInstrument.bundle/Contents/Resources/Automation.tracetemplate "$APP" -e UIASCRIPT testRun.js
instruments -w ${UDID_device}  -t $PATH_TO_AUTO_TEMPL/Automation.tracetemplate "$APP" -e UIASCRIPT testRun.js

#del temporary files
#rm -rf ./$APP.*
#rm -rf ./fruitstrap
#rm -rf ./instrumentscli*
