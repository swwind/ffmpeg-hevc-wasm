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
    // console.log("worker:", message.data);
    // console.log("worker:", Module);

    if (message.data.type === "decode") {
      log("loading video...");

      const width = message.data.width;
      const height = message.data.height;
      const fps = message.data.fps;
      const canvas = message.data.offscreen;
      const ctx = canvas.getContext('2d');
      canvas.width = width;
      canvas.height = height;

      const buffer = await (await fetch(message.data.url)).arrayBuffer();
      const ptr = Module._malloc(buffer.byteLength);
      Module.HEAPU8.set(new Uint8Array(buffer), ptr);

      Module._init(ptr, buffer.byteLength);

      const frameSize = message.data.width * message.data.height * 3 / 2;
      const frame = Module._malloc(frameSize);

      let consume = 0;

      function nextFrame(id) {
        const before = Date.now();

        if (!Module._next_frame(frame)) {
          log(`worker: done, avg ${Math.round(1000 / (consume / id))}fps.`);
          return;
        }

        const now = Date.now()

        // enqueue next frame
        setTimeout(nextFrame, before + 1_000 / fps - now, id + 1);
        consume += now - before;

        log(`frame#${id}: consume ${now - before}ms, avg ${Math.round(1000 / (consume / (id + 1)))}fps`);

        const frameBuffer = Module.HEAPU8.subarray(frame, frame + frameSize);
        const videoFrame = new VideoFrame(frameBuffer, {
          'codedWidth': width,
          'codedHeight': height,
          timestamp: id * (1_000 / fps),
          'format': 'I420',
        });

        ctx.drawImage(videoFrame, 0, 0);

        // postMessage({ type: 'frame', frame: videoFrame }, [videoFrame]);
      }

      nextFrame(0)
    }
  };

  postMessage({ type: "initialize" });
});
