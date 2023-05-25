//C库
#include <string.h>

//单片机头文件
#include "sys.h"


//网络设备
#include "esp8266.h"

//硬件驱动
#include "delay.h"
#include "usart.h"
#include "led.h"

float temper;


//硬件初始化
void Hardware_Init(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	
	delay_init();			//延时函数初始化

	Usart1Init(115200); 	//串口1，驱动ESP8266用
	ESP8266_Init();
	ESP8266_SendData(16);
	ESP8266_GetData();
}


int main(void)
{
	Hardware_Init();

	while(1)
	{	
		delay_ms(10);
	
	}
}



















