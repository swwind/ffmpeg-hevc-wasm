import decoder from "./build/decode_video.js";

function log(message, color) {
  postMessage({
    type: "fps",
    message,
    color,
  });
}

decoder().then((Module) => {
  onmessage = async (message) => {
    console.log("worker:", message.data);
    console.log("worker:", Module);

    if (message.data.type === "decode") {
      log("loading video...");

      const width = message.data.width;
      const height = message.data.height;
      const fps = message.data.fps;

      const buffer = await (await fetch(message.data.url)).arrayBuffer();
      const ptr = Module._malloc(buffer.byteLength);
      Module.HEAPU8.set(new Uint8Array(buffer), ptr);

      Module._init(ptr, buffer.byteLength);

      const frameSize = message.data.width * message.data.height * 3 / 2;
      const frame = Module._malloc(frameSize);

      function nextFrame(id) {
        const before = Date.now();

        if (!Module._next_frame(frame)) {
          log('worker: done.');
          return;
        }

        const now = Date.now()

        // enqueue next frame
        setTimeout(nextFrame, before + 1_000 / fps - now, id + 1);
        log(`frame#${id}: consume ${now - before}ms`);

        const frameBuffer = Module.HEAPU8.subarray(frame, frame + frameSize);
        const videoFrame = new VideoFrame(frameBuffer, {
          'codedWidth': width,
          'codedHeight': height,
          timestamp: id * (1_000 / fps),
          'format': 'I420',
        });

        postMessage({ type: 'frame', frame: videoFrame }, [videoFrame]);
      }

      nextFrame(0)
    }
  };

  postMessage({ type: "initialize" });
});
