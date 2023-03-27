import decoder from "./build/decode_video.js";
import { createYUV420PRenderer } from "./yuv420p-renderer.js";

function log(message, color) {
  postMessage({
    type: "fps",
    message,
    color,
  });
}

decoder().then((Module) => {
  onmessage = async (message) => {
    // console.log("worker:", message.data);

    if (message.data.type === "decode") {
      log("loading video...");

      const response = await fetch(message.data.url);
      const buffer = await response.arrayBuffer();

      // initialize
      const pkt = Module.__packet_alloc();
      const codec = Module.__find_decoder_hevc();
      const parser = Module.__parser_init(codec);
      const ctx = Module.__alloc_context3(codec);
      Module.__avcodec_open2(ctx, codec);
      const frame = Module.__frame_alloc();

      // copy video into memory
      const ptr = Module.__malloc(buffer.byteLength);
      Module.HEAPU8.set(new Uint8Array(buffer), ptr);

      // decode frames
      function* generator() {
        let data = ptr;
        let data_size = buffer.byteLength;

        const size = width * height;

        while (1) {
          const ret = Module.__parser_parse(parser, ctx, pkt, data, data_size);
          if (ret < 0) break;

          data += ret;
          data_size -= ret;

          if (Module.__packet_size(pkt) > 0) {
            const ret = Module.__send_packet(ctx, pkt);
            if (ret < 0) break;

            while (Module.__receive_frame(ctx, frame) > 0) {
              const yptr = Module.__frame_data_y(frame);
              const uptr = Module.__frame_data_u(frame);
              const vptr = Module.__frame_data_v(frame);

              const yData = Module.HEAPU8.subarray(yptr, yptr + size);
              const uData = Module.HEAPU8.subarray(uptr, uptr + size / 4);
              const vData = Module.HEAPU8.subarray(vptr, vptr + size / 4);

              yield [yData, uData, vData];
              Module.__frame_unref(frame);
            }
          }
        }
      }

      // start playing video
      const canvas = message.data.canvas;
      const width = message.data.width;
      const height = message.data.height;

      canvas.width = width;
      canvas.height = height;

      const render = createYUV420PRenderer(canvas, width, height);

      const frames = generator();

      const fps = message.data.fps;
      const frameIntervalTime = 1000 / fps;

      await new Promise((resolve) => {
        const playStart = Date.now();

        let frame = frames.next();
        let frameNumber = 0;
        const interval = setInterval(() => {
          if (frame.done) {
            clearInterval(interval);
            return resolve();
          }

          ++frameNumber;

          const frameStart = Date.now();
          render(...frame.value);
          frame = frames.next();

          const frameTime = Date.now() - frameStart;
          const playTime = Date.now() - playStart;

          log(
            `frame: #${frameNumber}, time = ${frameTime.toFixed(
              2
            )}/${frameIntervalTime.toFixed(2)}ms, fps = ${(
              1000 /
              (playTime / frameNumber)
            ).toFixed(2)}/${fps}`,
            frameTime > frameIntervalTime ? "red" : "green"
          );
        }, frameIntervalTime);
      });

      Module.__free(ptr);
    }
  };

  postMessage({ type: "initialize" });
});
