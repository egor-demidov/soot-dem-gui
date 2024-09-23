#!/bin/bash

# Add linuxdeployqt to path
export PATH="/home/egor/Downloads/linuxdeployqt/cmake-build/tools/linuxdeployqt:$PATH"
# Add appimagetool to path
export PATH="/home/egor/Downloads/appimagetool/cmake-build/src:$PATH"

EXE_PATH="soot_dem_gui"

cd cmake-build-release
cp -R -u -p ../soot_dem_gui.desktop .
cp -R -u -p ../icons/app.png .
linuxdeployqt $EXE_PATH -appimage
mv soot_dem_gui-*-x86_64.AppImage soot_dem_gui-linux-x86_64.AppImage
mkdir -p soot_dem_gui-linux-x86_64
mv soot_dem_gui-linux-x86_64.AppImage soot_dem_gui-linux-x86_64/
cp -R -u -p ../dist/* soot_dem_gui-linux-x86_64/
tar -czvf soot_dem_gui-linux-x86_64.tar.gz soot_dem_gui-linux-x86_64/
