#!/bin/bash
PWD_=`echo -n $PWD | sed -E "s/\/mnt\/c/C:/"`
#ARDUINOPATH="/mnt/c/fd_work/TuKuRutchExe/Arduino"
ARDUINOPATH="$PWD/../../Arduino"

#################
TARGETS=\

function buildSRCs() {
  PARAMS=`node parseRobotJson.js $1 params`
  #node parseRobotJson.js $1 jsonToCpp2         # src.ino生成

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
  echo "usage: build.sh <target>"
  exit 1
fi

if [ $1 == "all" ]; then
  echo "TARGETS=$TARGETS"

  for target in $TARGETS; do
    echo ""
    echo "building $target..."
    buildSRCs $target
  done

else
  buildSRCs $1
  if [ $# -ge 2 ]; then
    CMD=`node parseRobotJson.js $1 burnFW $2`
    echo $CMD
    $CMD
  fi
fi

# /mnt/c/fd_work/TuKuRutchExe/Arduino/arduino.exe
