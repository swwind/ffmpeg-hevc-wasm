/*
 * Copyright (c) 2001 Fabrice Bellard
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <emscripten.h>
#include <libavcodec/avcodec.h>
#include <libavutil/frame.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static AVCodecContext *codec_ctx = NULL;
static AVCodecParserContext *parser_ctx = NULL;
static AVFrame *frame = NULL;
static AVPacket *packet = NULL;
static uint8_t *input_data = NULL;
static size_t input_size = 0;
static size_t input_offset = 0;

EMSCRIPTEN_KEEPALIVE
void init(char *buf, size_t len) {
  input_data = (uint8_t *)buf;
  input_size = len;
  input_offset = 0;

  const AVCodec *codec = avcodec_find_decoder(AV_CODEC_ID_HEVC);
  if (!codec) {
    fprintf(stderr, "Codec HEVC not found\n");
    exit(1);
  }

  parser_ctx = av_parser_init(codec->id);
  if (!parser_ctx) {
    fprintf(stderr, "Parser not found\n");
    exit(1);
  }

  codec_ctx = avcodec_alloc_context3(codec);
  if (!codec_ctx) {
    fprintf(stderr, "Could not allocate codec context\n");
    exit(1);
  }

  if (avcodec_open2(codec_ctx, codec, NULL) < 0) {
    fprintf(stderr, "Could not open codec\n");
    exit(1);
  }

  frame = av_frame_alloc();
  if (!frame) {
    fprintf(stderr, "Could not allocate frame\n");
    exit(1);
  }

  packet = av_packet_alloc();
  if (!packet) {
    fprintf(stderr, "Could not allocate packet\n");
    exit(1);
  }
}

EMSCRIPTEN_KEEPALIVE
bool next_frame(char *output_frame) {

  if (avcodec_receive_frame(codec_ctx, frame) == 0) {
    // Assuming YUV420 format; copy frame data
    size_t y_size = codec_ctx->width * codec_ctx->height;
    size_t uv_size = y_size / 4;
    memcpy(output_frame, frame->data[0], y_size);                     // Y
    memcpy(output_frame + y_size, frame->data[1], uv_size);           // U
    memcpy(output_frame + y_size + uv_size, frame->data[2], uv_size); // V
    av_frame_unref(frame);
    return true; // Frame successfully decoded
  }

  // fprintf(stderr, "Looks like no frame is here\n");

  if (input_offset >= input_size) {
    fprintf(stderr, "EOF\n");
    return false; // End of input
  }

  int ret =
      av_parser_parse2(parser_ctx, codec_ctx, &packet->data, &packet->size,
                       input_data + input_offset, input_size - input_offset,
                       AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
  if (ret < 0) {
    fprintf(stderr, "Error while parsing\n");
    return false;
  }

  // fprintf(stderr, "Walks suceed %d\n", ret);
  input_offset += ret;

  if (packet->size > 0) {
    // fprintf(stderr, "Yes there is a packet!\n");

    if (avcodec_send_packet(codec_ctx, packet) < 0) {
      fprintf(stderr, "Error sending packet\n");
      return false;
    }

    return next_frame(output_frame);
  }

  // fprintf(stderr, "Sayonara, onei-sama: %d\n", packet->size);
  // return false; // No frame available
  return next_frame(output_frame);
}

void cleanup() {
  avcodec_free_context(&codec_ctx);
  av_parser_close(parser_ctx);
  av_packet_free(&packet);
}
