/*
 * main.c
 *
 *  Created on: 2017��9��12��
 *      Author: saber
 */

#include "comm.h"
#include "relay/relay.h"
#include "clocks/clocks.h"
#include "app/gsm/gsm.h"
#include "fatfs/ff.h"
#include "adc/adc.h"
#include "app/protocol/protocol.h"


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

int main(){
    /* Halting WDT  */
    MAP_WDT_A_holdTimer();
    //MAP_Interrupt_enableSleepOnIsrExit();

    //relay init
    config_relay(RELAY_K_GSM|RELAY_K_CAMERA|RELAY_K_IO|RELAY_K_IO_POWER);
    reset_relay(RELAY_K_GSM|RELAY_K_CAMERA|RELAY_K_IO|RELAY_K_IO_POWER);

    //clock init
    clock_init();

    //timer init
    timer_init();

    //tfcard init
    f_mount(&fatfs, "",0);


#ifdef HAS_VAISAL
    //init for sensor vaisal
    vaisal_init();
#endif


#ifdef HAS_TH10S
    //init for sensor th10s
    th10s_init();
#endif


#ifdef HAS_CAMERA
    camera_init();
#endif
    //adc14 init
    ADC14_init();

    while(1){
        if(!main_timer_irq_flag){
            MAP_PCM_gotoLPM3();
        }

        if(main_timer_irq_flag){
            main_timer_process();
        }
    }
}

void main_timer_process(){

    bool ret = false;
    uint8_t i = 0;

    /*
    //open 485 serial port and IO 12V
    set_relay(RELAY_K_IO|RELAY_K_IO_POWER);


#ifdef HAS_VAISAL
    delay_ms(1000); //wait for 10s with
    start_vaisal_rev();
    delay_ms(1000);
    stop_vaisal_rev();

#endif


    reset_relay(RELAY_K_IO|RELAY_K_IO_POWER);
    */

    //construct the msg

    set_relay(RELAY_K_GSM);
    gsm_init();
    gsm_connect();

    //send msg
    ret=upload_data_start();

#ifdef HAS_VAISAL
    if(ret){
        for(i = 0;i<TH_DATAS_NUM;++i){
            ret&=upload_data(vaisal_datas[i].sign,vaisal_datas[i].avg_value);
        }
    }
#endif
    ret&=upload_data_end();

    if(ret == false){//send fail

    }

    //gsm off
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


