DIS=Tukurutch.microbitV2.hex
mergehex.exe -m s113_nrf52_7.2.0_softdevice.hex src.ino.BBCmicrobitV2.hex -o $DIS
cp $DIS ../../../scratch3/
sleep 3
