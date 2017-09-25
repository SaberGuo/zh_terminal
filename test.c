/*
 * test.c
 *
 *  Created on: 2017Äê9ÔÂ23ÈÕ
 *      Author: saber
 */

#include "test.h"
#include "app/power/power.h"
#include "relay/relay.h"
#include "fatfs/ff.h"

#ifdef HAS_CAMERA
#include "app/camera/camera.h"
#endif

uint8_t rec_buffer[1024];
FATFS fatfs_t;

void camera_test(){
    dc_5v_enable();
    dc_33v_enable();
    tfcard_enable();

    set_relay(RELAY_K_CAMERA);

    //init for tfcard
    f_mount(&fatfs_t,"",0);

    //camera init
    camera_init();
    int32_t ret =0;
    while(1){
        ret = tcpc_run(rec_buffer);
    }
}

void tfcard_test(){
    dc_5v_enable();
    dc_33v_enable();
    tfcard_enable();
    FIL image_fil;
    tfcard_driver_config();
    BYTE work[1024];
    //FRESULT res=f_mkfs("",FM_ANY, 0, work, sizeof(work));
    //printf("f mount res is %d\n",res);
    FRESULT res = f_mount(&fatfs_t,"",1);
    printf("f mkfs res is %d\n",res);
    res = f_open(&image_fil,"image.jpg", FA_WRITE|FA_CREATE_ALWAYS);
    printf("f open res is %d\n",res);
    if(res == FR_OK)
    {
        f_close(&image_fil);
    }

    //set_relay(RELAY_K_CAMERA);
}
