#!/bin/bash

SC3_PATH="/mnt/c/Users/n-tom/scratch3"

DIRNAME=TukuBoard1.0
EXTNAME=TukuBoard
cat $DIRNAME/robot_pcmode/robot_pcmode.js | sed -e "s/DummyName/$EXTNAME/g" > $SC3_PATH/scratch-vm/src/extensions/scratch3_tukurutch/$EXTNAME.js


DIRNAME=Duke32AIO
EXTNAME=Duke32AIO
cat $DIRNAME/robot_pcmode/robot_pcmode.js | sed -e "s/DummyName/$EXTNAME/g" > $SC3_PATH/scratch-vm/src/extensions/scratch3_tukurutch/$EXTNAME.js
