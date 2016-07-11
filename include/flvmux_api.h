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
    uint32_t size;
    int64_t pts;
    int64_t dts;

    int type; // 0-v 1-a
};

struct flvmux_para {
    int has_audio;
    int has_video;
    int afmt;
    int vfmt;
};

#define AAC_ADTS_HEADER_SIZE 7
#define FLV_TAG_HEAD_LEN 11
#define FLV_PRE_TAG_LEN 4
struct AudioSpecificConfig {
    uint8_t audio_object_type;
    uint8_t sample_frequency_index;
    uint8_t channel_configuration;
};

struct flvmux_context {
    struct flvmux_para para;
    // audio
    int audio_config_ok;
    struct AudioSpecificConfig config;
    // video 
    uint8_t header[1024];
    int header_size;
    int video_config_ok;
};

struct flvmux_context *flvmux_open(struct flvmux_para *para);
int flvmux_setup_audio_frame(struct flvmux_context *handle, struct flvmux_packet *in, struct flvmux_packet *out);
int flvmux_setup_video_frame(struct flvmux_context *handle, struct flvmux_packet *in, struct flvmux_packet *out);
int flvmux_close(struct flvmux_context *handle);

#endif
