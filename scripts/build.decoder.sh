#!/usr/bin/bash

set -e

[ -d build ] && rm -rf build
mkdir build

emcc src/decode_video.c \
  ffmpeg-lib/lib/libavcodec.a \
  ffmpeg-lib/lib/libavutil.a \
  -O2 \
  -I"ffmpeg-lib/include" \
  -s WASM=1 \
  -s MODULARIZE \
  -s ENVIRONMENT="worker" \
  -s MAXIMUM_MEMORY=67108864 \
  -s ALLOW_MEMORY_GROWTH=1 \
  -s EXPORT_ES6=1 \
  -s EXPORTED_RUNTIME_METHODS=ccall,cwrap \
  -o build/decode_video.js

sed -i 's/= import.meta.url/= undefined/g' build/decode_video.js
