LIST=('analogRemote' 'TukurutchEsp' 'TukurutchEspCamera' 'TukurutchCommon')
ARDUINO_PATH="/mnt/c/fd_work/Arduino"

for NAME in ${LIST[@]}; do
  echo $NAME
  rm -rf $ARDUINO_PATH/portable/sketchbook/libraries/$NAME
  cp -a $NAME $ARDUINO_PATH/portable/sketchbook/libraries/
#  rm -f ../../$NAME.zip
#  zip -q ../../$NAME.zip $NAME/*
done

sleep 3
