# set external variables
IconSource=$1

mkdir IconSet.iconset
sips -z 16 16     "$IconSource" --out IconSet.iconset/icon_16x16.png
sips -z 32 32     "$IconSource" --out IconSet.iconset/icon_16x16@2x.png
sips -z 32 32     "$IconSource" --out IconSet.iconset/icon_32x32.png
sips -z 64 64     "$IconSource" --out IconSet.iconset/icon_32x32@2x.png
sips -z 128 128   "$IconSource" --out IconSet.iconset/icon_128x128.png
sips -z 256 256   "$IconSource" --out IconSet.iconset/icon_128x128@2x.png
sips -z 256 256   "$IconSource" --out IconSet.iconset/icon_256x256.png
sips -z 512 512   "$IconSource" --out IconSet.iconset/icon_256x256@2x.png
sips -z 512 512   "$IconSource" --out IconSet.iconset/icon_512x512.png
cp "$IconSource" IconSet.iconset/icon_512x512@2x.png
iconutil -c icns IconSet.iconset
rm -R IconSet.iconset