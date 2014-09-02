#!/usr/bin/python

import pymongo
import bson

import sys

report = open('testId', 'w')
report2 = open('Data/testId', 'w')
	
connection = None;
try:
	connection = pymongo.Connection("by2-buildmachine.wargaming.net", 27017)
except:
	connection = None

if None != connection:

	db = connection['LevelPerformanceTests']
	collection = db['LevelPerformanceTestsResult']
	
	currTest = collection.find_one({'_id': 'GlobalTestId'})
	if None != currTest:
		testId = currTest['LastTestId'] + 1;
		currTest['LastTestId'] = testId;
		collection.update({'_id': 'GlobalTestId'}, {'$set' : {'LastTestId' : testId}});
		report.write(str(testId));
		report2.write(str(testId));
	else:
		doc = {};
		doc['_id'] = 'GlobalTestId';
		doc['LastTestId'] = 1;
		collection.insert(doc);
		report.write(str(1));
		report2.write(str(1));
report.close()
