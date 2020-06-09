#include "stm32_eth.h"
#include "netconf.h"
#include "main.h"
#include "helloworld.h"
#include "httpd.h"
#include "tftpserver.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define SYSTEMTICK_PERIOD_MS  10

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
__IO uint32_t LocalTime = 0; /* this variable is used to create a time reference incremented by 10ms */
uint32_t timingdelay;
uint8_t  flag_LedFlicker = RESET;
uint32_t LedCounter = 0;
/* Private function prototypes -----------------------------------------------*/
void Led_Periodic_Handle(void);
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
int main(void)
{
  
  /* Setup STM32 system (clocks, Ethernet, GPIO, NVIC) and STM3210C-EVAL resources */
  System_Setup();

  /* Initilaize the LwIP satck */
  LwIP_Init();

  /* Initilaize the webserver module */
  httpd_init();

  /* Infinite loop */
  while (1)
  {
	/* LwIP periodic services are done here */
	LwIP_Periodic_Handle(LocalTime);
	Led_Periodic_Handle();
  }
}

void Led_Periodic_Handle(void)
{
	if(flag_LedFlicker){
		if(LedCounter < 500){
			LED_ON();
		}
		if(LedCounter < 1000){
			LED_OFF();
		}else{
			LedCounter = 0;
		}
	}
}


/**
  * @brief  Inserts a delay time.
  * @param  nCount: number of 10ms periods to wait for.
  * @retval None
  */
void Delay(uint32_t nCount)
{
  /* Capture the current local time */
  timingdelay = LocalTime + nCount;  

  /* wait until the desired delay finish */  
  while(timingdelay > LocalTime)
  {     
  }
}



/**
  * @brief  Updates the system local time
  * @param  None
  * @retval None
  */
void Time_Update(void)
{
  LocalTime += SYSTEMTICK_PERIOD_MS;
  LedCounter += SYSTEMTICK_PERIOD_MS;
}
