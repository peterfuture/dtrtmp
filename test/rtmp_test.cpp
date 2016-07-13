/*
 * =====================================================================================
 *
 *    Filename   :  rtmp_test.cpp
 *    Description:
 *    Version    :  1.0
 *    Created    :  2016年06月28日 18时21分49秒
 *    Revision   :  none
 *    Compiler   :  gcc
 *    Author     :  peter-s
 *    Email      :  peter_future@outlook.com
 *    Company    :  dt
 *
 * =====================================================================================
 */

#include <fcntl.h>
#include <unistd.h>

#include "rtmp_api.h"
#include "flvmux_api.h"

#include "log_print.h"
#define TAG "RTMP-TEST"

// Red5 RTMP Server
#define RTMP_LIVE_ADDR "rtmp://127.0.0.1:1935/live/test"
#define VIDEO_SIZE 10 *1024 *1024
#define AUDIO_SIZE 5*1024*1024

#define AAC_ADTS_HEADER_SIZE 7
uint8_t *get_adts(uint32_t *len, uint8_t **offset, uint8_t *start, uint32_t total)
{
    uint8_t *p  =  *offset;
    uint32_t frame_len_1;
    uint32_t frame_len_2;
    uint32_t frame_len_3;
    uint32_t frame_length;
   
    if (total < AAC_ADTS_HEADER_SIZE) {
        return NULL;
    }
    if ((p - start) >= total) {
        return NULL;
    }
    
    if (p[0] != 0xff) {
        return NULL;
    }
    if ((p[1] & 0xf0) != 0xf0) {
        return NULL;
    }
    frame_len_1 = p[3] & 0x03;
    frame_len_2 = p[4];
    frame_len_3 = (p[5] & 0xe0) >> 5;
    frame_length = (frame_len_1 << 11) | (frame_len_2 << 3) | frame_len_3;
    *offset = p + frame_length;
    *len = frame_length;
    return p;
}

static uint32_t find_start_code(uint8_t *buf, uint32_t zeros_in_startcode)
{
    uint32_t info;
    uint32_t i;

    info = 1;
    if ((info = (buf[zeros_in_startcode] != 1) ? 0 : 1) == 0) {
        return 0;
    }

    for (i = 0; i < zeros_in_startcode; i++)
        if (buf[i] != 0) {
            info = 0;
            break;
        };

    return info;
}

uint8_t * get_nal(uint32_t *len, uint8_t **offset, uint8_t *start, uint32_t total)
{
    uint32_t info;
    uint8_t *q ;
    uint8_t *p  =  *offset;
    *len = 0;

    while (1) {
        info =  find_start_code(p, 3);
        if (info == 1) {
            break;
        }
        p++;
        if ((p - start) >= total) {
            return NULL;
        }
    }
    q = p + 4;
    p = q;
    while (1) {
        info =  find_start_code(p, 3);
        if (info == 1) {
            break;
        }
        p++;
        if ((p - start) >= total) {
            return NULL;
        }
    }

    *len = (p - q);
    *offset = p;
    return q;
}

uint8_t *h264_find_NAL(uint8_t *buffer, uint32_t total)
{
    uint8_t *buf = buffer;
    //log_print(TAG, "Total:%u %02x %02x %02x %02x\n", total, buf[0], buf[1], buf[2], buf[3]);
    while (total > 4) {
#if 0
        if (buf[0] == 0x00 && buf[1] == 0x00 && buf[2] == 0x01) {
            // Found a NAL unit with 3-byte startcode
            if (buf[3] & 0x1F == 0x5) {
                // Found a reference frame, do something with it
            }
            break;
        } else 
#endif
        if (buf[0] == 0x00 && buf[1] == 0x00 && buf[2] == 0x00 && buf[3] == 0x01) {
            // Found a NAL unit with 4-byte startcode
            if (buf[4] & 0x1F == 0x5) {
                // Found a reference frame, do something with it
            }
            break;

        }
        buf++;
        total--;
    }
    if (total <= 4) {
        return NULL;
    }
    return buf;
}

