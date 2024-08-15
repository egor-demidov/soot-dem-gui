#!/bin/zsh

TARGET_NAME=gui_design_soot_dem

rm -rf deploy
mkdir deploy
cd deploy
export CC=/opt/homebrew/opt/llvm/bin/clang
export CXX=/opt/homebrew/opt/llvm/bin/clang++
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
ninja install
cd $TARGET_NAME.app
for f in **/*
do
  [[ "$f" =~ ^.*.(dylib|.framework)$ ]] && codesign -f -s - $f
done
cd ../..
