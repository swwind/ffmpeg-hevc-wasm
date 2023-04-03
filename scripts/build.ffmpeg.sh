#!/usr/bin/bash

set -e

LIB_TARGET="$PWD/ffmpeg-lib"

FFMPEG_FLAGS=(
  --cc=emcc
  --cxx=em++
  --ar=emar
  --nm=emnm
  --ranlib=emranlib
  --prefix=$LIB_TARGET
  --enable-cross-compile
  --target-os=none
  --arch=x86_64
  --cpu=generic
  --enable-gpl
  --enable-version3
  --disable-sdl2
  --disable-iconv
  --disable-runtime-cpudetect
  --disable-cuda-llvm
  --disable-programs
  --disable-doc
  --disable-avdevice
  --disable-swresample
  --disable-swscale
  --disable-postproc
  --disable-avformat
  --disable-avfilter
  --disable-everything
  --disable-debug
  --disable-asm
  --disable-fast-unaligned
  --disable-network
  --enable-parser=hevc
  --enable-decoder=hevc
)

[ -d "$LIB_TARGET" ] && rm -rf "$LIB_TARGET"
mkdir "$LIB_TARGET"

cd ffmpeg

emconfigure ./configure ${FFMPEG_FLAGS[@]}

emmake make
emmake make install
