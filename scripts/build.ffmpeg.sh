#!/usr/bin/bash

set -e

LIB_TARGET="$PWD/ffmpeg-lib"

ASM_FLAGS=(
  --disable-altivec        # disable AltiVec optimizations
  --disable-vsx            # disable VSX optimizations
  --disable-power8         # disable POWER8 optimizations
  # --disable-amd3dnow       # disable 3DNow! optimizations
  # --disable-amd3dnowext    # disable 3DNow! extended optimizations
  # --disable-mmx            # disable MMX optimizations
  # --disable-mmxext         # disable MMXEXT optimizations
  # --disable-sse            # disable SSE optimizations
  # --disable-sse2           # disable SSE2 optimizations
  # --disable-sse3           # disable SSE3 optimizations
  # --disable-ssse3          # disable SSSE3 optimizations
  # --disable-sse4           # disable SSE4 optimizations
  # --disable-sse42          # disable SSE4.2 optimizations
  # --disable-avx            # disable AVX optimizations
  # --disable-xop            # disable XOP optimizations
  # --disable-fma3           # disable FMA3 optimizations
  # --disable-fma4           # disable FMA4 optimizations
  # --disable-avx2           # disable AVX2 optimizations
  # --disable-avx512         # disable AVX-512 optimizations
  # --disable-avx512icl      # disable AVX-512ICL optimizations
  # --disable-aesni          # disable AESNI optimizations
  --disable-armv5te        # disable armv5te optimizations
  --disable-armv6          # disable armv6 optimizations
  --disable-armv6t2        # disable armv6t2 optimizations
  --disable-vfp            # disable VFP optimizations
  --disable-neon           # disable NEON optimizations
  --disable-inline-asm     # disable use of inline assembly
  --disable-x86asm         # disable use of standalone x86 assembly
  --disable-mipsdsp        # disable MIPS DSP ASE R1 optimizations
  --disable-mipsdspr2      # disable MIPS DSP ASE R2 optimizations
  --disable-msa            # disable MSA optimizations
  --disable-mipsfpu        # disable floating point MIPS optimizations
  --disable-mmi            # disable Loongson MMI optimizations
  --disable-lsx            # disable Loongson LSX optimizations
  --disable-lasx           # disable Loongson LASX optimizations
  --disable-rvv            # disable RISC-V Vector optimizations
)

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
  ${ASM_FLAGS[@]}
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
