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

#include "rtmp_api.h"
#include "flvmux_api.h"

int main()
{
    struct rtmp_para rtmp_para;
    memset(&rtmp_para, 0, sizeof(struct rtmp_para));
    rtmp_para.write_enable = 1;
    strcpy(rtmp_para.uri, "rtmp://127.0.0.1:1935/live/test");
    struct rtmp_context *rtmp_handle = rtmp_open(&rtmp_para);
    struct flvmux_para flv_para;
    memset(&flv_para, 0, sizeof(struct flvmux_para));
    flv_para.has_video = 1;
    struct flvmux_context *flv_handle = flvmux_open(&flv_para);
    rtmp_close(rtmp_handle);
    flvmux_close(flv_handle);
    return 0;
}
