# we are in Resources/Deployment/macOS/ -> change directory to project root
cd ../../../

# set external variables
CodeSignCertName=$1 # e.g. "Developer ID Application: SomeIdentifier"
NotarizationUser=$2 # appleid to use for notarization
AppBundleId=$3 # app bundle id like "com.SomeIdentifier.SpaConBridge"

echo "Using certificate $CodeSignCertName for codesigning."
echo "Using AppleId $NotarizationUser and bundle name $AppBundleId for notarization."

# set convenience variables
AppBundlePath=Builds/MacOSX/build/Release
CreateDmgPath=submodules/create-dmg/create-dmg
DmgTargetPath=SpaConBridge.dmg

# build the project
cd Resources/Deployment/macOS
./build_project.sh
cd ../../../

# create project disk image
test -f "$DmgTargetPath" && rm "$DmgTargetPath"
ln -s /Applications "$AppBundlePath"/Applications
"$CreateDmgPath" --window-size 410 240 "$DmgTargetPath" "$AppBundlePath"

# codesign the disk image
codesign --force --sign "$CodesignCertName" "$DmgTargetPath"

# trigger notarization
# (assumes that a keychain item "SCB_DMG_Notarization_PW" exists, created e.g. with "xcrun altool --store-password-in-keychain-item "SCB_DMGNotarization_PW" -u "abc" -p "def"")
xcrun altool --notarize-app --primary-bundle-id "$AppBundleId" --username "$NotarizationUser" --password "@keychain:SCB_DMGNotarization_PW" --file "$DmgTargetPath"

# notarization result can be queried as follows
# xcrun altool --notarization-info 2EFE2717-52EF-43A5-96DC-0797E4CA1041 -u "$NotarizationUser"
