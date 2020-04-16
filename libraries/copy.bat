set DIRNAME_S=RemoconRobo

set DIRNAME=BTO_ArduinoIR
copy %DIRNAME_S%\robot.js %DIRNAME%\robot.js /Y
copy %DIRNAME_S%\robot_pcmode\robot_pcmode.ino.template %DIRNAME%\robot_pcmode\robot_pcmode.ino.template /Y

set DIRNAME=Buzzer
copy %DIRNAME_S%\robot.js %DIRNAME%\robot.js /Y
copy %DIRNAME_S%\robot_pcmode\robot_pcmode.ino.template %DIRNAME%\robot_pcmode\robot_pcmode.ino.template /Y

set DIRNAME=esp32
copy %DIRNAME_S%\robot.js %DIRNAME%\robot.js /Y
copy %DIRNAME_S%\robot_pcmode\robot_pcmode.ino.template %DIRNAME%\robot_pcmode\robot_pcmode.ino.template /Y

set DIRNAME=Koov
copy %DIRNAME_S%\robot.js %DIRNAME%\robot.js /Y
copy %DIRNAME_S%\robot_pcmode\robot_pcmode.ino.template %DIRNAME%\robot_pcmode\robot_pcmode.ino.template /Y

set DIRNAME=LiquidCrystal
copy %DIRNAME_S%\robot.js %DIRNAME%\robot.js /Y
copy %DIRNAME_S%\robot_pcmode\robot_pcmode.ino.template %DIRNAME%\robot_pcmode\robot_pcmode.ino.template /Y

set DIRNAME=m5stack
copy %DIRNAME_S%\robot.js %DIRNAME%\robot.js /Y
copy %DIRNAME_S%\robot_pcmode\robot_pcmode.ino.template %DIRNAME%\robot_pcmode\robot_pcmode.ino.template /Y

set DIRNAME=mBot
copy %DIRNAME_S%\robot.js %DIRNAME%\robot.js /Y
copy %DIRNAME_S%\robot_pcmode\robot_pcmode.ino.template %DIRNAME%\robot_pcmode\robot_pcmode.ino.template /Y

set DIRNAME=QuadCrawler
copy %DIRNAME_S%\robot.js %DIRNAME%\robot.js /Y
copy %DIRNAME_S%\robot_pcmode\robot_pcmode.ino.template %DIRNAME%\robot_pcmode\robot_pcmode.ino.template /Y

set DIRNAME=Spresense
copy %DIRNAME_S%\robot.js %DIRNAME%\robot.js /Y
copy %DIRNAME_S%\robot_pcmode\robot_pcmode.ino.template %DIRNAME%\robot_pcmode\robot_pcmode.ino.template /Y
pause
