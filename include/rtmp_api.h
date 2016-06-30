/*
 * =====================================================================================
 *
 *    Filename   :  rtmp_api.h
 *    Description:
 *    Version    :  1.0
 *    Created    :  2016年06月28日 18时41分47秒
 *    Revision   :  none
 *    Compiler   :  gcc
 *    Author     :  peter-s (), peter_future@outlook.com
 *    Company    :  dt
 *
 * =====================================================================================
 */

#ifndef RTMP_API_H
#define RTMP_API_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct rtmp_para {
    char uri[1024];
    char ip[100];
    int port;
    int write_enable;
};

struct rtmp_context {
    struct rtmp_para para;
    void *rtmp;
};

struct rtmp_context *rtmp_open(struct rtmp_para *para);
int rtmp_read(struct rtmp_context *handle, uint8_t *data, int size);
int rtmp_write(struct rtmp_context *handle, uint8_t *data, int size);
int rtmp_pause(struct rtmp_context *handle, int time);
int rtmp_set_parameter();
int rtmp_get_parameter();
int rtmp_close(struct rtmp_context *handle);

#endif
