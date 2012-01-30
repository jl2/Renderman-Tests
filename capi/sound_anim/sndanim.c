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

void doFrame(int fNum, double rval, char *fName);
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

void doFrame(int fNum, double rval, char *fName) {

    RiFrameBegin(fNum);
    static RtColor Color = {.2, .4, .6} ;

    char buffer[256];
    sprintf(buffer, "images/%s%05d.tif", fName, fNum);
    RiDisplay(buffer,(char*)"file",(char*)"rgba",RI_NULL);
  
    RiFormat(800, 600,  1.25);
    RiLightSource((char*)"distantlight",RI_NULL);
    RiProjection((char*)"perspective",RI_NULL);
  
    RiTranslate(0.0,0.0,20);
  
    RiWorldBegin();
  
    RiColor(Color);

    /* RiRotate(fNum, 0.0,1.0,0.0); */

    RtFloat  roughness = 0.03;
    int trace = 1;
    RtFloat opac[] = {0.5, 0.9, 0.3};

    RiSurface((char*)"matte", RI_NULL);
    RiSphere(rval, -rval, rval, 360, RI_NULL);

    RiWorldEnd();
    RiFrameEnd();
}

int32_t average_between(audio_data_t *ad, size_t idx1, size_t idx2) {
    int64_t sum = 0;
    for (size_t i = idx1; i < idx2; ++i ) {
        int32_t tmp = get_sample(ad, i, 0);
        if (tmp<0) {
            tmp = -tmp;
        }
        sum += tmp;
    }
    int32_t av = (int32_t)(sum / (int64_t)(idx2-idx1));
    return av;
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
        maxVal = 1<<7;
        break;
    case 2:
        maxVal = 1<<14;
        break;
    default:
        maxVal = 1<<31;
    }
    
    RiBegin(RI_NULL);
    fftw_complex *fft_in, *fft_out;
    fftw_plan fft_plan;
    const int N = 32;
    fft_in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    fft_out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    fft_plan = fftw_plan_dft_1d(N, fft_in, fft_out, FFTW_FORWARD, FFTW_ESTIMATE);

    const double PI = 3.141592654;
    size_t fnum = 0;

    for (size_t i = 0; i<(snd_data.num_samples-per_frame); i+= per_frame) {
        
        for (size_t j=0; j< N;++j) {
            double tmp = get_sample(&snd_data, i, 0) / (double)maxVal;
            /* fft_in[j]= tmp * 0.5 * (1-cos((2.0*PI*j)/(N-1))); */
            fft_in[j] = tmp;
        }
        
        fftw_execute(fft_plan);
        /* int32_t sval = average_between(&snd_data, i, i+per_frame); */
        /* int32_t sval = get_sample(&snd_data, i, 0); */
        double mag = 0.0;
        /* for (size_t j = 0; j< N/2; ++j) { */
        mag += cabs(fft_out[0]);
        /* } */
        /* double rval = 1.0+mag/(N/2); */
        double rval = mag + 1.0;
        printf("Calling doFrame(%lu, %f, %s) of %lu\n", fnum,rval,argv[2], (snd_data.num_samples-per_frame)/per_frame);
        fnum += 1;
        doFrame(fnum, rval, argv[2]);
    }

    RiEnd();
    fftw_destroy_plan(fft_plan);
    fftw_free(fft_in);
    fftw_free(fft_out);
    free(snd_data.samples);

    return 0;
}


