#!/bin/bash
ARDUINO_PATH="/mnt/c/fd_work/TuKuRutch2/Arduino"
ESP32_VER=`ls ${ARDUINO_PATH}/portable/packages/esp32/hardware/esp32/`

#################
TARGETS=\

#################
S3TARGETS=\
"lovyanGFX "\
"uno32 "\
"DevkitC "\
"esp32cam "\
"M5Unified "\
"QuadCrawlerAI other/cameratch32 "\
"esp32camlcd/SquareCam "\
"M5CoreS3 "\
"QuadCrawler RemoconRobo "\
"other/GroveBeginnerKit "\

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
