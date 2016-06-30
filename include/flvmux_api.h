/*
 * =====================================================================================
 *
 *    Filename   :  flv_mux.h
 *    Description:  
 *    Version    :  1.0
 *    Created    :  2016年06月30日 13时15分35秒
 *    Revision   :  none
 *    Compiler   :  gcc
 *    Author     :  peter-s (), peter_future@outlook.com
 *    Company    :  dt
 *
 * =====================================================================================
 */

#ifndef FLVMUX_H
#define FLVMUX_H

#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <string.h>

struct flvmux_packet {
    uint8_t *data;
    int size;
    int64_t pts;
    int64_t dts;
};

struct flvmux_para {
    int hav_audio;
    int has_video;
    int afmt;
    int vfmt;
};

struct flvmux_context {
    struct flvmux_para para;
    char header[1024];
    int header_size;
    int video_config_ok;
};

struct flvmux_context *flvmux_open(struct flvmux_para *para);
int flvmux_setup_audio_frame(struct flvmux_context *handle, struct flvmux_packet *in, struct flvmux_packet *out);
int flvmux_setup_video_frame(struct flvmux_context *handle, struct flvmux_packet *in, struct flvmux_packet *out);
int flvmux_close(struct flvmux_context *handle);

#endif
