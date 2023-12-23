#!/bin/bash
ARDUINO_PATH="/mnt/c/fd_work/Arduino"
ESP32_VER=`ls ${ARDUINO_PATH}/portable/packages/esp32/hardware/esp32/`

#################
TARGETS=\
"lovyanGFX lovyanGFX/c3 lovyanGFX/c3u lovyanGFX/s3 lovyanGFX/s3u lovyanGFX/pico "\
"uno32 uno32/s3u uno32/avr uno32/pico "\
"DevkitC DevkitC/s3 "\
"esp32cam esp32cam/M5TimerCam esp32cam/M5CameraCar esp32cam/s3 esp32cam/s3u esp32cam/unitCam esp32cam/unitCamS3 esp32cam/unitCamS3u esp32cam/testCam esp32cam/testCamS3 "\
"M5Unified M5Unified/s3 "\
"QuadCrawlerAI "\
"esp32camlcd/SquareCam esp32camlcd/xiao32S3sense "\
"M5CoreS3 "\
"QuadCrawler RemoconRobo "\
"other/GroveBeginnerKit "\

# other/cameratch32  - サーボが動作せず

#################
S3TARGETS=\
"lovyanGFX "\
"uno32 "\
"DevkitC "\
"esp32cam "\
"M5Unified "\
"QuadCrawlerAI "\
"esp32camlcd/SquareCam "\
"M5CoreS3 "\
"QuadCrawler RemoconRobo "\
"other/GroveBeginnerKit "\

# other/cameratch32  - サーボが動作せず

function buildSRCs() {
  PARAMS=`node parseRobotJson.js $1 params`
  node parseRobotJson.js $1 jsonToCpp2         # src.ino生成

  echo $PARAMS > buildlog.txt
  $ARDUINO_PATH/arduino_debug.exe $PARAMS --save-prefs
  if [ $? -ne 0 ]; then
    echo "error in $1"
    exit 1
  fi

  PWD_=`echo -n $PWD | sed -e "s@/mnt/c@C:@"`
  time $ARDUINO_PATH/arduino_debug.exe --verify --verbose --preserve-temp-files $PWD_/$1/src/src.ino >> buildlog.txt
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
    ARDUINO_PATH_=`echo $ARDUINO_PATH | sed -e "s@/mnt/c@@"`
    CMD=`node parseRobotJson.js $1 burnFW $2 | sed -e "s@ARDUINO_PATH@${ARDUINO_PATH_}@g" | sed -e "s@ESP32_VER@${ESP32_VER}@g"`
    echo $CMD
    $CMD
  fi
fi
