#!/usr/bin/env python

import os
import sys
import shutil

if len(sys.argv) != 2:
	print """usage:
add_test.py TestClassName"""
	quit()

test_class_name = sys.argv[1]

#gamecore
gc_read = "Classes/Infrastructure/GameCore.cpp"
gc_write = "Classes/Infrastructure/GameCore.cpp.new"

with open(gc_write, 'w') as outfile:
    with open(gc_read, 'r') as infile:
    	for line in infile:
    		if 0 <= line.find("$UNITTEST_INCLUDE"):
    			outfile.write('#include "Tests/'+test_class_name+'.h"\n')
    		if 0 <= line.find("$UNITTEST_CTOR"):
    			outfile.write('    new '+test_class_name+'();\n')
    		outfile.write(line)

shutil.move(gc_write, gc_read)

#header
h_read = "Classes/Infrastructure/TestTemplate.h.template"
h_write = "Classes/Tests/"+test_class_name+".h"

with open(h_write, 'w') as outfile:
    with open(h_read, 'r') as infile:
    	data = infile.read()
    	data = data.replace("$UNITTEST_GUARD_IFDEF$", test_class_name.upper())
    	data = data.replace("$UNITTEST_CLASSNAME$", test_class_name)
    	outfile.write(data)

#source
cpp_read = "Classes/Infrastructure/TestTemplate.cpp.template"
cpp_write = "Classes/Tests/"+test_class_name+".cpp"

with open(cpp_write, 'w') as outfile:
    with open(cpp_read, 'r') as infile:
    	data = infile.read()
    	data = data.replace("$UNITTEST_CLASSNAME$", test_class_name)
    	outfile.write(data)
