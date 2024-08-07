Generate .ico set from .png with imagemagick

magick "app.png" -define icon:auto-resize=16,24,32,48,64,72,96,128,256 "app.ico"

Generate macOS .icns with https://github.com/moinism/png2icns

png2icns app-mac.png -o app-mac.icns
