#ifndef __MAIN_H
#define __MAIN_H


#include "stm32f10x.h"
#include "stm32f107.h"


#define LED_ON()     GPIO_WriteBit(GPIOC,GPIO_Pin_13,Bit_RESET)
#define LED_OFF()    GPIO_WriteBit(GPIOC,GPIO_Pin_13,Bit_SET)


extern uint8_t  flag_LedFlicker;

void Time_Update(void);
void Delay(uint32_t nCount);


#endif /* __MAIN_H */


