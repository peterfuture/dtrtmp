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
#define VIDEO_SIZE 3 *1024 *1024
#define AUDIO_SIZE 1 *1024 *1024
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
    log_print(TAG, "Total:%u %02x %02x %02x %02x\n", total, buf[0], buf[1], buf[2], buf[3]);
    while (total > 4) {
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
    int audio_support = 0;
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
#if 0
    ret = rtmp_write(rtmp_handle, buf_264, total_264);
    ret = rtmp_write(rtmp_handle, buf_aac, total_aac);
    log_print(TAG, "send audio size:%u video size:%u \n", total_aac, total_264);
    return 0;
#endif
    uint8_t *abuf_start = buf_aac;
    uint8_t *abuf_end = buf_aac + total_aac;
    uint8_t *abuf_cur = buf_aac;

    uint8_t *vbuf_start = buf_264;
    uint8_t *vbuf_end = buf_264 + total_264;
    uint8_t *vbuf_cur = buf_264;
    uint8_t *vbuf_off = buf_264;

    uint8_t *nal, *nal_pps, *nal_frame, *nal_next;
    uint32_t nal_len, nal_pps_len, nal_frame_len;

    // parse av packet & send rtmp packet
    while (1) {
        // process one audio frame
        if (!audio_support) {
            goto video_process;
        }

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
            //nal_sps = get_nal(&nal_sps_len, &vbuf_off, vbuf_start, total_264);
            nal_pps = h264_find_NAL(vbuf_off, vbuf_start + total_264 - vbuf_off);
            nal_len = nal_pps - nal - 4;
            log_print(TAG, "nal: %02x %d pos:%d\n", nal[4], nal_len, nal - vbuf_start);
            vbuf_off = nal_pps + 3;
            nal_frame = h264_find_NAL(vbuf_off, vbuf_start + total_264 - vbuf_off);
            nal_pps_len = nal_frame - nal_pps - 4;
            log_print(TAG, "pps: %02x %d pos:%d \n", nal_pps[4], nal_pps_len, nal_pps - vbuf_start);
            vbuf_off = nal_frame + 3;
            nal_next = h264_find_NAL(vbuf_off, vbuf_start + total_264 - vbuf_off);
            nal_frame_len = nal_next - nal_frame - 4;
            log_print(TAG, "frame: %02x %d pos:%d \n", nal_frame[4], nal_frame_len, nal_frame - vbuf_start);
            vbuf_off = nal_next;

            int packet_size = nal_len + nal_pps_len + nal_frame_len + 12;
            log_print(TAG, "packetsize: %d\n", packet_size);

            struct flvmux_packet in, out;
            memset(&in, 0, sizeof(struct flvmux_packet));
            memset(&out, 0, sizeof(struct flvmux_packet));
            in.data = nal;
            in.size = packet_size;
                
            ret = flvmux_setup_video_frame(flv_handle, &in, &out);
            if (ret > 0) {
                log_print(TAG, "Start send data :%d %02x %02x %02x %02x %02x\n", ret, out.data[0], out.data[1], out.data[2], out.data[out.size -1], out.data[out.size - 2]);
                ret = rtmp_write(rtmp_handle, out.data, (int)out.size);
                free(out.data);
            }
        } else {
            nal_next = h264_find_NAL(vbuf_off, vbuf_start + total_264 - vbuf_off);
            nal_frame_len = nal_next - nal - 4;
            vbuf_off = nal_next;

            int packet_size = nal_frame_len + 4;
            log_print(TAG, "packetsize: %d\n", packet_size);

            struct flvmux_packet in, out;
            memset(&in, 0, sizeof(struct flvmux_packet));
            memset(&out, 0, sizeof(struct flvmux_packet));
            in.data = nal;
            in.size = packet_size;
            ret = flvmux_setup_video_frame(flv_handle, &in, &out);
            if (ret > 0) {
                log_print(TAG, "Start send data :%d %02x %02x %02x %02x %02x\n", ret, out.data[0], out.data[1], out.data[2], out.data[out.size -1], out.data[out.size - 2]);
                ret = rtmp_write(rtmp_handle, out.data, ret);
            }

        }
end:
        usleep(1000000);
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
