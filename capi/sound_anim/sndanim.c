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

void show_audio_info(audio_data_t *data) {
    printf("Buffer size: %lu\n"
           "Used buffer: %lu\n"
           "Num Samples: %lu\n"
           "Sample size: %lu\n"
           "Sample rate: %lu\n"
           "Channels   : %d\n"
           "Duration   : %5.2f\n"
           "Planar     : %d\n",
           data->buffer_size,
           data->used_buffer_size,
           data->num_samples,
           data->sample_size,
           data->sample_rate,
           data->channels,
           data->duration,
           data->planar);

}
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
             size_t cur, int fft_size, fftw_complex *fft_data[],
             char *fName) {

    RiFrameBegin(fNum);

    char buffer[256];
    sprintf(buffer, "images/%s%05d.tif", fName, fNum);
    RiDisplay(buffer,(char*)"file",(char*)"rgba",RI_NULL);
  
    RiFormat(800, 600,  1.25);
    RiLightSource((char*)"distantlight",RI_NULL);
    RiProjection((char*)"perspective",RI_NULL);
  
    RiTranslate(0.0,0.0,0.8*fft_size);
    /* RiTranslate(0.0,0.0,15); */
    RiRotate( -120.0, 1.0, 0.0, 0.0);
    RiRotate(90.0, 0.0,0.0, 1.0);
    /* RiRotate( 45.0, 0.0, 1.0, 0.0); */
  
    RiWorldBegin();
  
    /* RiRotate(fNum, 0.0,1.0,0.0); */

    RiSurface((char*)"matte", RI_NULL);
    /* RiSphere(rval, -rval, rval, 360, RI_NULL); */

    RiTranslate(-fft_size/2.0, -fft_size/2.0+fft_size/4.0, 0);
    
    size_t real_i = cur;
    RtPoint *pts = malloc(sizeof(RtPoint)*(fft_size*fft_size/2));
    RtColor *colors = malloc(sizeof(RtColor)*(fft_size*fft_size/2));
    RtInt *numCurves = malloc(sizeof(RtInt)*fft_size);
    /* double *widths = malloc(sizeof(double)*fft_size); */
    size_t cp = 0;
    for (int i=fft_size-1; i>=0; --i) {
        real_i += 1;
        if (real_i == fft_size) {
            real_i = 0;
        }
        numCurves[i] = fft_size/2;
        /* for (int i=0; i<fft_size-1; ++i) { */
        for (size_t j=0; j<fft_size/2; ++j) {

            colors[cp][0] = real_i/(double)(fft_size-1);
            colors[cp][1] = cabs(fft_data[real_i][j]);
            if (colors[cp][1]>1.0) { colors[cp][1] = 1.0; }
            colors[cp][2] = j/(double)(fft_size/2);
            
            pts[cp][0] = fft_size-i;
            pts[cp][2] = cabs(fft_data[real_i][j]);
            pts[cp][1] = j;
            
            /* widths[cp] = 0.1; */
            /* RiPolygon( 3, "P", (RtPointer)pts, "Cs", (RtPointer)colors, RI_NULL ); */
            cp += 1;
        }
    }
    
    RiCurves( "linear", fft_size, numCurves, "nonperiodic", "P", (RtPointer)pts, "Cs", (RtPointer)colors, RI_NULL );

    /* free(widths); */
    free(numCurves);
    free(colors);
    free(pts);

    /* RtInt num_polys = (2*fft_size)*(fft_size/2); */
    /* RtInt *num_verts =malloc(sizeof(RtInt)*num_polys); */
    /* RtInt total = 0; */
    /* for (size_t i = 0; i< num_polys; ++i) { */
    /*     num_verts[i] = 3; */
    /*     total +=3; */
    /* } */
    /* RtInt *verts  = malloc(sizeof(RtInt)*total); */
    /* for (size_t i = 0; i<total; i+=3) { */
    /*     if ((i %2) == 0) { */
    /*         verts[i]=i; */
    /*         verts[i+1]=i+1; */
    /*         verts[i+2]=i+fft_size; */
    /*     } else { */
    /*         verts[i]=i+1; */
    /*         verts[i+1]=i+fft_size+1; */
    /*         verts[i+2]=i+fft_size; */
    /*     } */
    /* } */

    /* printf("Rendering %d polygons\n", num_polys); */
    /* /\* RiPointsPolygons(num_polys, num_verts, verts, "P", pts, "Cs", colors, RI_NULL); *\/ */
    /* free(verts); */
    /* free(num_verts); */
    /* free(pts); */
    /* free(colors); */

    RiWorldEnd();
    RiFrameEnd();
}

int main(int argc, char *argv[]) {
    if (argc<2) {
        printf("No file names given.\n");
        printf("Use:\n\t%s audio_file output_prefix\n\n", argv[0]);
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
        maxVal = 1<<28;
    }
    const int N = 120;

    fftw_complex *fft_in  __attribute__ ((aligned (16)));
    fftw_complex **fft_out  __attribute__ ((aligned (16)));

    fftw_plan fft_plan  __attribute__ ((aligned (16)));

    fft_in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    fft_out = (fftw_complex**) fftw_malloc(sizeof(fftw_complex*) * N);

    for (int i=0; i<N; ++i) {
        fft_out[i] = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    }

    fft_plan = fftw_plan_dft_1d(N, fft_in, fft_out[0], FFTW_FORWARD, FFTW_ESTIMATE);

    size_t fnum, cur_out;

    for (size_t i=0; i<N; ++i) {
        fft_in[i] = 0.0;
        for (size_t j=0; j<N; ++j) {
            fft_out[i][j] = 0.0;
        }
    }
    show_audio_info(&snd_data);
    
    RiBegin(RI_NULL);
    
    size_t num_frames = (snd_data.num_samples-per_frame)/per_frame;
    for (size_t i = 0, cur_out = 0, fnum = 1; i<(snd_data.num_samples-per_frame); i+= per_frame, ++fnum) {

        size_t j_size = N/2;
        size_t stp = per_frame/(j_size);
        size_t ci = per_frame*15+i;
        for (size_t j=0; j< j_size;++j) {
            
            fft_in[j] = (double)get_sample(&snd_data, ci, 0)/(double)maxVal;
            ci += stp;
        }

        fftw_execute_dft(fft_plan, fft_in, fft_out[cur_out]);

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
        fftw_free(fft_out[i]);
    }
    fftw_destroy_plan(fft_plan);
    fftw_free(fft_out);
    fftw_free(fft_in);
    fftw_cleanup();
    free(snd_data.samples);

    return 0;
}


