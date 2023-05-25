//C��
#include <string.h>

//��Ƭ��ͷ�ļ�
#include "sys.h"


//�����豸
#include "esp8266.h"

//Ӳ������
#include "delay.h"
#include "usart.h"
#include "led.h"

float temper;


//Ӳ����ʼ��
void Hardware_Init(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	
	delay_init();			//��ʱ������ʼ��

	Usart1Init(115200); 	//����1������ESP8266��
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



















