#!/usr/bin/python
from __future__ import with_statement
from contextlib import closing
from zipfile import ZipFile, ZIP_DEFLATED
import sys
import os
import pymongo
import bson
import yaml
import datetime
import shutil
import platform;

import shlex
import subprocess

mapsDir = './DataSource/3d/';
formatParam = 'tegra'

executables = { "Darwin": 'Tools/ResEditor/dava.framework/Tools/ResourceEditor/ResourceEditorQt.app/Contents/MacOS/ResourceEditorQt',
"Windows": 'Tools/ResEditor/dava.framework/Tools/ResourceEditor/ResourceEditorQtVS2010.exe',
"Microsoft": 'Tools/ResEditor/dava.framework/Tools/ResourceEditor/ResourceEditorQtVS2010.exe' }

def zipdir(basedir, archivename):
    assert os.path.isdir(basedir)
    with closing(ZipFile(archivename, "w", ZIP_DEFLATED)) as z:
        for root, dirs, files in os.walk(basedir):
            #NOTE: ignore empty directories
            for fn in files:
                absfn = os.path.join(root, fn)
                zfn = absfn[len(basedir)+len(os.sep):] #XXX: relative path
                z.write(absfn, zfn)

def run_command(command):
    p = subprocess.Popen(command,
                         stdout=subprocess.PIPE,
                         stderr=subprocess.STDOUT)
    return iter(p.stdout.readline, b'')

def getZippedSize(sceneFile):
	pervDir = os.getcwd();
	os.chdir("../../../wot.blitz/");

    	currentDir = os.getcwd(); 
    	dataDir =  os.path.realpath(currentDir + "/../Data/3d/");
    	dataSourceDir = os.path.realpath(currentDir + "/DataSource/3d/");
    	index = dataSourceDir.find(':');
    	if ( index != -1):
        	dataSourceDir = dataSourceDir[index + 1:];

	outDir = os.getcwd() + '/export_process/';

	exportDir = os.path.realpath(outDir);
	index = exportDir.find(':')
    	if ( index != -1):
        	exportDir = exportDir[index + 1:];


	executable = executables[platform.system()];

	os.spawnv(os.P_WAIT, executable, [executable, '-sceneexporter', '-export', '-indir', dataSourceDir, '-outdir', exportDir, '-processfile', sceneFile, '-forceclose', '-format', formatParam]);

	zipdir(outDir[:-1], 'export_process.zip');
	zipSize = os.path.getsize('export_process.zip');

	shutil.rmtree(outDir[:-1]);
	os.remove('export_process.zip');
	os.chdir(pervDir);
	
	return zipSize;



arguments = sys.argv[1:]

if 0 == len(arguments) or 2 < len(arguments):
	print 'Usage: ./LevelPerformanceTestReport.py [Test ID] <Texture Format>'
	exit(1)

testID = arguments[0]

if 2 == len(arguments):
	formatParam = arguments[1]

def LogError(logfile, message):
	logfile.write('<br/><font color="red">')
	logfile.write(message)
	logfile.write('</font>\n')


stream = open('Data/Config/config.yaml', 'r')
config = yaml.load(stream)

report = open('report.html', 'w')
report.write('<!DOCTYPE html>\n')
report.write('<html>\n <head>\n')
report.write('<title> Level Performance Test Report, Test ID: ' + testID + '</title>\n')
report.write('</head>\n <body style="font-family: calibri;">\n')
report.write('<H1> Level Performance Test Report</H1>\nTest ID: ' + testID + '\n')

connection = None;
try:
	connection = pymongo.Connection("by2-buildmachine.wargaming.net", 27017)
except:
	connection = None

if None != connection:

	db = connection['LevelPerformanceTests']
	collection = db['LevelPerformanceTestsResult']
	
	currTest = collection.find_one({'_id': testID})
	if None != currTest:
		if 'DeviceDescription' in currTest:
			report.write('<H2> Device: ' + currTest['DeviceDescription'] + '</H2>\n')
		report.write('<H3> Date: ' + datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S") + '</H3></br>\n')
		
		
		report.write('<table border="3" cellspacing="2"><tr>\n')
		for var in config['list']:
			color = '#'
			for c in var[1]:
				color += '{0:02x}'.format(c)
			report.write('<td bgcolor="' + color + '" height="50" width="50"/>\n')
			
		report.write('</tr><tr>\n')
		
		colorsCount = len(config['list'])
		for i in range(0, colorsCount):
			if i < (colorsCount - 1):
				report.write('<td align="center">' + str(config['list'][i][0]) + ' - ' + str(config['list'][i+1][0]) + '</br>fps</td>\n')
			else:
				report.write('<td align="center">'  + str(config['list'][i][0]) + '+ </br>fps</td>\n')
			
		report.write('</tr></table></br>\n')
		
		levelNames = currTest.keys()
		levelNames.sort()
		for levelName in levelNames:
			if '_id' != levelName and 'globalIndex' != levelName and 'DeviceDescription' != levelName and 'ErrorLog' != levelName:
				report.write('<H2> Level: ' + levelName + ' </H2>\n')

				level = currTest[levelName]
				reportValues = level.keys()
				reportValues.sort()
				for reportValue in reportValues:
					if 'FPS average value' == reportValue:
						report.write('<b><i>' + reportValue + '</i>: ' + level[reportValue] + '</b><br/>\n')
					elif 'SceneFilePath' == reportValue:
						sceneFilePath = level[reportValue];
						pervDir = os.getcwd();
						os.chdir("../../../wot.blitz/");
						sceneFileSize = os.path.getsize(mapsDir + sceneFilePath)/(1024. * 1024.);
						os.chdir(pervDir);

						if sceneFileSize > 11. :
							report.write('<b style="color:Red"><i>SceneFileSize</i>: ' + ('%.2f' % (sceneFileSize)) + ' Mb (Limit: 11 Mb)</b><br/>\n')
						else:
							report.write('<b><i>SceneFileSize </i>: ' + ('%.2f' % (sceneFileSize)) + ' Mb (Limit: 11 Mb)</b><br/>\n')

						zippedSceneSize = getZippedSize(sceneFilePath)/(1024. * 1024.);
						report.write('<b><i>ZippedSceneSize</i>: ' + ('%.2f' % (zippedSceneSize)) + ' Mb</b><br/>\n')

					elif 'TextureFilesSize' == reportValue:
						report.write('<b><i>' + reportValue + '</i>: ' + level[reportValue] + '</b><br/>\n')
					elif 'TextureMemorySize' == reportValue:
						if 46. < float(level[reportValue].split()[0]):
							report.write('<b style="color:Red"><i>' + reportValue + '</i>: ' + level[reportValue] + ' (Limit: 46 Mb)</b><br/>\n')
						else:
							report.write('<b><i>' + reportValue + '</i>: ' + level[reportValue] + ' (Limit: 46 Mb)</b><br/>\n')

				if 'ResultImagePNG' in level:
					imageFile = open(levelName + '.png', 'wb')
					imageFile.write(level['ResultImagePNG'])
					report.write('<img src="./' + levelName + '.png"' + ' alt="'+ levelName +'"></br>\n')
		
		if 'ErrorLog' in currTest:
			errorsLogDb = currTest['ErrorLog']
			if len(errorsLogDb) > 0:
				LogError(report, "Errors:")
				for errorKey in errorsLogDb:
					if errorKey != '_id':
						LogError(report, errorsLogDb[errorKey])
		
	else:
		LogError(report, "There are no test with ID: " + testID)
		
#	collection.remove({"_id": testID})
	
else:
	LogError(report, "Can't connect to Database")
	
report.write('</body>')
report.close()
