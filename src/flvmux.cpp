/*
 * =====================================================================================
 *
 *    Filename   :  flv_mux.cpp
 *    Description:  
 *    Version    :  1.0
 *    Created    :  2016��06��30�� 13ʱ00��09��
 *    Revision   :  none
 *    Compiler   :  gcc
 *    Author     :  peter-s
 *    Email      :  peter_future@outlook.com
 *    Company    :  dt
 *
 * =====================================================================================
 */

#include "flvmux_api.h"
#include "rtmp.h"

#include "log_print.h"
#define TAG "FLVMUX"

#define AAC_ADTS_HEADER_SIZE 7
#define FLV_TAG_HEAD_LEN 11
#define FLV_PRE_TAG_LEN 4

// @brief start publish
// @param [in] rtmp_sender handler
// @param [in] flag        stream falg
// @param [in] ts_us       timestamp in us
// @return             : 0: OK; others: FAILED
static const AVal av_onMetaData = AVC("onMetaData");
static const AVal av_duration = AVC("duration");
static const AVal av_width = AVC("width");
static const AVal av_height = AVC("height");
static const AVal av_videocodecid = AVC("videocodecid");
static const AVal av_avcprofile = AVC("avcprofile");
static const AVal av_avclevel = AVC("avclevel");
static const AVal av_videoframerate = AVC("videoframerate");
static const AVal av_audiocodecid = AVC("audiocodecid");
static const AVal av_audiosamplerate = AVC("audiosamplerate");
static const AVal av_audiochannels = AVC("audiochannels");
static const AVal av_avc1 = AVC("avc1");
static const AVal av_mp4a  = AVC("mp4a");
static const AVal av_onPrivateData = AVC("onPrivateData");
static const AVal av_record = AVC("record");

struct flvmux_context *flvmux_open(struct flvmux_para *para)
{
    struct flvmux_context *handle = (struct flvmux_context *)malloc(sizeof(struct flvmux_context));
    if(!handle)
        return NULL;
    memset(handle, 0, sizeof(struct flvmux_context));
    
    memcpy(&handle->para, para, sizeof(struct flvmux_para));
    int64_t ts_us = 0;
   
#if 0 // Used for save FLV File 
    // setup FLV Header
    char flv_file_header[] = "FLV\x1\x5\0\0\0\x9\0\0\0\0"; // have audio and have video
    if(para->has_video && para->has_audio)
        flv_file_header[4] = 0x05;
    else if (para->has_audio && !para->has_video)
        flv_file_header[4] = 0x04;
    else if (!para->has_audio && para->has_video)
        flv_file_header[4] = 0x01;
    else
        flv_file_header[4] = 0x00;
    
    memcpy(handle->header, flv_file_header, 13);
    handle->header_size += 13;
#endif
    
    // setup flv header
    uint32_t body_len;
    uint32_t offset = 0;
    uint32_t output_len;
    char buffer[48];
    char *output = buffer; 
    char *outend = buffer + sizeof(buffer); 
    char send_buffer[512];

    output = AMF_EncodeString(output, outend, &av_onMetaData);
    *output++ = AMF_ECMA_ARRAY;
    output = AMF_EncodeInt32(output, outend, 1);
    output = AMF_EncodeNamedNumber(output, outend, &av_duration, 0.0);
    body_len = output - buffer;
    output_len = body_len + FLV_TAG_HEAD_LEN + FLV_PRE_TAG_LEN;
    send_buffer[offset++] = 0x12; //tagtype scripte 
    send_buffer[offset++] = (uint8_t)(body_len >> 16); //data len
    send_buffer[offset++] = (uint8_t)(body_len >> 8); //data len
    send_buffer[offset++] = (uint8_t)(body_len); //data len
    send_buffer[offset++] = (uint8_t)(ts_us >> 16); //time stamp
    send_buffer[offset++] = (uint8_t)(ts_us >> 8); //time stamp
    send_buffer[offset++] = (uint8_t)(ts_us); //time stamp
    send_buffer[offset++] = (uint8_t)(ts_us >> 24); //time stamp
    send_buffer[offset++] = 0x00; //stream id 0
    send_buffer[offset++] = 0x00; //stream id 0
    send_buffer[offset++] = 0x00; //stream id 0

    memcpy(send_buffer + offset, buffer, body_len); //H264 sequence parameter set
    memcpy(handle->header + handle->header_size, send_buffer, output_len);
    handle->header_size += output_len;

    log_print(TAG, "FLVMUX Open ok\n");
    return handle;
}

int flvmux_setup_audio_frame(struct flvmux_context *handle, struct flvmux_packet *in, struct flvmux_packet *out)
{
    return 0;
}

