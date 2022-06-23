rmdir                    ..\..\Arduino\portable\sketchbook\libraries\analogRemote\  /S /Q
rmdir                    ..\..\Arduino\portable\sketchbook\libraries\TukurutchEsp\  /S /Q
rmdir                    ..\..\Arduino\portable\sketchbook\libraries\TukurutchEspCamera\  /S /Q
rmdir                    ..\..\Arduino\portable\sketchbook\libraries\TukurutchEspCamera2\  /S /Q

xcopy analogRemote       ..\..\Arduino\portable\sketchbook\libraries\analogRemote\  /S /Q /Y
xcopy TukurutchEsp       ..\..\Arduino\portable\sketchbook\libraries\TukurutchEsp\  /S /Q /Y
xcopy TukurutchEspCamera ..\..\Arduino\portable\sketchbook\libraries\TukurutchEspCamera\    /S /Q /Y
rem xcopy TukurutchEspCamera2 ..\..\Arduino\portable\sketchbook\libraries\TukurutchEspCamera2\    /S /Q /Y
pause
