#!/usr/bin/env python

import sys
import subprocess

arguments = sys.argv[1:]

execfile('copy_framework_materials.py')

convertGraphics = [sys.executable, 'convert_graphics.py']

if len(arguments) == 0:
    convertGraphics.extend(['-gpu', 'PowerVR_iOS'])
    convert3d.extend(['PowerVR_iOS'])
    convert3dTanks.extend(['PowerVR_iOS'])
    convert3dFX.extend(['PowerVR_iOS'])
else:
    gpuParam = arguments[0]
    convertGraphics.extend(['-gpu', gpuParam])

subprocess.call(convertGraphics)
print "subprocess.call " + "[%s]" % ", ".join(map(str, convertGraphics))