static uint8_t *h264_find_IDR_frame(char *buffer, int total)
{
    uint8_t *buf = (uint8_t *)buffer;
    while(total > 4){
        if (buf[0]==0x00 && buf[1]==0x00 && buf[2]==0x01) {
            // Found a NAL unit with 3-byte startcode
            if(buf[3] & 0x1F == 0x5) {
                // Found a reference frame, do something with it
            }
            break;
        }
        else if (buf[0]==0x00 && buf[1]==0x00 && buf[2]==0x00 && buf[3]==0x01) {
            // Found a NAL unit with 4-byte startcode
            if(buf[4] & 0x1F == 0x5) {
                // Found a reference frame, do something with it
            }
            break;
        }
        buf++;
        total--;
    }

    if(total <= 4)
        return NULL;
    return buf;
}

static uint8_t *h264_find_NAL(uint8_t *buffer, int total)
{
    uint8_t *buf = (uint8_t *)buffer;
    while(total > 4){
#if 0
        if (buf[0]==0x00 && buf[1]==0x00 && buf[2]==0x01) {
            // Found a NAL unit with 3-byte startcode
            if(buf[3] & 0x1F == 0x5) {
                // Found a reference frame, do something with it
            }
            break;
        }
        else 
#endif
        if (buf[0]==0x00 && buf[1]==0x00 && buf[2]==0x00 && buf[3]==0x01) {
            // Found a NAL unit with 4-byte startcode
            if(buf[4] & 0x1F == 0x5) {
                // Found a reference frame, do something with it
            }
            break;
        }
        buf++;
        total--;
    }

    if(total <= 4)
        return NULL;
    return buf;
}

