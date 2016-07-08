/*
 * =====================================================================================
 *
 *    Filename   :  rtmp_api.c
 *    Description:
 *    Version    :  1.0
 *    Created    :  2016年06月28日 18时30分27秒
 *    Revision   :  none
 *    Compiler   :  gcc
 *    Author     :  peter-s
 *    Email      :  peter_future@outlook.com
 *    Company    :  dt
 *
 * =====================================================================================
 */

#include "rtmp.h"
#include "rtmp_api.h"
#include "flvmux_api.h"

#include "log_print.h"
#define TAG "RTMP"

struct rtmp_context *rtmp_open(struct rtmp_para *para)
{
    struct rtmp_context *handle = (struct rtmp_context *)malloc(sizeof(struct rtmp_context));
    if (!handle) {
        return NULL;
    }
    memset(handle, 0, sizeof(struct rtmp_context));
    memcpy(&handle->para, para, sizeof(struct rtmp_para));
    RTMP *rtmp = RTMP_Alloc();
    if (!rtmp) {
        free(handle);
        return NULL;
    }
    RTMP_Init(rtmp);
    int ret = RTMP_SetupURL(rtmp, para->uri);
    if (!ret) {
        goto fail;
    }
    if (para->write_enable) {
        RTMP_EnableWrite(rtmp);
    }
    ret = RTMP_Connect(rtmp, NULL);
    if (!ret) {
        goto fail;
    }
    ret = RTMP_ConnectStream(rtmp, 0);
    if (!ret) {
        goto fail;
    }
    handle->rtmp = (void *)rtmp;
    log_print(TAG, "Rtmp open fail\n");
    return handle;
fail:

    RTMP_Free(rtmp);
    free(handle);
    log_print(TAG, "Rtmp open fail\n");
    return NULL;
}

int rtmp_read(struct rtmp_context *handle, uint8_t *data, int size)
{
    return RTMP_Read((struct RTMP*)handle->rtmp, (char *)data, size);
}

int rtmp_write(struct rtmp_context *handle, uint8_t *data, int size)
{
    int ret = 0;
    int wlen = 0;
    while(wlen < size) {
        ret = RTMP_Write((struct RTMP*)handle->rtmp, (char *)(data + wlen), size - wlen);
        if(ret > 0)
            wlen += ret;
    }
    return wlen;
}

int rtmp_pause(struct rtmp_context *handle, int time)
{
    return RTMP_Pause((struct RTMP*)handle->rtmp, time);
}

int rtmp_set_parameter()
{
    return 0;
}

int rtmp_get_parameter()
{
    return 0;
}

int rtmp_close(struct rtmp_context *handle)
{
    RTMP *rtmp = (struct RTMP *)handle->rtmp;
    RTMP_Close(rtmp);
    RTMP_Free(rtmp);
    free(handle);

    return 0;
}


