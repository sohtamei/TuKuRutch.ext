#!/bin/bash
echo "const uint8_t index_ov2640_html_gz[] = {"                        > camera_index.h
gzip -c index_ov2640.html | hexdump -e '16/1 "0x%02x, "' -e '"\n"' | sed 's/0x  ,//g' >> camera_index.h
echo "};"                                                              >> camera_index.h
echo "#define index_ov2640_html_gz_len sizeof(index_ov2640_html_gz)"   >> camera_index.h
echo                                                                   >> camera_index.h
echo "const uint8_t index_ov3660_html_gz[] = {"                        >> camera_index.h
gzip -c index_ov3660.html | hexdump -e '16/1 "0x%02x, "' -e '"\n"' | sed 's/0x  ,//g' >> camera_index.h
echo "};"                                                              >> camera_index.h
echo "#define index_ov3660_html_gz_len sizeof(index_ov3660_html_gz)"   >> camera_index.h
