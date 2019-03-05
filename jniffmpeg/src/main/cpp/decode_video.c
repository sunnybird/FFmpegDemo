//
// Created by fengjl on 2019/3/5.
//

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
 * @file
 * video decoding with libavcodec API example
 *
 * @example decode_video.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/dict.h>
#include "decode_video.h"
#include "nativelog.h"
#include "android/log.h"


static void pgm_save(unsigned char *buf, int wrap, int xsize, int ysize,
                     char *filename)
{
    FILE *f;
    int i;
    f = fopen(filename,"w");
    fprintf(f, "P5\n%d %d\n%d\n", xsize, ysize, 255);
    for (i = 0; i < ysize; i++)
        fwrite(buf + i * wrap, 1, xsize, f);
    fclose(f);
}
static void decode(AVCodecContext *dec_ctx, AVFrame *frame, AVPacket *pkt,
                   const char *filename)
{
    char buf[1024];
    int ret;
    ret = avcodec_send_packet(dec_ctx, pkt);
    if (ret < 0) {
        LOGD("Error sending a packet for decoding : %d \n",ret);
        exit(1);
    }
    while (ret >= 0) {
        ret = avcodec_receive_frame(dec_ctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0) {
            LOGD("Error during decoding\n");
            exit(1);
        }
        LOGD("saving frame %3d\n", dec_ctx->frame_number);
        fflush(stdout);
        /* the picture is allocated by the decoder. no need to
           free it */
        snprintf(buf, sizeof(buf), "%s-%d", filename, dec_ctx->frame_number);
        pgm_save(frame->data[0], frame->linesize[0],
                 frame->width, frame->height, buf);
    }
}
int  decode_video(const char* srcPath, const char * desPath)
{
    const char *filename, *outfilename;
    const AVCodec *codec;
    AVCodecParserContext *parser;
    AVCodecContext *c= NULL;
    FILE *f;
    AVFrame *frame;
    uint8_t inbuf[INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
    uint8_t *data;
    size_t   data_size;
    int ret;
    AVPacket *pkt;

    filename    = srcPath;
    outfilename = desPath;

    av_register_all();

    pkt = av_packet_alloc();
    if (!pkt)
        exit(1);
    /* set end of buffer to 0 (this ensures that no overreading happens for damaged MPEG streams) */
    memset(inbuf + INBUF_SIZE, 0, AV_INPUT_BUFFER_PADDING_SIZE);


    AVFormatContext *fmt_ctx = NULL;
    AVDictionaryEntry *tag = NULL;
    int ret1;

    fmt_ctx = avformat_alloc_context();


    if ((ret1 = avformat_open_input(&fmt_ctx, filename, NULL, NULL)))
        return ret1;
    while ((tag = av_dict_get(fmt_ctx->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)))
        LOGD("%s=%s\n", tag->key, tag->value);


    // Retrieve stream information
    if (avformat_find_stream_info(fmt_ctx, NULL) < 0) {
        LOGD("Couldn't find stream information.");
        return -1;
    }

    int videoStream = -1;

    for(int i=0; i < fmt_ctx->nb_streams; i++ ){
        if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO
            && videoStream < 0) {
            videoStream = i;
        }
    }

    if (videoStream == -1) {
        LOGD("Didn't find a video stream.");
        return -1; // Didn't find a video stream
    }

    enum AVCodecID  avCodecID = fmt_ctx->streams[videoStream]->codecpar->codec_id;

    /* find the MPEG-1 video decoder */
    codec = avcodec_find_decoder(avCodecID);
    if (!codec) {
        LOGD("Codec not found\n");
        exit(1);
    }
    parser = av_parser_init(codec->id);
    if (!parser) {
        LOGD("parser not found\n");
        exit(1);
    }
    c = avcodec_alloc_context3(codec);
    if (!c) {
        LOGD("Could not allocate video codec context\n");
        exit(1);
    }
    /* For some codecs, such as msmpeg4 and mpeg4, width and height
       MUST be initialized there because this information is not
       available in the bitstream. */
    /* open it */
    if (avcodec_open2(c, codec, NULL) < 0) {
        LOGD("Could not open codec\n");
        exit(1);
    }
    f = fopen(filename, "rb");
    if (!f) {
        LOGD("Could not open %s\n", filename);
        exit(1);
    }
    frame = av_frame_alloc();
    if (!frame) {
        LOGD("Could not allocate video frame\n");
        exit(1);
    }
    while (!feof(f)) {
        /* read raw data from the input file */
        data_size = fread(inbuf, 1, INBUF_SIZE, f);
        if (!data_size)
            break;
        /* use the parser to split the data into frames */
        data = inbuf;
        while (data_size > 0) {
            ret = av_parser_parse2(parser, c, &pkt->data, &pkt->size,
                                   data, data_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
            if (ret < 0) {
                LOGD("Error while parsing");
                exit(1);
            }
            data      += ret;
            data_size -= ret;
            if (pkt->size)
                decode(c, frame, pkt, outfilename);
        }
    }
    /* flush the decoder */
    decode(c, frame, NULL, outfilename);
    fclose(f);
    av_parser_close(parser);
    avcodec_free_context(&c);
    av_frame_free(&frame);
    av_packet_free(&pkt);
    return 0;
}

