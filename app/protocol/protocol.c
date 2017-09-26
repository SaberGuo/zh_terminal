/*
 * protocol.c
 *
 *  Created on: 2017年9月14日
 *      Author: saber
 */

#include "protocol.h"

#include "app/gsm/gsm.h"
#include "base/clocks/clocks.h"
#include "fatfs/ff.h"

#define RETRY 2

char *PUSH_DATA_HEAD="device_id:30,device_config_id:100,method:push_data,count:5;";
char *PUSH_IMAGE_HEAD="device_id:30,device_config_id:100,method:push_image,key:IMG1,size:%d,ts:%d;";
char *PACKAGE_FORMAT="%s:%.2f,ts:%d;";
char *PUSH_DATA_FINISH="method:push_data_finish;";
char *PUSH_DATA_READY="push_data_ready";
char *PUSH_IMAGE_READY="push_image_ready";
char *DATA_UPLOADED="data_uploaded";
char *FAIL="ERROR";
char *TIME_SECOND="ts:";

uint32_t now_time = 0;

uint8_t protocol_buffer[MAX_TX_BUFFER];

bool upload_data_start(){
    char* p = NULL;
    char* pres = NULL;
    uint8_t i = 0;
    for(;i<RETRY;++i){
        clear_buffer();

        msg_send(PUSH_DATA_HEAD);
        delay_ms(1000);

        if(wait_msg_reved()){
            p = msg_read();
            pres = strstr(p, PUSH_DATA_READY);
            if(*pres){
                pres = strstr(p, TIME_SECOND);
                if(*pres){
                   now_time = atoi(pres+strlen(TIME_SECOND));
                }
                return true;
            }
        }
    }
    return false;
}

bool upload_image_start(){
    FATFS fatfs_t;
    FRESULT res;
    FIL fir;
    uint32_t bw;
    uint16_t package=0;
    uint16_t package_total=0;
    char* p = NULL;
    char* pres = NULL;
    uint8_t i = 0;
    for(;i<RETRY;++i){
        clear_buffer();

        res=f_mount(&fatfs_t, "", 1);       //立即挂载
        if(res==FR_OK){
            res=f_open(&fir, "0:/TIMG.JPG", FA_OPEN_ALWAYS|FA_READ);
            if(res == FR_OK){
                FSIZE_t size=fir.obj.objsize;
                UINT num=1024;
                if(size%1024 == 0){
                    package_total=size/num;
                    sprintf((char *)protocol_buffer, PUSH_IMAGE_HEAD, package_total, now_time);
                    msg_send(protocol_buffer);
                }
                else{
                    FSIZE_t remain_char;
                    package_total=size/num+1;
                    sprintf((char *)protocol_buffer, PUSH_IMAGE_HEAD, package_total, now_time);
                    msg_send(protocol_buffer);
                }
            }
            f_close(&fir);
        }
        delay_ms(1000);

        if(wait_msg_reved()){
            p = msg_read();
            pres = strstr(p, PUSH_IMAGE_READY);
            if(*pres){
                return true;
            }
        }
    }
    return false;
}

bool upload_data(char * key, float value){
    char* p = NULL;
    char* pres = NULL;


    memset(protocol_buffer,0,MAX_TX_BUFFER);
    sprintf((char *)protocol_buffer, PACKAGE_FORMAT, key, value, now_time);

    msg_send(protocol_buffer);
    /*
    uint8_t i = 0;
    for(;i<RETRY;++i){
        clear_buffer();
        msg_send(protocol_buffer);
        if(wait_msg_reved())
        {
            p = msg_read();
            pres = strstr(p, FAIL);
            if(*pres){
            }
            else{
                return true;
            }
        }
    }


    return false;
    */
    return true;
}

bool upload_data_end(){
    char* p = NULL;
    char* pres = NULL;
    uint8_t i = 0;
    for(;i<RETRY;++i){
        clear_buffer();
        msg_send(PUSH_DATA_FINISH);
        delay_ms(1000);
        if(wait_msg_reved()){
            p = msg_read();
            pres = strstr(p, DATA_UPLOADED);
            if(*pres){
                return true;
            }
        }
    }
    return false;


}
