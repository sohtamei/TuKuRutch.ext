DIS=Tukurutch.microbitV1.hex
mergehex.exe -m s110_nrf51_8.0.0_softdevice.hex robot_pcmode.ino.BBCmicrobit.hex -o $DIS
cp $DIS ../../../scratch3/
sleep 3
