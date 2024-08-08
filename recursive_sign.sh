#!/bin/zsh

APP_BUNDLE=build/gui_design_soot_dem.app
IDENTITY="Developer ID Application: Egor Demidov (V8NHL8MF4D)"

START_PATH="$(pwd)"
cd $APP_BUNDLE
for f in **/*
do
  [[ "$f" =~ ^.*.(dylib|.framework)$ ]] && codesign -f -o runtime -s $IDENTITY $f
done
cd $START_PATH
codesign -f -o runtime -s $IDENTITY $APP_BUNDLE

echo "SIGNED, VALIDATING..."

codesign -vvv --deep --strict $APP_BUNDLE

# Validation:
# spctl --verbose=4 --assess --type execute MyApp.app/

# To submit for notarization:
# xcrun notarytool submit soot-dem-gui.zip --wait --keychain-profile "notarytool-password"

# To download the log:
# xcrun notarytool log ee932ac6-2f04-4504-8692-adce7a6eadac --keychain-profile "notarytool-password" developer_log.json

# To staple the ticket:
# xcrun stapler staple "Overnight TextEditor.app"

# Check history:
# xcrun notarytool history --keychain-profile "notarytool-password"
