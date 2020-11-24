# change directory to project root
cd ../../../

# set convenience variables
JUCEDir=/Applications/JUCE
ProjucerPath="$JUCEDir"/Projucer.app/Contents/MacOS/Projucer
JucerProjectPath=SoundscapeBridgeApp.jucer
XCodeProjectPath=Builds/MacOSX/SoundscapeBridgeApp.xcodeproj
AppBundlePath=Builds/MacOSX/build/Release
CreateDmgPath=submodules/create-dmg/create-dmg
DmgTargetPath=SoundscapeBridgeApp.dmg

# export projucer project
"$ProjucerPath" --resave "$JucerProjectPath"

# Start the build
xcodebuild -project "$XCodeProjectPath" -configuration Release -jobs 8

# create project disk image
test -f "$DmgTargetPath" && rm "$DmgTargetPath"
ln -s /Applications "$AppBundlePath"/Applications
"$CreateDmgPath" --window-size 410 240 "$DmgTargetPath" "$AppBundlePath"