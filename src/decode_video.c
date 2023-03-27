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

/**
 * @file libavcodec video decoding API usage example
 * @example decode_video.c *
 *
 * Read from an HEVC video file, decode frames.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libavcodec/avcodec.h>

#include <errno.h>
#include <emscripten.h>

EMSCRIPTEN_KEEPALIVE
unsigned char *_frame_data_y(AVFrame *frame)
{
    return frame->data[0];
}

EMSCRIPTEN_KEEPALIVE
unsigned char *_frame_data_u(AVFrame *frame)
{
    return frame->data[1];
}

EMSCRIPTEN_KEEPALIVE
unsigned char *_frame_data_v(AVFrame *frame)
{
    return frame->data[2];
}

EMSCRIPTEN_KEEPALIVE
void _frame_unref(AVFrame *frame)
{
    av_frame_unref(frame);
}

EMSCRIPTEN_KEEPALIVE
int _receive_frame(AVCodecContext *c, AVFrame *frame)
{
    int ret;
    ret = avcodec_receive_frame(c, frame);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
    {
        return 0;
    }
    else if (ret < 0)
    {
        fprintf(stderr, "Error during decoding\n");
        return -1;
    }

    // decode_frame(frame);
    return 1;
}

EMSCRIPTEN_KEEPALIVE
int _send_packet(AVCodecContext *c, AVPacket *pkt)
{
    int ret;

    ret = avcodec_send_packet(c, pkt);
    // printf("avcodec_send_packet: %d\n", ret);
    if (ret < 0)
    {
        fprintf(stderr, "Error sending a packet for decoding\n");
        return -1;
    }

    return 0;
}

EMSCRIPTEN_KEEPALIVE
/* use the parser to split the data into frames */
int _parser_parse(AVCodecParserContext *parser, AVCodecContext *c, AVPacket *pkt, uint8_t *data, size_t data_size)
{
    int ret;

    ret = av_parser_parse2(parser, c, &pkt->data, &pkt->size,
                           data, data_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
    if (ret < 0)
    {
        fprintf(stderr, "Error while parsing: %s\n", strerror(errno));
        return -1;
    }

    return ret;
}

EMSCRIPTEN_KEEPALIVE
int _packet_size(AVPacket *pkt)
{
    return pkt->size;
}

EMSCRIPTEN_KEEPALIVE
AVPacket *_packet_alloc()
{
    AVPacket *pkt = av_packet_alloc();
    if (!pkt)
    {
        fprintf(stderr, "Failed to alloc packet\n");
        return NULL;
    }
    return pkt;
}

EMSCRIPTEN_KEEPALIVE
const AVCodec *_find_decoder_hevc()
{

    /* find the HEVC video decoder */
    const AVCodec *codec = avcodec_find_decoder(AV_CODEC_ID_HEVC);
    if (!codec)
    {
        fprintf(stderr, "Codec HEVC not found\n");
        return NULL;
    }

    return codec;
}

EMSCRIPTEN_KEEPALIVE
AVCodecParserContext *_parser_init(AVCodec *codec)
{
    AVCodecParserContext *parser = av_parser_init(codec->id);
    if (!parser)
    {
        fprintf(stderr, "parser not found\n");
        return NULL;
    }
    return parser;
}

EMSCRIPTEN_KEEPALIVE
AVCodecContext *_alloc_context3(AVCodec *codec)
{

    AVCodecContext *c = avcodec_alloc_context3(codec);
    if (!c)
    {
        fprintf(stderr, "Could not allocate video codec context\n");
        return NULL;
    }
    return c;
}

EMSCRIPTEN_KEEPALIVE
int _avcodec_open2(AVCodecContext *c, AVCodec *codec)
{
    /* open it */
    if (avcodec_open2(c, codec, NULL) < 0)
    {
        fprintf(stderr, "Could not open codec\n");
        return -1;
    }
    return 0;
}

EMSCRIPTEN_KEEPALIVE
AVFrame *_frame_alloc()
{
    AVFrame *frame = av_frame_alloc();
    if (!frame)
    {
        fprintf(stderr, "Could not allocate video frame\n");
        return NULL;
    }
    return frame;
}

EMSCRIPTEN_KEEPALIVE
void *_malloc(size_t size)
{
    return malloc(size);
}

EMSCRIPTEN_KEEPALIVE
void _free(void *ptr)
{
    return free(ptr);
}
