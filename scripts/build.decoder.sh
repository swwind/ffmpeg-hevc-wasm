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
  -s EXPORTED_FUNCTIONS="['_malloc']" \
  -msimd128 \
  -o build/decode_video.js

wasm-opt -O2 --enable-bulk-memory --enable-threads --enable-simd -o build/decode_video_o2.wasm build/decode_video.wasm
mv build/decode_video_o2.wasm build/decode_video.wasm

# sed -i 's/= import.meta.url/= undefined/g' build/decode_video.js
