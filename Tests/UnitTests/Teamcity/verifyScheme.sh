# !/bin/bash

DIR_SCH="xcuserdata/$(whoami).xcuserdatad/xcschemes"

if [ -f "../TemplateProjectMacOS.xcodeproj/$DIR_SCH/TemplateProjectMacOS.xcscheme" ]; then
   echo "scheme for MacOS exists"
else
  mkdir -p "../TemplateProjectMacOS.xcodeproj/$DIR_SCH/"
  cp -Rf schemes/MacOS/* "../TemplateProjectMacOS.xcodeproj/$DIR_SCH" 
fi

if [ -f "../TemplateProjectiPhone.xcodeproj/$DIR_SCH/UnitTests.xcscheme" ]; then
   echo "scheme for iPhone exists"
else
  mkdir -p "../TemplateProjectiPhone.xcodeproj/$DIR_SCH/"
  cp -Rf schemes/iPhone/* "../TemplateProjectiPhone.xcodeproj/$DIR_SCH" 
fi
