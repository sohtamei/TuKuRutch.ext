#!/bin/bash

BASE_PATH="../../../"

copy_sb3 () {
  if [ "$3" ]; then
    cat $1/robot_pcmode/robot_pcmode.js | sed -e "s/DummyName/$2/g" | sed -e "s/const $3 = false;/const $3 = true;/" > $BASE_PATH/scratch-vm/src/extensions/scratch3_tukurutch/$2.js
  else
    cat $1/robot_pcmode/robot_pcmode.js | sed -e "s/DummyName/$2/g" > $BASE_PATH/scratch-vm/src/extensions/scratch3_tukurutch/$2.js
  fi
}

copy_sb3 TukuBoard1.0   TukuBoard
copy_sb3 M5CameraCar    M5CameraCar    SupportCamera
copy_sb3 M5StickCPlus   M5Series
copy_sb3 QuadCrawlerAI QuadCrawlerAI SupportCamera
