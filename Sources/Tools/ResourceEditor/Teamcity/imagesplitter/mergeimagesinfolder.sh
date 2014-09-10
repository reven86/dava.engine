#!/bin/bash

# Use this command-file to merge 4 channel images to one
# call as: ./mergeimagesinfolder.sh sourceimages_folder
# as result generated image would be created in sourceimages_folder (merged.png)

ResourceEditorQt.app/Contents/MacOS/ResourceEditorQt -imagesplitter -merge -folder $1
