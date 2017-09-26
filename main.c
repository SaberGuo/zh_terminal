/*
 * main.c
 *
 *  Created on: 2017Äê9ÔÂ12ÈÕ
 *      Author: saber
 */

#include "comm.h"
#include "relay/relay.h"
#include "clocks/clocks.h"
#include "app/gsm/gsm.h"
#include "fatfs/ff.h"
#include "adc/adc.h"
#include "app/protocol/protocol.h"
#include "app/power/power.h"

#ifdef IS_TEST
#include "test.h"
#endif


#ifdef HAS_VAISAL
#include "app/vaisal/vaisal.h"
#endif

#ifdef HAS_TH10S
#include "app/th10s/th10s.h"
#endif

#ifdef HAS_CAMERA
#include "app/camera/camera.h"
#endif



FATFS fatfs;
bool main_timer_irq_flag = false;
bool img_timer_irq_flag = false;


int main(){
    /* Halting WDT  */
    MAP_WDT_A_holdTimer();

    //clock init
    clock_init();
    //timer init
    timer_init();

    //relay init
    config_relay(RELAY_K_GSM|RELAY_K_CAMERA|RELAY_K_IO|RELAY_K_IO_POWER);
    reset_relay(RELAY_K_GSM|RELAY_K_CAMERA|RELAY_K_IO|RELAY_K_IO_POWER);
    //set_relay(RELAY_K_GSM);

    while(1){
        if(!(main_timer_irq_flag|img_timer_irq_flag)){
            MAP_PCM_gotoLPM3();
        }

        if(main_timer_irq_flag){
            main_timer_process();
        }

        if(img_timer_irq_flag){
            img_timer_process();
        }
    }

}
void img_timer_process(){

    //
    img_timer_irq_flag = false;
}

void main_timer_process(){

    bool ret = false;
    uint8_t i = 0;

    dc_5v_enable();
    dc_33v_enable();

    //construct the msg

    set_relay(RELAY_K_GSM);
    gsm_init();
    gsm_connect();

    msg_send("abc");
    //send msg
    ret=upload_data_start();
#ifdef HAS_VAISAL
    if(ret){
        for(i = 0;i<TH_DATAS_NUM;++i){
            ret&=upload_data(vaisal_datas[i].sign,vaisal_datas[i].avg_value);
        }
    }
#endif


#ifdef IS_TEST
    if(ret){
        ret=upload_image_start();
        if(ret){
            //camera_test();
            tfcard_test();
        }
    }
#endif


    ret&=upload_data_end();


    if(ret == false){//send fail

    }

    //gsm off
    gsm_close();
    reset_relay(RELAY_K_GSM);

    main_timer_irq_flag = false;


}

//main timer for process
void TA1_N_IRQHandler(void)
{
    //MAP_PCM_setPowerStateNonBlocking(PCM_AM_LDO_VCORE1);
    MAP_Timer_A_clearInterruptFlag(TIMER_A1_BASE);
    main_timer_irq_flag = true;
}

//img collection timer for process
void TA0_N_IRQHandler(void)
{
    MAP_Timer_A_clearInterruptFlag(TIMER_A0_BASE);
    img_timer_irq_flag = true;
}


