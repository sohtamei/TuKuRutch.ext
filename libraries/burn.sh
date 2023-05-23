#!/bin/bash

if [ $# -le 1 ]; then
  echo "usage: burn.sh <target> <COMxx>"
  exit 1
fi

CMD=`node parseRobotJson.js $1 burnFW $2`
echo $CMD
$CMD
