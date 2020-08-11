#!/bin/bash

SC3_PATH="/mnt/c/Users/n-tom/scratch3"

copy_sb3 () {
  if [ "$3" ]; then
    cat $1/robot_pcmode/robot_pcmode.js | sed -e "s/DummyName/$2/g" | sed -e "s/const $3 = false;/const $3 = true;/" > $SC3_PATH/scratch-vm/src/extensions/scratch3_tukurutch/$2.js
  else
    cat $1/robot_pcmode/robot_pcmode.js | sed -e "s/DummyName/$2/g" > $SC3_PATH/scratch-vm/src/extensions/scratch3_tukurutch/$2.js
  fi
}

copy_sb3 TukuBoard1.0 TukuBoard
copy_sb3 Duke32AIO    Duke32AIO
copy_sb3 M5CameraCar  WifiCar    SupportCamera
