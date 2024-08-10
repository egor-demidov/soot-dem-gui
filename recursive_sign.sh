#!/bin/zsh

APP_DIR=build
APP_NAME="soot_dem_gui"
APP_BUNDLE="$APP_DIR/$APP_NAME.app"
APP_ARCHIVE="$APP_DIR/$APP_NAME"
APP_DMG="$APP_DIR/${APP_NAME}-macos-arm64"
DMG_NAME="$APP_NAME"
EXECUTABLE="$APP_BUNDLE/Contents/MacOS/soot_dem_gui"
IDENTITY="Developer ID Application: Egor Demidov (V8NHL8MF4D)"


echo "SIGNING RECURSIVELY..."

START_PATH="$(pwd)"
cd $APP_BUNDLE
for f in **/*
do
  [[ "$f" =~ ^.*.(dylib|.framework)$ ]] && codesign -f -o runtime -s $IDENTITY $f
done
cd $START_PATH
codesign  -f -o runtime -s $IDENTITY $EXECUTABLE
codesign -f -o runtime -s $IDENTITY $APP_BUNDLE

echo "SIGNED, VALIDATING SIGNATURE..."

codesign -vvv --deep --strict $APP_BUNDLE

echo "CREATING AN ARCHIVE..."

ditto -c -k --sequesterRsrc --keepParent $APP_BUNDLE $APP_ARCHIVE

echo "SUBMITTING FOR NOTARIZATION..."

xcrun notarytool submit $APP_ARCHIVE --wait --keychain-profile "notarytool-password"

echo "STAPLING NOTARIZATION TO THE SIGNATURE..."

xcrun stapler staple $APP_BUNDLE

echo "STAPLED, VERIFYING THE BUNDLE..."

spctl --verbose=4 --assess --type execute $APP_BUNDLE

#echo "CREATING RELEASE DMG..."

ditto $APP_BUNDLE "$APP_DIR/${DMG_NAME}_dir/$APP_NAME.app"
ditto "dist/" "$APP_DIR/${DMG_NAME}_dir/"
hdiutil create -volname $DMG_NAME -srcfolder "$APP_DIR/${DMG_NAME}_dir" $APP_DMG

# TODO:
#/usr/bin/ditto -c -k --sequesterRsrc --keepParent "$APP_PATH" "$ZIP_PATH"

# Validation:
# spctl --verbose=4 --assess --type execute MyApp.app/

# To submit for notarization:
# xcrun notarytool submit soot-dem-gui.zip --wait --keychain-profile "notarytool-password"

# To download the log:
# xcrun notarytool log c8e30ff7-456c-4216-bf00-82870c2ff08b --keychain-profile "notarytool-password" developer_log.json

# To staple the ticket:
# xcrun stapler staple "Overnight TextEditor.app"

# Check history:
# xcrun notarytool history --keychain-profile "notarytool-password"
