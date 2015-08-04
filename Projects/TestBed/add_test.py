#!/usr/bin/env python

import os
import sys
import shutil

def updateGameCore(className):
    """Updates GameCore.cpp to add new test"""

    gc_read = "Classes/Infrastructure/GameCore.cpp"
    gc_write = "Classes/Infrastructure/GameCore.cpp.new"

    with open(gc_write, 'w') as outfile:
        with open(gc_read, 'r') as infile:
            for line in infile:
                if line.startswith("//$UNITTEST_INCLUDE"):
                    outfile.write('#include "Tests/'+className+'.h"\n')
                if line.startswith("//$UNITTEST_CTOR"):
                    outfile.write('    new '+className+'();\n')
                outfile.write(line)

    shutil.move(gc_write, gc_read)

def checkTest(className):
    """Checks that test's header and source files exist"""

    h_write = "Classes/Tests/"+className+".h"
    cpp_write = "Classes/Tests/"+className+".cpp"

    if os.path.exists(h_write):
        print "Header with name %s already exists!" % h_write
        return True
    if os.path.exists(cpp_write):
        print "Source with name %s already exists!" % cpp_write
        return False

    return True

def createHeader(className):
    """Create test's header file from template"""

    h_read = "Classes/Infrastructure/TestTemplate.h.template"
    h_write = "Classes/Tests/"+className+".h"

    with open(h_write, 'w') as outfile:
        with open(h_read, 'r') as infile:
            data = infile.read()
            data = data.replace("$UNITTEST_GUARD_IFDEF$", className.upper())
            data = data.replace("$UNITTEST_CLASSNAME$", className)
            outfile.write(data)

def createSource(className):
    """Create test's source file from template"""

    cpp_read = "Classes/Infrastructure/TestTemplate.cpp.template"
    cpp_write = "Classes/Tests/"+className+".cpp"

    with open(cpp_write, 'w') as outfile:
        with open(cpp_read, 'r') as infile:
            data = infile.read()
            data = data.replace("$UNITTEST_CLASSNAME$", className)
            outfile.write(data)


if __name__ == "__main__":
    """Stand-alone run"""

    if len(sys.argv) != 2:
        print """usage: add_test.py TestClassName"""
        quit()

    test_class_name = sys.argv[1]

    if not checkTest(test_class_name):
        quit()

    createHeader(test_class_name)
    createSource(test_class_name)
    updateGameCore(test_class_name)
    print "Complete"

