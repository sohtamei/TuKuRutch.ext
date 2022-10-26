#!/bin/bash
NAME_SRC=src.update.js       #`ls *.update.js -1 | head -1`
NAME_DES=src.update.js.h
echo "conv $NAME_SRC -> $NAME_DES"
echo "const uint8_t updateJS[] = {"                                           > $NAME_DES
gzip -c $NAME_SRC | hexdump -e '16/1 "0x%02x, "' -e '"\n"' | sed 's/0x  ,//g' >> $NAME_DES
echo "};"                                                                     >> $NAME_DES
echo "#define updateJS_len sizeof(updateJS)"                                  >> $NAME_DES
sleep 2
