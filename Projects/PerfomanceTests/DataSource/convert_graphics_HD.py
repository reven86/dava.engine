#!/usr/bin/python
#
#  convert_graphics_HD.py
#  DAVA SDK
#
#  Created by Vitaliy Borodovsky on 10/27/08.
#  Copyright (c) 2008 DAVA Consulting, LLC. All rights reserved.
#

import os;
import sys;
import subprocess;

def do(): # it's function for external usage
	convertGraphicsHD = [sys.executable, 'convert_graphics.py']
	convertGraphicsHD.extend(sys.argv[1:])
	convertGraphicsHD.extend(['--dirs', 'Gfx2'])
	subprocess.call(convertGraphicsHD)

if __name__ == '__main__':
	do()