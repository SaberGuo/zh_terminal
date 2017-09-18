/*
 * adc.h
 *
 *  Created on: 2017��9��13��
 *      Author: saber
 */

#ifndef BASE_ADC_ADC_H_
#define BASE_ADC_ADC_H_

extern void adc14_init(void);
extern void adc14_start_sample(void);
extern void adc14_stop_sample(void);
extern float get_adc14_value(uint8_t adc_no);

#endif /* BASE_ADC_ADC_H_ */
