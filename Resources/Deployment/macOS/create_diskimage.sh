# we are in Resources/Deployment/macOS/ -> change directory to project root
cd ../../../

# set convenience variables
AppBundlePath=Builds/MacOSX/build/Release/SpaConBridge.app
ChangeLogPath=CHANGELOG.md
CreateDmgPath=submodules/create-dmg/create-dmg
LicensePath=LICENSE
IconSetSourcePng=SpaConBridge.png
VolIconPath=Resources/Images/Iconset.icns
DmgTargetPath=SpaConBridge.dmg
DmgContentsCollectionPath=ContentsPath

# create the icns from existing png
cd Resources/Images
chmod +x makeIconset.sh
./makeIconset.sh "$IconSetSourcePng"
cd ../../

# collect dmg contents
test -d "$DmgContentsCollectionPath" && rm -r "$DmgContentsCollectionPath"
mkdir "$DmgContentsCollectionPath"
cp -r "$AppBundlePath" "$DmgContentsCollectionPath"
cp -r "$ChangeLogPath" "$DmgContentsCollectionPath"
ln -s /Applications "$DmgContentsCollectionPath"/Applications

# create project disk image
test -f "$DmgTargetPath" && rm "$DmgTargetPath"
"$CreateDmgPath" --eula "$LicensePath" --window-size 565 245 --volicon "$VolIconPath" "$DmgTargetPath" "$DmgContentsCollectionPath"

# cleanup
test -d "$DmgContentsCollectionPath" && rm -r "$DmgContentsCollectionPath"
test -f "$VolIconPath" && rm "$VolIconPath"