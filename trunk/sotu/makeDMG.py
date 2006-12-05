#!/usr/bin/python
import os
import shutil
import re
import time
import sys

#print "Build started at " + time.strftime("%Y%m%d %H:%M:%S")

if len(sys.argv)==2:
	VERSION=sys.argv[1]
else:
	VERSION=time.strftime("%Y%m%d%H%M%S")
SANDBOX="/Users/fbecker/sandbox"
FRAME_ABS="CriticalMass.app/Contents/Frameworks/"
FRAME_REL="@executable_path/../Frameworks/"

def UpdateBinary( binaryName):
	r = re.match(".*/([^/]*dylib).*", binaryName)
	if r != None:
		libName = r.group(1)
		if os.path.isfile( FRAME_ABS + libName):
			return

		shutil.copy(SANDBOX + "/lib/" + libName, FRAME_ABS)

		os.system("install_name_tool -id " + FRAME_REL + libName + " " + FRAME_ABS + libName)

	print "Updating " + binaryName
	cmd="otool -L " + binaryName
	handle = os.popen(cmd, 'r', 1)
	for line in handle:
		#print line,
		r = re.match("\s*("+SANDBOX+".*dylib).*", line);
		if r != None:
			library = r.group(1)
			r = re.match(".*/([^/]*dylib).*", library)
			if r != None:
				libName = r.group(1)
				os.system("install_name_tool -change " + library + " " + FRAME_REL + libName + " " + binaryName)
				UpdateBinary( FRAME_ABS + libName)
	handle.close()

def ReplaceVersion( textFileName, outFileName):
#	print "Replacing __NO_VERSION__ string in " + textFileName
	cmd="cat " + textFileName
	handle = os.popen(cmd, 'r', 1)
	outFile = file(outFileName, "w+")
	for line in handle:
		newLine = re.sub( "__NO_VERSION__", VERSION, line)
		outFile.write(newLine);

	handle.close()

if os.path.isdir("CriticalMass.app"):
	shutil.rmtree("CriticalMass.app")

os.makedirs("CriticalMass.app/Contents/MacOS")
os.makedirs("CriticalMass.app/Contents/Resources")
os.makedirs("CriticalMass.app/Contents/Frameworks")

#shutil.copy("game/Info.plist", "CriticalMass.app/Contents/")
ReplaceVersion( "game/Info.plist", "CriticalMass.app/Contents/Info.plist");

shutil.copy("game/resource.dat", "CriticalMass.app/Contents/Resources")
shutil.copy("game/critter.icns", "CriticalMass.app/Contents/Resources")
shutil.copy("data/music/lg-criti.xm", "CriticalMass.app/Contents/Resources")

shutil.copy("game/critter", "CriticalMass.app/Contents/MacOS")

UpdateBinary( "CriticalMass.app/Contents/MacOS/critter")

critterDir="CriticalMass-" + VERSION
critterDMG=critterDir + ".dmg"
if os.path.exists( critterDMG):
	os.remove( critterDMG)

if os.path.exists( critterDir):
	shutil.rmtree( critterDir)

os.makedirs( critterDir)
shutil.move( "CriticalMass.app", critterDir+"/CriticalMass.app")
shutil.copy( "Readme.html", critterDir)
shutil.copy( "COPYING", critterDir)
shutil.copy( "GoConfigDirectory.command", critterDir)

print "Building DMG..."
os.system( "hdiutil create -quiet -ov -srcfolder " + critterDir + " -format UDZO " + critterDMG)
print "Created shiny DMG: " + critterDMG

#shutil.rmtree( critterDir)
#print "Build completed at " + time.strftime("%Y%m%d %H:%M:%S")
