$CERTIFICATE_OWNER = "Open Source Developer, Egor Demidov"
$TIMESTAMP_ADDRESS = "http://time.certum.pl"
$ALGORITHM = "sha256"
$EXE_FILE = "cmake-build-release/soot_dem_gui.exe"
$ARCHIVE_FILE = "cmake-build-release/soot_dem_gui-windows-x86_64.zip"

signtool sign /n $CERTIFICATE_OWNER /t $TIMESTAMP_ADDRESS /fd $ALGORITHM /v $EXE_FILE

signtool verify /pa $EXE_FILE

$compress = @{
  Path = $EXE_FILE, "dist\examples", "dist\LICENSE.txt"
  CompressionLevel = "Fastest"
  DestinationPath = $ARCHIVE_FILE
}
Compress-Archive @compress
