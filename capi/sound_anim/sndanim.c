/*
  sndanim.c
  
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

#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/mem.h"

#include <stdio.h>
#include <complex.h>
#include <fftw3.h>

#include <ri.h>

typedef struct audio_data_s {
    uint8_t *samples;
    size_t buffer_size;
    size_t used_buffer_size;
    size_t num_samples;
    size_t sample_size;
    size_t sample_rate;
    int8_t channels;
    double duration;
    int8_t planar;
} audio_data_t;

int32_t get_sample(audio_data_t *ad, size_t idx, int8_t channel) {
    int32_t rv = 0;
    if (idx > ad->num_samples ||
        channel<0 || channel > ad->channels) {
        return rv;
    }
    int mul = 1;
    int offset = 0;
    if (ad->planar == 1) {
        mul = 1;
        offset = ad->num_samples * channel;
    } else {
        offset = channel;
        mul = ad->channels;
    }
    
    switch (ad->sample_size) {
    case 1:
    {
        int8_t tmp = ad->samples[mul * idx+offset];
        rv = (int32_t)tmp;
        break;
    }
    case 2:
    {
        int16_t tmp = ((int16_t*)ad->samples)[mul*idx/2 + offset];
        rv = (int32_t)tmp;
        break;
    }
    default:
        rv = ((int32_t*)ad->samples)[mul*idx/4 + offset];
    }
    return rv;
}

void doFrame(int fNum,
             /* double rval, */
             size_t cur, int fft_size, fftw_complex *fft_data[],
             char *fName);

int read_audio(char *fname, audio_data_t *ad);


