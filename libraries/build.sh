#!/bin/bash
ARDUINOPATH="/mnt/c/fd_work/TuKuRutchExe/Arduino"
PWD_=`echo -n $PWD | sed -E "s/\/mnt\/c/C:/"`

#################
TARGETS=\
"lovyanGFX lovyanGFXc3 lovyanGFXc3u lovyanGFXs3 lovyanGFXs3u lovyanGFXpico "\
"QuadCrawlerAI SquareCam cameratch32 "\
"xiao32S3sense "\
"uno32 uno32s3u uno uno.pico "\
"DevkitCs3 DevkitC "\
"M5TimerCam M5CameraCar esp32camS3 esp32camS3u esp32cam unitCam testCam testCamS3 "\
"M5Unified M5UnifiedS3 "\
"M5CoreS3 "\
"QuadCrawler RemoconRobo "\
"GroveBeginnerKit "\

#################
S3TARGETS=\
"uno32 "\
"DevkitCs3 "\
"lovyanGFX "\
"M5TimerCam "\
"M5Unified "\
"QuadCrawlerAI SquareCam cameratch32 "\
"M5CoreS3 "\
"QuadCrawler RemoconRobo "\
"GroveBeginnerKit "\

function buildSRCs() {
  PARAMS=`node parseRobotJson.js $1 params`
  node parseRobotJson.js $1 jsonToCpp2         # src.ino生成

  echo $PARAMS > buildlog.txt
  $ARDUINOPATH/arduino_debug.exe $PARAMS --save-prefs
  if [ $? -ne 0 ]; then
    echo "error in $1"
    exit 1
  fi

  time $ARDUINOPATH/arduino_debug.exe --verify --verbose --preserve-temp-files $PWD_/$1/src/src.ino >> buildlog.txt
  if [ $? -ne 0 ]; then
    echo "error in $1"
    exit 1
  fi
  node parseRobotJson.js $1 copyBins
}

if [ $# -eq 0 ]; then
  echo "usage: build.sh <target> [COM1]"
  exit 1
fi

if [ $1 == "all" ]; then
  echo "TARGETS=$TARGETS"
  echo "S3TARGETS=$S3TARGETS"

  for target in $TARGETS; do
    echo ""
    echo "building $target..."
    buildSRCs $target
  done

  for target in $S3TARGETS; do
    echo ""
    echo "building $target..."
    node parseRobotJson.js $target jsonToJs
  done
else
  buildSRCs $1
  node parseRobotJson.js $1 jsonToJs
  if [ $# -ge 2 ]; then
    CMD=`node parseRobotJson.js $1 burnFW $2`
    echo $CMD
    $CMD
  fi
fi

# /mnt/c/fd_work/TuKuRutchExe/Arduino/arduino.exe
