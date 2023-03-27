# ffmpeg-hevc-wasm

Just for play and tutorial.

[Online demo](https://swwind.github.io/ffmpeg-hevc-wasm)

## Build

```
git clone https://github.com/swwind/ffmpeg-hevc-wasm
cd ffmpeg-hevc-wasm
git submodule update --init --depth=1

yarn
yarn build.ffmpeg
yarn build.wasm
yarn build
```

Then start a http-server serve `/dist` directory.
