LIST=('analogRemote' 'TukurutchEsp' 'TukurutchEspCamera' 'TukurutchCommon')

for NAME in ${LIST[@]}; do
  echo $NAME
  rm -rf ../../Arduino/portable/sketchbook/libraries/$NAME
  cp -a $NAME ../../Arduino/portable/sketchbook/libraries/
  rm -f ../../$NAME.zip
  zip -q ../../$NAME.zip $NAME/*
done

sleep 3