int main()
{
    int ret;
    int audio_support = 1;
    int video_support = 1;
    struct rtmp_para rtmp_para;
    memset(&rtmp_para, 0, sizeof(struct rtmp_para));
    rtmp_para.write_enable = 1;
    strcpy(rtmp_para.uri, RTMP_LIVE_ADDR);
    struct rtmp_context *rtmp_handle = rtmp_open(&rtmp_para);
    if (!rtmp_handle) {
        return -1;
    }
    log_print(TAG, "[%s:%d] Trace\n", __FUNCTION__, __LINE__);

    struct flvmux_para flv_para;
    memset(&flv_para, 0, sizeof(struct flvmux_para));
    flv_para.has_audio = audio_support;
    flv_para.has_video = video_support;
    struct flvmux_context *flv_handle = flvmux_open(&flv_para);
    if (!flv_handle) {
        return -1;
    }
    log_print(TAG, "[%s:%d] Trace\n", __FUNCTION__, __LINE__);

    // First Send FLV Header
    ret = rtmp_write(rtmp_handle, flv_handle->header, flv_handle->header_size);
    if (ret < 0) {
        return -1;
    }
    log_print(TAG, "Header send ok. ret:%d \n", ret);

    int fd_264 = open("out.264", O_RDONLY);
    uint8_t * buf_264 = (uint8_t *)malloc(VIDEO_SIZE);
    uint32_t total_264 = read(fd_264, buf_264, (VIDEO_SIZE));
    close(fd_264);
    int fd_aac = open("out.aac", O_RDONLY);
    uint8_t * buf_aac = (uint8_t *)malloc(AUDIO_SIZE);
    uint32_t total_aac = read(fd_aac, buf_aac, (AUDIO_SIZE));
    close(fd_aac);

    log_print(TAG, "audio size:%u video size:%u \n", total_aac, total_264);

    uint8_t *abuf_start = buf_aac;
    uint8_t *abuf_end = buf_aac + total_aac;
    uint8_t *abuf_off = buf_aac;

    uint32_t len;
    uint32_t audio_frame_len;
    uint8_t *audio_frame_start;

    uint8_t *vbuf_start = buf_264;
    uint8_t *vbuf_end = buf_264 + total_264;
    uint8_t *vbuf_cur = buf_264;
    uint8_t *vbuf_off = buf_264;

    uint8_t *nal, *nal_pps, *nal_frame, *nal_next;
    uint32_t nal_len, nal_pps_len, nal_frame_len;

        
    struct flvmux_packet audio_pkt_in, audio_pkt_out;
    struct flvmux_packet video_pkt_in, video_pkt_out;
    // parse av packet & send rtmp packet
    while (1) {
        // process one audio frame
        if (!audio_support) {
            goto video_process;
        }
        audio_frame_start = get_adts(&audio_frame_len, &abuf_off, abuf_start, abuf_start + total_aac - abuf_off);
        if (audio_frame_start == NULL){
            abuf_off = abuf_start;
            continue;
        }
        memset(&audio_pkt_in, 0, sizeof(struct flvmux_packet));
        memset(&audio_pkt_out, 0, sizeof(struct flvmux_packet));
        audio_pkt_in.data = (uint8_t *)audio_frame_start;
        audio_pkt_in.size = (uint32_t)audio_frame_len;
        ret = flvmux_setup_audio_frame(flv_handle, &audio_pkt_in, &audio_pkt_out);
        if (ret > 0) {
            ret = rtmp_write(rtmp_handle, audio_pkt_out.data, (int)audio_pkt_out.size);
            free(audio_pkt_out.data);
            log_print(TAG, "Send audio apkt ok size:%d ret:%d\n", audio_pkt_out.size, ret);
        } else
            log_print(TAG, "Send audio apkt failed size:%d ret:%d\n", audio_pkt_out.size, ret);
 
video_process:
        // process one video frame
        if (!video_support) {
            goto end;
        }

        //nal = get_nal(&nal_len, &vbuf_off, vbuf_start, total_264);
        nal = h264_find_NAL(vbuf_off, vbuf_start + total_264 - vbuf_off);
        if (nal == NULL) {
            log_print(TAG, "not found nal \n");
            goto end;
        }
        vbuf_off = nal + 4;
        if (nal[4] == 0x67) {
            // Including sps & pps
            nal_pps = h264_find_NAL(vbuf_off, vbuf_start + total_264 - vbuf_off);
            nal_len = nal_pps - nal - 4;
            //log_print(TAG, "nal: %02x %d pos:%d\n", nal[4], nal_len, nal - vbuf_start);
            vbuf_off = nal_pps + 4;
            nal_frame = h264_find_NAL(vbuf_off, vbuf_start + total_264 - vbuf_off);
            nal_pps_len = nal_frame - nal_pps - 4;
            //log_print(TAG, "pps: %02x %d pos:%d \n", nal_pps[4], nal_pps_len, nal_pps - vbuf_start);
            vbuf_off = nal_frame + 4;
            nal_next = h264_find_NAL(vbuf_off, vbuf_start + total_264 - vbuf_off);
            nal_frame_len = nal_next - nal_frame - 4;
            //log_print(TAG, "frame: %02x %d pos:%d \n", nal_frame[4], nal_frame_len, nal_frame - vbuf_start);
            vbuf_off = nal_next;

            int packet_size = nal_len + nal_pps_len + nal_frame_len + 12;
            log_print(TAG, "Frame Including PPS: sps:[%d:%d] pps:[%d:%d]\n", nal - vbuf_start, nal_len, nal_pps - vbuf_start, nal_pps_len, nal_frame - vbuf_start, nal_frame_len);
            //log_print(TAG, "packetsize: %d\n", packet_size);

            memset(&video_pkt_in, 0, sizeof(struct flvmux_packet));
            memset(&video_pkt_out, 0, sizeof(struct flvmux_packet));
            video_pkt_in.data = nal;
            video_pkt_in.size = packet_size;
            
            ret = flvmux_setup_video_frame(flv_handle, &video_pkt_in, &video_pkt_out);
            if (ret > 0) {
                ret = rtmp_write(rtmp_handle, video_pkt_out.data, (int)video_pkt_out.size);
                free(video_pkt_out.data);
            }
        } else if (nal[4] == 0x65 || (nal[4] & 0x1f) == 0x01) {
            
            nal_next = h264_find_NAL(vbuf_off, vbuf_start + total_264 - vbuf_off);
            nal_frame_len = nal_next - nal - 4;
            vbuf_off = nal_next;

            int packet_size = nal_frame_len + 4;
            //log_print(TAG, "Frame without pps: [%d:%d] \n", nal - vbuf_start, nal_frame_len);
            log_print(TAG, "frame with out pps: %02x %d pos:%d \n", nal[4], nal_frame_len, nal - vbuf_start);
            //log_print(TAG, "packetsize: %d\n", packet_size);

            memset(&video_pkt_in, 0, sizeof(struct flvmux_packet));
            memset(&video_pkt_out, 0, sizeof(struct flvmux_packet));
            video_pkt_in.data = nal;
            video_pkt_in.size = packet_size;
            ret = flvmux_setup_video_frame(flv_handle, &video_pkt_in, &video_pkt_out);
            if (ret > 0) {
                ret = rtmp_write(rtmp_handle, video_pkt_out.data, ret);
                free(video_pkt_out.data);
            }
        } else {
            nal_next = h264_find_NAL(vbuf_off, vbuf_start + total_264 - vbuf_off);
            nal_frame_len = nal_next - nal - 4;
            vbuf_off = nal_next;
            log_print(TAG, "SKip frame : %02x %d pos:%d \n", nal[4], nal_frame_len, nal - vbuf_start);
            //log_print(TAG, "data:%02x \n", nal[4]);
        }
end:
        usleep(30*1000);
        continue;
    }


Finish:
    log_print(TAG, "quit \n");
    rtmp_close(rtmp_handle);
    flvmux_close(flv_handle);
    free(buf_264);
    free(buf_aac);
    return 0;
}