int read_audio(char *fname, audio_data_t *ad) {
    // It's important this be aligned correctly...
    AVFormatContext *pFormatCtx __attribute__ ((aligned (16)));

    if (avformat_open_input(&pFormatCtx, fname, NULL, 0) != 0) {
        return -1;
    }
    
    avformat_find_stream_info(pFormatCtx, NULL);

    if (pFormatCtx->streams[0]->codec->codec_type
        != AVMEDIA_TYPE_AUDIO) {
        
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
    
    double sec_duration = pFormatCtx->duration/(double)AV_TIME_BASE;
    ad->duration = sec_duration;

    int brate = pFormatCtx->bit_rate;
    
    int xp = 0;
    int total_data_size = 0;
    int total_samples = 0;

    int estimated_buff_size = brate *(int)floor(sec_duration)/2;
    
    int allocated_buffer = estimated_buff_size;

    ad->samples = malloc(allocated_buffer);
    
    ad->channels = aCodecCtx->channels;
    
    ad->sample_rate = aCodecCtx->sample_rate;
    int rv = av_read_frame(pFormatCtx, &packet);
    while (packet.size > 0) {
        int len = avcodec_decode_audio4(pFormatCtx->streams[0]->codec,
                                        frame, &gotit, &packet);

        int plane_size;
        int data_size = av_samples_get_buffer_size(
            &plane_size,
            aCodecCtx->channels,
            frame->nb_samples,
            aCodecCtx->sample_fmt, 1);

        if (total_data_size+data_size > allocated_buffer) {
            allocated_buffer = allocated_buffer*1.25;
            ad->samples = realloc(ad->samples, allocated_buffer);
        }
        memcpy(ad->samples+total_data_size, frame->extended_data[0], data_size);
        total_data_size += data_size;
        total_samples += frame->nb_samples;
        
        rv = av_read_frame(pFormatCtx, &packet);
        
    }
    // Use the last frame to fill in the info needed
        
    ad->used_buffer_size = total_data_size;
    ad->buffer_size = allocated_buffer;
    ad->planar = av_sample_fmt_is_planar(aCodecCtx->sample_fmt);
    ad->sample_size = av_get_bytes_per_sample(aCodecCtx->sample_fmt);
    ad->samples = realloc(ad->samples, total_data_size);
    ad->buffer_size = total_data_size;
    ad->num_samples = total_samples;
    avcodec_close(aCodecCtx);
    avformat_close_input(&pFormatCtx);
    return 1;
}

void doFrame(int fNum,
             /* double rval, */
             size_t cur, int fft_size, fftw_complex *fft_data[],
             char *fName) {

    RiFrameBegin(fNum);
    static RtColor Color = {.2, .4, .6} ;

    char buffer[256];
    sprintf(buffer, "images/%s%05d.tif", fName, fNum);
    RiDisplay(buffer,(char*)"file",(char*)"rgba",RI_NULL);
  
    RiFormat(800, 600,  1.25);
    RiLightSource((char*)"distantlight",RI_NULL);
    RiProjection((char*)"perspective",RI_NULL);
  
    RiTranslate(0.0,0.0,0.8*fft_size);
    /* RiTranslate(0.0,0.0,15); */
    RiRotate( -100.0, 1.0, 0.0, 0.0);
    RiRotate(75.0, 0.0,0.0, 1.0);
    /* RiRotate( 45.0, 0.0, 1.0, 0.0); */
  
    RiWorldBegin();
  
    RiColor(Color);

    /* RiRotate(fNum, 0.0,1.0,0.0); */

    RtFloat  roughness = 0.03;
    int trace = 1;
    RtFloat opac[] = {0.5, 0.9, 0.3};

    RiSurface((char*)"matte", RI_NULL);
    /* RiSphere(rval, -rval, rval, 360, RI_NULL); */

    RiTranslate(-fft_size/2.0, -fft_size/2.0+fft_size/4.0, 0);
    size_t fft_idx = cur;
    for (int i=fft_size-1; i>=0; --i) {
        fft_idx += 1;
        if (fft_idx == fft_size) {
            fft_idx = 0;
        }
        /* printf("fft_idx = %lu, i = %d\n", fft_idx, i); */
        for (size_t j=0; j<fft_size/2; ++j) {
            RiAttributeBegin(); {
                double fft_v = cabs(fft_data[fft_idx][j]);
                double r = i/(double)(fft_size-1);
                double g = fft_v;
                if (g>1.0) { g = 1.0; }
                double b = j/(double)(fft_size/2);
                /* printf("rgb = %f %f %f", r, g, b); */
                RtColor tc = {r,g,b};
                RiColor(tc);
                
                RiTranslate(fft_size-i, j, fft_v);
                RiSphere(0.5, -0.5, 0.5, 360, RI_NULL);
            } RiAttributeEnd();
        }
    }

    RiWorldEnd();
    RiFrameEnd();
}

int main(int argc, char *argv[]) {
    if (argc<2) {
        printf("No file names given.\n");
        exit(1);
    }
    av_register_all();

    audio_data_t snd_data;
    read_audio(argv[1], &snd_data);

    int per_frame = snd_data.sample_rate/30;

    int32_t maxVal = 0;
    switch (snd_data.sample_size) {
    case 1:
        maxVal = 1<<6;
        break;
    case 2:
        maxVal = 1<<12;
        break;
    default:
        maxVal = 1<<30;
    }
    const int N = 128;

    fftw_complex *fft_in  __attribute__ ((aligned (16)));
    fftw_complex **fft_out  __attribute__ ((aligned (16)));

    fftw_plan *fft_plan  __attribute__ ((aligned (16)));

    fft_in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    fft_out = (fftw_complex**) fftw_malloc(sizeof(fftw_complex*) * N);

    fft_plan = (fftw_plan*) fftw_malloc(sizeof(fftw_plan) * N);
    
    for (int i=0; i<N; ++i) {
        printf("Allocating plan %d\n", i);
        fft_out[i] = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
        
        if (fft_out[i] != NULL) {
            printf("Creating plan for %d\n", i);
            fft_plan[i] = fftw_plan_dft_1d(N, fft_in, fft_out[i], FFTW_FORWARD, FFTW_ESTIMATE);
        } else {
            printf("fftw_malloc failed!\n");
            exit(1);
        }
    }

    size_t fnum, cur_out;

    printf("Filling fft_out\n");
    for (size_t i=0; i<N; ++i) {
        fft_in[i] = 0.0;
        for (size_t j=0; j<N; ++j) {
            fft_out[i][j] = 0.0;
        }
    }
    RiBegin(RI_NULL);
    size_t num_frames = (snd_data.num_samples-per_frame)/per_frame;
    for (size_t i = 0, cur_out = 0, fnum = 1; i<(snd_data.num_samples-per_frame); i+= per_frame, ++fnum) {

        for (size_t j=0; j< N/2;++j) {
            fft_in[j] = (double)get_sample(&snd_data, per_frame*15+i+j, 0)/(double)maxVal;
        }

        fftw_execute_dft(fft_plan[0], fft_in, fft_out[cur_out]);

        printf("Calling doFrame %lu of %lu\n", fnum, num_frames);

        doFrame(fnum,
                cur_out, N, fft_out,
                argv[2]);

        cur_out += 1;
        if (cur_out == N) {
            cur_out = 0;
        }
    }

    RiEnd();
    for (size_t i=0; i<N; ++i) {
        fftw_destroy_plan(fft_plan[i]);
        fftw_free(fft_out[i]);
    }
    fftw_free(fft_out);
    fftw_free(fft_in);
    fftw_free(fft_plan);
    fftw_cleanup();
    free(snd_data.samples);

    return 0;
}


