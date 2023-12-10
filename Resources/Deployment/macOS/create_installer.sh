# we are in Resources/Deployment/macOS/ -> change directory to project root
cd ../../../

# set external variables
CodeSignCertName=$1 # e.g. "Developer ID Application: SomeIdentifier"

echo "Using certificate $CodeSignCertName for codesigning."

# set convenience variables
AppBundlePath=Builds/MacOSX/build/Release
CreateDmgPath=submodules/create-dmg/create-dmg
DmgTargetPath=SpaConBridge.dmg

# build the project and create dmg
cd Resources/Deployment/macOS
./build_project.sh
./create_diskimage.sh
cd ../../../

# codesign the disk image
codesign --force --sign "$CodesignCertName" "$DmgTargetPath"

# trigger notarization
# (assumes that credentials are stored to keychain, e.g. with xcrun notarytool store-credentials APP_NOTARIZATION_CREDENTIALS_PROFILE --apple-id APPLE_ID --team-id TEAM_ID)
xcrun notarytool submit --keychain-profile "APP_NOTARIZATION_CREDENTIALS_PROFILE" --wait "$DmgTargetPath"
# staple the notarization to the artifact for offline operation
xcrun stapler staple "$DmgTargetPath"