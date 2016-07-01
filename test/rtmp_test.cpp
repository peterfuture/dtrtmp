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

// Red5 RTMP Server
#define RTMP_LIVE_ADDR "rtmp://127.0.0.1:1935/live/test"

int main()
{
    int audio_support = 0;
    int video_support = 1;
    struct rtmp_para rtmp_para;
    memset(&rtmp_para, 0, sizeof(struct rtmp_para));
    rtmp_para.write_enable = 1;
    strcpy(rtmp_para.uri, RTMP_LIVE_ADDR);
    struct rtmp_context *rtmp_handle = rtmp_open(&rtmp_para);
    if(!rtmp_handle)
        return -1;

    struct flvmux_para flv_para;
    memset(&flv_para, 0, sizeof(struct flvmux_para));
    flv_para.has_audio = audio_support;
    flv_para.has_video = video_support;
    struct flvmux_context *flv_handle = flvmux_open(&flv_para);
    if(!flv_handle)
        return -1;

    int fd_264 = open("264.dat", O_RDONLY);
    uint8_t * buf_264 = (uint8_t *)malloc(3 *1024 * 1024);
    int total_264 = read(fd_264, buf_264, (1024*1024 *3));
    close(fd_264);
    int fd_aac = open("aac.dat", O_RDONLY);
    uint8_t * buf_aac = (uint8_t *)malloc(3 *1024 * 1024);
    int total_aac = read(fd_264, buf_264, (1024*1024 *3));
    close(fd_aac);

    uint8_t *abuf_start = buf_aac;
    uint8_t *abuf_end = buf_aac + total_aac;
    uint8_t *abuf_cur = buf_aac;

    uint8_t *vbuf_start = buf_264;
    uint8_t *vbuf_end = buf_264 + total_264;
    uint8_t *vbuf_cur = buf_264;

    int ret;
    // First Send FLV Header
    ret = rtmp_write(rtmp_handle, flv_handle->header, flv_handle->header_size);
    if(ret < 0)
        return -1;

    // parse av packet & send rtmp packet
    while(1) {
        // process one audio frame
        if(audio_support) {

        }
        // process one video frame
        if(video_support) {

        }

    }


Finish:
    rtmp_close(rtmp_handle);
    flvmux_close(flv_handle);
    free(buf_264);
    free(buf_aac);
    return 0;
}
