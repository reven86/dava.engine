#!/usr/bin/python

import pymongo
import bson

import sys
arguments = sys.argv[1:]

if 0 == len(arguments) or 2 != len(arguments):
	print 'Usage: ./dbparser.py [CollectionName] [PlatformName]'
	exit(1)

collectionName = arguments[0]
platformName = arguments[1]
# platformName = 'MacOS'
# platformName = 'iPhone'
# platformName = 'Win32'
# platformName = 'Android'


def LogError(logfile, message):
	logfile.write('<br/><font color="red">')
	logfile.write(message)
	logfile.write('</font>')


def AddColumnCell(logfile, message):
	logfile.write('<td style="border: 1px solid gray; cellspacing: 1px; cellpadding: 1px;">')
	logfile.write(message)
	logfile.write('</td>')
	



report = open('report.html', 'w')
report.write('<!DOCTYPE html>')
report.write('<html> <head>')
report.write('<title> PerformanceTest Report </title>')
report.write('</head> <body>')

connection = None;
try:
	# connection = pymongo.Connection("localhost", 27017)
	connection = pymongo.Connection("by2-buildmachine.wargaming.net", 27017)
except:
	connection = None

if None != connection:
	#set database
	db = connection['PerformanceTest']
	report.write('<br/> DataBase: ' + db.name)

	#set collection
	collection = db[collectionName]
	report.write('<br/> Collection: ' + collection.name)

	report.write('<br/> Delta formula is: delta = (newValue - prevValue) / prevValue')

	#get cursor for collection
	platform = collection.find_one({'_id': platformName})
	if None != platform:
		screenNames = platform.keys()
		screenNames.sort()
		for screenName in screenNames:
			if '_id' != screenName and 'globalIndex' != screenName:
				report.write('<H1> Screen: ' + screenName + ' </H1>')

				screen = platform[screenName]
				testNames = screen.keys()
				testNames.sort()
				for testName in testNames:
					if '_id' != testName:
						
						report.write('<H3> Test: ' + testName + ' </H3>')
						
						report.write('<table style="border: 1px solid gray; cellspacing: 1px; cellpadding: 1px; border-collapse: collapse;">')

						report.write('<tr style="border: 1px solid gray; cellspacing: 1px; cellpadding: 1px;">')
						
						AddColumnCell(report, 'Run Time')
						AddColumnCell(report, 'Min Time')
						AddColumnCell(report, 'Max Time')
						AddColumnCell(report, 'Average Time')

						AddColumnCell(report, 'Min delta')
						AddColumnCell(report, 'Max delta')
						AddColumnCell(report, 'Average delta')

						AddColumnCell(report, 'Device')

						report.write('</tr>')

						test = screen[testName]
						testResultsNames = test.keys()
						testResultsNames.sort()
				
						prevAverage = -1;
						prevMin = -1
						prevMax = -1
						for testResultsName in testResultsNames:
							if "_id" != testResultsName:
								
								testData = test[testResultsName]

								averageValue = testData['Average']
								minValue = testData['MinTime']
								maxValue = testData['MaxTime']

								def PrintDelta(prevValue, newValue):
									if prevValue != -1:
								
										if prevValue != 0:
											delta = (float(newValue) - float(prevValue)) / float(prevValue)

											sign = ''
											color = 'black'
											if 0 < delta:
												sign = '+'
												color = 'red'
											elif delta < 0:
												color = 'green'

											report.write("<font size = '2' color = '" +color + "'>")
											report.write(sign+'%(percent).2f%%' % {'percent': delta*100})
											report.write('</font>')
									
										else:
											report.write('&nbsp')
									else:
										report.write('&nbsp')
								
								report.write('<tr style="border: 1px solid gray; cellspacing: 1px; cellpadding: 1px;">')
								
								AddColumnCell(report, testData['RunTime'])

								AddColumnCell(report, '%.2f'%float(minValue))
								AddColumnCell(report, '%.2f'%float(maxValue))
								AddColumnCell(report, '%.2f'%float(averageValue))

								report.write('<td style="border: 1px solid gray; cellspacing: 1px; cellpadding: 1px;">')
								PrintDelta(prevMin, minValue)
								report.write('</td>')

								report.write('<td style="border: 1px solid gray; cellspacing: 1px; cellpadding: 1px;">')
								PrintDelta(prevMax, maxValue)
								report.write('</td>')

								report.write('<td style="border: 1px solid gray; cellspacing: 1px; cellpadding: 1px;">')
								PrintDelta(prevAverage, averageValue)
								report.write('</td>')
								
								if testData['DeviceFamily'] == 0:
									AddColumnCell(report, 'Handset')
								elif testData['DeviceFamily'] == 1:
									AddColumnCell(report, 'Pad')
								elif testData['DeviceFamily'] == 2:
									AddColumnCell(report, 'Desktop')
								else:
									AddColumnCell(report, 'Unknown')
								
								report.write('</tr>')
								
								prevMin = minValue
								prevMax = maxValue
								prevAverage = averageValue

						report.write('</table>')
	else:
		LogError(report, "There are no tests for platform " + platformName + " at collection " + collectionName)
else:
	LogError(report, "Can't connect to Database")
	
report.write('</body>')
report.close()