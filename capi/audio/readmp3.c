/*
  readmp3.c
  
  Copyright (c) 2012, Jeremiah LaRocco jeremiah.larocco@gmail.com

  Permission to use, copy, modify, and/or distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include <inttypes.h>
#include <math.h>
#include <limits.h>
#include <signal.h>

#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/mem.h"

#include <stdio.h>

#include <unistd.h>
#include <assert.h>

const int AUDIO_INBUF_SIZE = 4096*5;
const int AUDIO_REFILL_THRESH = 4096;

int read_audio(char *fname, int16_t **buffer, int *audio_size, int *format) {
    // It's important this be aligned correctly...
    AVFormatContext *pFormatCtx __attribute__ ((aligned (16)));

    if (avformat_open_input(&pFormatCtx, fname, NULL, 0) != 0) {
        return -1;
    }
    
    avformat_find_stream_info(pFormatCtx, NULL);

    double sec_duration = pFormatCtx->duration/(double)AV_TIME_BASE;
    int brate = pFormatCtx->bit_rate;
    int estimated_buff_size = brate *(int)floor(sec_duration)/2;
    if (pFormatCtx->streams[0]->codec->codec_type!= AVMEDIA_TYPE_AUDIO) {
        avformat_close_input(&pFormatCtx);
        return -1;
    }
    
    AVPacket packet;

    av_init_packet(&packet);

    AVCodecContext *aCodecCtx;
    aCodecCtx=pFormatCtx->streams[0]->codec;

    AVCodec         *aCodec;
    aCodec = avcodec_find_decoder(aCodecCtx->codec_id);
    if (!aCodec) {
        avformat_close_input(&pFormatCtx);
        return -1;
    }
    
    if (avcodec_open2(aCodecCtx, aCodec, NULL) < 0) {
        avformat_close_input(&pFormatCtx);
        return -1;
    }

    int gotit = 0;
    AVFrame *frame = NULL;
    if (!frame) {
        if (!(frame = avcodec_alloc_frame())) {
            avcodec_close(aCodecCtx);
            avformat_close_input(&pFormatCtx);
            return -1;
        }
    } else
        avcodec_get_frame_defaults(frame);
    
    int xp = 0;
    int rv = av_read_frame(pFormatCtx, &packet);
    int total_data_size = 0;
    int allocated_buffer = estimated_buff_size;
    *buffer = malloc(allocated_buffer);

    int planar = 0;
    while (packet.size > 0) {
        int len = avcodec_decode_audio4(pFormatCtx->streams[0]->codec, frame, &gotit, &packet);

        int ch, plane_size;
        planar = av_sample_fmt_is_planar(aCodecCtx->sample_fmt);
        int data_size = av_samples_get_buffer_size(&plane_size, aCodecCtx->channels,
                                                   frame->nb_samples,
                                                   aCodecCtx->sample_fmt, 1);

        if (total_data_size+data_size > allocated_buffer) {
            allocated_buffer = allocated_buffer*1.25;
            *buffer = realloc(*buffer, allocated_buffer);
        }
        memcpy(*buffer, frame->extended_data[0], data_size);
        total_data_size += data_size;
        
        rv = av_read_frame(pFormatCtx, &packet);
    }
    *format = planar;
    *audio_size = total_data_size;
    avcodec_close(aCodecCtx);
    avformat_close_input(&pFormatCtx);
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc<2) {
        printf("No file names given.\n");
        exit(1);
    }
    av_register_all();

    int16_t *buffer;
    int audio_size = 0;
    int format = -1;
    
    read_audio(argv[1], &buffer, &audio_size, &format);

    printf("Read %d bytes of audio data from %s\n", audio_size, argv[1]);
    
    free(buffer);

    return 0;
}
