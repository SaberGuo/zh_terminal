/*
 * th10s.h
 *
 *  Created on: 2017��9��13��
 *      Author: saber
 */

#ifndef APP_TH10S_TH10S_H_
#define APP_TH10S_TH10S_H_

extern void th10s_init();
extern void th10s_close();
extern void th10s_process(void);

extern float get_wind_speed();
extern float get_wind_direction();

#endif /* APP_TH10S_TH10S_H_ */
