DIS=TukurutchV1.hex
cat robot_pcmode.ino.BBCmicrobit.hex | sed -e "s/:00000001FF//" > $DIS
cat s110_nrf51_8.0.0_softdevice.hex >> $DIS
cp $DIS ../../../scratch3/
sleep 3
