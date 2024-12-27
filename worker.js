import decoder from "./build/decode_video.js";

function log(message, color) {
  postMessage({
    type: "fps",
    message,
    color,
  });
}

decoder().then((Module) => {

  let frame, frameSize;
  let consume = 0;
  let id = 0;
  let width, height;

  onmessage = async (message) => {
    // console.log("worker:", message.data);
    // console.log("worker:", Module);

    if (message.data.type === "decode") {
      log("loading video...");

      width = message.data.width;
      height = message.data.height;
      // const fps = message.data.fps;

      const buffer = await (await fetch(message.data.url)).arrayBuffer();
      const ptr = Module._malloc(buffer.byteLength);
      Module.HEAPU8.set(new Uint8Array(buffer), ptr);

      Module._init(ptr, buffer.byteLength);

      frameSize = width * height * 3 / 2;
      frame = Module._malloc(frameSize);

      postMessage({ type: 'ready' });
    }

    if (message.data.type === 'frame') {
      // process next frame
      const before = Date.now();

      if (!Module._next_frame(frame)) {
        log(`done, avg ${Math.round(1000 / (consume / id))}fps.`);
        postMessage({ type: 'end' });
        return;
      }

      const now = Date.now()

      consume += now - before;

      log(`frame#${id}: consume ${now - before}ms, avg ${Math.round(1000 / (consume / (++id)))}fps`);

      const frameBuffer = Module.HEAPU8.subarray(frame, frame + frameSize);
      const videoFrame = new VideoFrame(frameBuffer, {
        codedWidth: width,
        codedHeight: height,
        timestamp: 0,
        format: 'I420',
      });

      postMessage({ type: 'frame', frame: videoFrame }, [videoFrame]);
    }

  };

  postMessage({ type: "initialize" });
});
