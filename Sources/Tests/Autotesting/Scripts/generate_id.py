#!/usr/bin/env python2.6
#
#  autotesting_generate_id.py
#  DAVA SDK
#
#  Created by Dmitry Shpakov on 6/13/12.
#  Copyright (c) 2008 DAVA Consulting, LLC. All rights reserved.
#

import os;
import sys;
import os.path;
import string;
#import random;
import time;
import datetime;

print "generate id"

arguments = sys.argv[1:]

if len(arguments) != 1:
    print 'Usage: ./generate_id.py ProjectName'
    exit(1)

projectName = arguments[0]


currentDate = datetime.datetime.today()
currentYear = currentDate.year
currentMonth = currentDate.month
currentDay = currentDate.day
currentDateStr = str(currentYear) + "{0:02d}".format(currentMonth) + "{0:02d}".format(currentDay)
print "testsDate: " + currentDateStr

projectFolder = os.getcwd()

idFilePath = os.path.realpath(projectFolder + "/Data/Autotesting/id.txt")
print "write to file " + idFilePath
file=open(idFilePath,'w')
file.write(currentDateStr)
file.write("\n")
file.write(projectName)
file.close()
   
print "generated id"