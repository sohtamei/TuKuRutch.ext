#!/bin/bash
ARDUINO_PATH="/mnt/c/fd_work/TuKuRutch2/Arduino"
ESP32_VER=`ls ${ARDUINO_PATH}/portable/packages/esp32/hardware/esp32/`

if [ $# -le 1 ]; then
  echo "usage: burn.sh <target> <COMxx>"
  exit 1
fi

ARDUINO_PATH_=`echo $ARDUINO_PATH | sed -e "s@/mnt/c@@"`
CMD=`node parseRobotJson.js $1 burnFW $2 | sed -e "s@ARDUINO_PATH@${ARDUINO_PATH_}@g" | sed -e "s@ESP32_VER@${ESP32_VER}@g"`
echo $CMD
$CMD
