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

int main()
{
    struct rtmp_para para;
    memset(&para, 0, sizeof(struct rtmp_para));
    para.write_enable = 1;
    strcpy(para.uri, "rtmp://127.0.0.1:1935/live/test");
    struct rtmp_context *handle = rtmp_open(&para);
    rtmp_close(handle);
    return 0;
}
