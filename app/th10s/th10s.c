/*
 * th10s.c
 *
 *  Created on: 2017年9月13日
 *      Author: saber
 */

#include "th10s.h"
#include "uart/uart.h"
#include "base/clocks/clocks.h"

#ifdef ewarm
#pragma data_alignment=1024
#else
#pragma DATA_ALIGN(controlTable, 1024)
#endif
uint8_t controlTable[1024];

#define MAX_BUFFER 100
#define TH_DATAS_NUM 3

#define SERIAL_READ  1
#define SERIAL_WRITE 2


uint8_t th_res_buffer[MAX_BUFFER];
uint8_t th_pbuffer=0;

const char modbus_command[]={0x01,0x03,0x00,0x00,0x00,0x02,0xC4,0x0B};

float temperature_value = 0;
float humility_value =0;

const eUSCI_UART_Config th10s_uart_config=
{
     EUSCI_A_UART_CLOCKSOURCE_SMCLK,          // 选择SMCLK时钟为源，6MHz
     39,                                      // bps=9600;
     1,                                       //
     0,                                      //
     EUSCI_A_UART_NO_PARITY,                  // No Parity
     EUSCI_A_UART_LSB_FIRST,                  // LSB First
     EUSCI_A_UART_ONE_STOP_BIT,               // One stop bit
     EUSCI_A_UART_MODE,                       // UART mode
     EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION
};


void th_dma_init(){
    MAP_DMA_enableModule();
    MAP_DMA_setControlBase(controlTable);
    MAP_DMA_assignChannel(DMA_CH1_EUSCIA0RX);
    MAP_DMA_disableChannelAttribute(DMA_CH1_EUSCIA0RX,UDMA_ATTR_ALL);
    MAP_DMA_setChannelControl(UDMA_PRI_SELECT | DMA_CH1_EUSCIA0RX,
          UDMA_SIZE_8 | UDMA_SRC_INC_NONE | UDMA_DST_INC_8 | UDMA_ARB_1);

    MAP_DMA_setChannelTransfer(UDMA_PRI_SELECT | DMA_CH1_EUSCIA0RX,
          UDMA_MODE_BASIC,
          (void*)UART_getReceiveBufferAddressForDMA(EUSCI_A0_BASE), th_res_buffer,
          MAX_BUFFER);
    MAP_DMA_enableChannel(1);

}


void th10s_init(){
    uart_config _th10s_uart_conf = {.gpio_group = GPIO_PORT_P1,
        .gpio_pins= GPIO_PIN2 | GPIO_PIN3,
        .port = EUSCI_A0_BASE,
        .pconf = &th10s_uart_config};

    uart_init(&_th10s_uart_conf);

    GPIO_setAsOutputPin(GPIO_PORT_P1,GPIO_PIN1);
    GPIO_setOutputHighOnPin(GPIO_PORT_P1,GPIO_PIN1);

    th_dma_init();
}

void set_serial_flag(uint8_t flag){
    if(flag == SERIAL_READ){
        GPIO_setOutputLowOnPin(GPIO_PORT_P1,GPIO_PIN1);
    }

    if(flag == SERIAL_WRITE){
        GPIO_setOutputHighOnPin(GPIO_PORT_P1,GPIO_PIN1);
    }
}

void get_modbus_value(char * command, uint8_t size){
    uint8_t i = 0;
    memset(th_res_buffer, 0, MAX_BUFFER);

    MAP_DMA_setChannelTransfer(UDMA_PRI_SELECT | DMA_CH1_EUSCIA0RX,
                        UDMA_MODE_BASIC,
                        (void*)UART_getReceiveBufferAddressForDMA(EUSCI_A0_BASE), th_res_buffer,
                        MAX_BUFFER);
    MAP_DMA_enableChannel(1);

    set_serial_flag(SERIAL_WRITE);
    for(;i<size;++i){
        UART_transmitData(EUSCI_A0_BASE,command[i]);
    }
    delay_ms(1);
    set_serial_flag(SERIAL_READ);
    delay_ms(30);
    //res = ((uint16_t)th_res_buffer[3])<<8|th_res_buffer[4];
}

void th10s_process(){
    uint16_t res = 0;
    uint16_t tem = 0;

    get_modbus_value(modbus_command, sizeof(modbus_command));
    res=((uint16_t)th_res_buffer[3])<<8 | th_res_buffer[4];
    if(res<1000){//the temperature > 0
        temperature_value = res/10.0;
    }
    else{//the temperature < 0
        tem=0xFFFF-res;
        temperature_value = -tem/10.0;
    }

    res=((uint16_t)th_res_buffer[5])<<8 | th_res_buffer[6];
    humility_value = res/10.0;

}

float get_temperature(){
    return temperature_value;
}

float get_humility(){
    return humility_value;
}
