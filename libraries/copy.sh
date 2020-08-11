#!/bin/bash

SRC_DIR=RemoconRobo
DIS_DIR="BTO_ArduinoIR Duke32AIO esp32 Koov LiquidCrystal M5CameraCar m5stack m5stickC mbot QuadCrawler QuadCrawlerESP RemoconRobo TukuBoard1.0 TukuBoardUSB"

SRC_DIR2=TukuBoard1.0
DIS_DIR2="Duke32AIO esp32 M5CameraCar m5stack m5stickC QuadCrawlerESP TukuBoard1.0"

for dis in $DIS_DIR; do
  if [ "$dis" != "$SRC_DIR" ]; then
    cp $SRC_DIR/robot.js $dis/robot.js
    cp $SRC_DIR/robot_pcmode/robot_pcmode.ino.template $dis/robot_pcmode/robot_pcmode.ino.template
  fi
done

for dis in $DIS_DIR2; do
  if [ "$dis" != "$SRC_DIR2" ]; then
    cp $SRC_DIR2/robot_pcmode/robot_pcmode.js.template $dis/robot_pcmode/robot_pcmode.js.template
  fi
done