// Complete Frame Process
int flvmux_setup_video_frame(struct flvmux_context *handle, struct flvmux_packet *in, struct flvmux_packet *out)
{
    int sps_len = 0;
    uint8_t *sps_buf = NULL;
    int frame_len = 0;
    uint8_t *frame_buf = NULL;

    uint8_t *buf_264 = (uint8_t *)in->data;
    uint32_t total_264 = (uint32_t)in->size;
    uint8_t *vbuf_start = buf_264;
    uint8_t *vbuf_end = buf_264 + total_264;
    uint8_t *vbuf_cur = buf_264;
    uint8_t *vbuf_off = buf_264;

    uint8_t *nal, *nal_pps, *nal_frame, *nal_next;
    uint32_t nal_len, nal_pps_len, nal_frame_len;

    uint8_t *output = NULL;
    uint32_t body_len;
    uint32_t output_len;

    uint32_t ts = (uint32_t)in->pts;
    uint32_t offset = 0;

PARSE_BEGIN:
    // Find Nal. Maybe SPS
    log_print(TAG, "Curpos:%d %02x %02x %02x %02x %02x\n", vbuf_off - vbuf_start, vbuf_off[0], vbuf_off[1], vbuf_off[2], vbuf_off[3], vbuf_off[4]);
    nal = h264_find_NAL(vbuf_off, vbuf_start + total_264 - vbuf_off);
    if (nal == NULL) {
        goto end;
    }
    vbuf_off = nal + 3;
    if (nal[4] == 0x67)  {
        
        if(handle->video_config_ok == 1) {
            log_print(TAG, "I frame configured \n");
            //return -1;
        }

        nal_pps = h264_find_NAL(vbuf_off, vbuf_start + total_264 - vbuf_off);
        nal_len = nal_pps - nal - 4;
        log_print(TAG, "nal: %02x %d pos:%d\n", nal[4], nal_len, nal - vbuf_start);
        vbuf_off = nal_pps + 3;
        nal_frame = h264_find_NAL(vbuf_off, vbuf_start + total_264 - vbuf_off);
        nal_pps_len = nal_frame - nal_pps - 4;
        log_print(TAG, "pps: %02x %d pos:%d \n", nal_pps[4], nal_pps_len, nal_pps - vbuf_start);
        vbuf_off = nal_frame;

        body_len = nal_len + nal_pps_len + 16;
        output_len = body_len + FLV_TAG_HEAD_LEN + FLV_PRE_TAG_LEN;
        output = (uint8_t *)malloc(output_len);

        // FLV TAG HEADER
        output[offset++] = 0x09; //tagtype video
        output[offset++] = (uint8_t)(body_len >> 16); //data len
        output[offset++] = (uint8_t)(body_len >> 8); //data len
        output[offset++] = (uint8_t)(body_len); //data len
        output[offset++] = (uint8_t)(ts >> 16); //time stamp
        output[offset++] = (uint8_t)(ts >> 8); //time stamp
        output[offset++] = (uint8_t)(ts); //time stamp
        output[offset++] = (uint8_t)(ts >> 24); //time stamp
        output[offset++] = in->pts; //stream id 0
        output[offset++] = 0x00; //stream id 0
        output[offset++] = 0x00; //stream id 0

        //FLV VideoTagHeader
        output[offset++] = 0x17; //key frame, AVC
        output[offset++] = 0x00; //avc sequence header
        output[offset++] = 0x00; //composit time
        output[offset++] = 0x00; // composit time
        output[offset++] = 0x00; //composit time

        //flv VideoTagBody --AVCDecoderCOnfigurationRecord
        output[offset++] = 0x01; //configurationversion
        output[offset++] = nal[1]; //avcprofileindication
        output[offset++] = nal[2]; //profilecompatibilty
        output[offset++] = nal[3]; //avclevelindication
        output[offset++] = 0xff; //reserved + lengthsizeminusone
        output[offset++] = 0xe1; //numofsequenceset
        output[offset++] = (uint8_t)(nal_len >> 8); //sequence parameter set length high 8 bits
        output[offset++] = (uint8_t)(nal_len); //sequence parameter set  length low 8 bits
        memcpy(output + offset, nal + 4, nal_len); //H264 sequence parameter set
        offset += nal_len;
        output[offset++] = 0x01; //numofpictureset
        output[offset++] = (uint8_t)(nal_pps_len >> 8); //picture parameter set length high 8 bits
        output[offset++] = (uint8_t)(nal_pps_len); //picture parameter set length low 8 bits
        memcpy(output + offset, nal_pps + 4, nal_pps_len); //H264 picture parameter set

        offset += nal_pps_len;
        uint32_t fff = body_len + FLV_TAG_HEAD_LEN;
        output[offset++] = (uint8_t)(fff >> 24); //data len
        output[offset++] = (uint8_t)(fff >> 16); //data len
        output[offset++] = (uint8_t)(fff >> 8); //data len
        output[offset++] = (uint8_t)(fff); //data len

        sps_len = output_len;
        sps_buf = output;
#if 0
        out->size = output_len;
        out->data = (uint8_t *)malloc(output_len);
        memcpy(out->data, output, output_len);
        free(output);
#endif
        handle->video_config_ok = 1;
        goto PARSE_BEGIN;
    } else if (nal[4] == 0x65) {
        nal_len = vbuf_end - nal - 4;
        log_print(TAG, "nal len:%d \n", nal_len); 
        body_len = nal_len + 5 + 4; //flv VideoTagHeader +  NALU length
        output_len = body_len + FLV_TAG_HEAD_LEN + FLV_PRE_TAG_LEN;
        output =(uint8_t *)malloc(output_len);
        if (!output)
            return -1;

        // FLV TAG HEADER
        output[offset++] = 0x09; //tagtype video
        output[offset++] = (uint8_t)(body_len >> 16); //data len
        output[offset++] = (uint8_t)(body_len >> 8); //data len
        output[offset++] = (uint8_t)(body_len); //data len
        output[offset++] = (uint8_t)(ts >> 16); //time stamp
        output[offset++] = (uint8_t)(ts >> 8); //time stamp
        output[offset++] = (uint8_t)(ts); //time stamp
        output[offset++] = (uint8_t)(ts >> 24); //time stamp
        output[offset++] = in->pts; //stream id 0
        output[offset++] = 0x00; //stream id 0
        output[offset++] = 0x00; //stream id 0

        //FLV VideoTagHeader
        output[offset++] = 0x17; //key frame, AVC
        output[offset++] = 0x01; //avc sequence header
        output[offset++] = 0x00; //composit time
        output[offset++] = 0x00; // composit time
        output[offset++] = 0x00; //composit time

        output[offset++] = (uint8_t)(nal_len >> 24); //nal length
        output[offset++] = (uint8_t)(nal_len >> 16); //nal length
        output[offset++] = (uint8_t)(nal_len >> 8); //nal length
        output[offset++] = (uint8_t)(nal_len); //nal length
        memcpy(output + offset, nal, nal_len);

        offset += nal_len;
        uint32_t fff = body_len + FLV_TAG_HEAD_LEN;
        output[offset++] = (uint8_t)(fff >> 24); //data len
        output[offset++] = (uint8_t)(fff >> 16); //data len
        output[offset++] = (uint8_t)(fff >> 8); //data len
        output[offset++] = (uint8_t)(fff); //data len

        frame_len = output_len;
        frame_buf = output;
#if 0
        out->size = output_len;
        out->data = (uint8_t *)malloc(output_len);
        memcpy(out->data, output, output_len);
        free(output);
#endif
    }

end:
    out->pts = in->pts;
    out->dts = in->dts;
    out->size = frame_len + sps_len;
    out->data = (uint8_t *)malloc(out->size);
    if(!out->data)
        return -1;

    if(sps_len > 0) {
        memcpy(out->data, sps_buf, sps_len);
        free(sps_buf);
    }
    if(frame_len > 0) {
        memcpy(out->data + sps_len, frame_buf, frame_len);
        free(frame_buf);
    }
    log_print(TAG, "sps:%d frame:%d \n", sps_len, frame_len);
    return out->size;
}

int flvmux_close(struct flvmux_context *handle)
{
    if(handle)
        free(handle);
    return 0;
}

