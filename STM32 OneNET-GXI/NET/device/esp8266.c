//��Ƭ��ͷ�ļ�
#include "stm32f10x.h"

//�����豸����
#include "esp8266.h"

//Ӳ������
#include "delay.h"
#include "usart.h"

//C��
#include <string.h>
#include <stdio.h>

#define ESP8266_WIFI_INFO		"AT+CWJAP=\"GXI\",\"66666666\"\r\n"

#define ESP8266_ONENET_INFO		"AT+CIPSTART=\"TCP\",\"183.230.40.33\",80\r\n"


char esp8266_buf[Buf2_Max];
unsigned short esp8266_cnt = 0, esp8266_cntPre = 0;
u8 First_Int = 0;
int GET_flag=0;//���ձ�־λ



//==========================================================
//	�������ƣ�	ESP8266_Init
//
//	�������ܣ�	��ʼ��ESP8266
//
//	��ڲ�����	��
//
//	���ز�����	��
//
//	˵����		
//==========================================================
void ESP8266_Init(void)
{
	
	CLR_Buf();

	while(UART1_Send_AT_Command_End("AT\r\n\r", "OK", 1,200))
		delay_ms(500);
	

	while(UART1_Send_AT_Command_End("AT+CWMODE=1\r\n", "OK",1,200))
		delay_ms(500);
	
	while(UART1_Send_AT_Command_End(ESP8266_WIFI_INFO, "GOT IP", 1,200))
		delay_ms(500);
	
	while(UART1_Send_AT_Command_End(ESP8266_ONENET_INFO, "CONNECT",2,200))
		delay_ms(500);
	
	while(UART1_Send_AT_Command_End("AT+CIPMODE=1\r\n", "OK",1,200))
		delay_ms(500);
	
	while(UART1_Send_AT_Command_End("AT+CIPSEND\r\n", "OK",1,200))
		delay_ms(500);

}

char temp1[200],data_temp[50];
void ESP8266_SendData(int data)
{
	int length=0;
	sprintf(data_temp,"\r\n\r\n,;TEMP3,%d",data);
	length=strlen(data_temp)-3;//�������ڵ�Ƭ����֪��\r\n��Ϊ���з���������˼����ַ�
	sprintf(temp1,"POST /devices/652936501/datapoints?type=5 HTTP/1.1\r\napi-key:RiplobQgl=D=XOnlhXyo=H8h7HM=\r\nHost:api.heclouds.com\r\nContent-Length:%d\r\n\r\n,;TEMP3,%d",length,data);
	UART1_Send_Command(temp1);

}

void ESP8266_GetData(void)
{
	char temp2[200];
	GET_flag=1;//��־�Ŵ����ж���Ҫ������
	//TEMP���������������
	sprintf(temp2,"GET /devices/652936501/datastreams/TEMP HTTP/1.1\r\napi-key:RiplobQgl=D=XOnlhXyo=H8h7HM=\r\nHost:api.heclouds.com\r\n");
	UART1_Send_Command(temp2);

}

/*
************************************************************
*	�������ƣ�	Usart1_Init
*
*	�������ܣ�	����2��ʼ��
*
*	��ڲ�����	baud���趨�Ĳ�����
*
*	���ز�����	��
*
*	˵����		TX-PA2		RX-PA3
************************************************************
*/
void Usart1Init(unsigned int baud)
{

	 GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//ʹ��USART1��GPIOAʱ��
  
	//USART1_TX   GPIOA.9
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
  GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.9
   
  //USART1_RX	  GPIOA.10��ʼ��
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//PA10
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
  GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.10  

	//Usart1 NVIC ����
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
  
	//USART ��ʼ������

	USART_InitStructure.USART_BaudRate = baud;//���ڲ�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

  USART_Init(USART1, &USART_InitStructure); //��ʼ������1
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//�������ڽ����ж�
  USART_Cmd(USART1, ENABLE);                    //ʹ�ܴ���1 

}


//==========================================================
//	�������ƣ�	USART2_IRQHandler
//
//	�������ܣ�	����2�շ��ж�
//
//	��ڲ�����	��
//
//	���ز�����	��
//
//	˵����		
//==========================================================
int flag=0,GET_TEMP=0;
void USART1_IRQHandler(void)
{

	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) //�����ж�
	{
		USART_ClearFlag(USART1, USART_FLAG_RXNE);
		esp8266_buf[First_Int] = USART_ReceiveData(USART1); 	 //�����յ����ַ����浽������
		if(First_Int >= Buf2_Max)       	  //���������,������ָ��ָ�򻺴���׵�ַ
		{
			First_Int = 0;
		}
		if(GET_flag==1)//��ȡTEMP������
		{
			if(esp8266_buf[First_Int]=='v') 																		    flag=1;
				else if(flag==1 && esp8266_buf[First_Int]=='a')							 						flag=2;
					else if(flag==2 && esp8266_buf[First_Int]=='l')												 flag=3;
						else if(flag==3 && esp8266_buf[First_Int]=='u')					   						flag=4;
							else if(flag==4 && esp8266_buf[First_Int]=='e')					  					 flag=5;
								else if(flag==5 && esp8266_buf[First_Int]=='"')				   					  flag=6;
									else if(flag==6 && esp8266_buf[First_Int]==':')				   					 flag=7;
								  else if(flag==7)			
											{
												if(esp8266_buf[First_Int]=='1')  GET_TEMP=1;
												else   													 GET_TEMP=0;
												GET_flag=0;//��ʾ������ϣ����ٽ����if�ж�
											}
								else                                     flag=0;
}
		First_Int++;                 	  //����ָ������ƶ�
	}

}


void UART1_SendString(char* s)
{
	while(*s)//����ַ���������
	{
		while(USART_GetFlagStatus(USART1, USART_FLAG_TC)==RESET); 
		USART_SendData(USART1 ,*s++);//���͵�ǰ�ַ�
	}
}
/********************************************************************************
	���ܣ�	���ڷ���ָ��
	������b  								Ҫ���͵��ַ���
				a  								�ڴ����ص��ַ���
				wait_time 				���ʹ���
				interval_time			�ȴ���Ӧʱ��
	���أ�	1 �ɹ�  0 ʧ��
	��ע��	��
*********************************************************************************/
u8 UART1_Send_AT_Command_End(char *b,char *a,u8 wait_time,u32 interval_time)         
{
	u8 i;
	i = 0;
	while(i < wait_time)    //���û���ҵ� �ͼ����ٷ�һ��ָ�� �ٽ��в���Ŀ���ַ���                
	{
		UART1_Send_Command(b);//���ڷ��� b �ַ��� ���﷢��\r\n 
		delay_ms(interval_time); //�ȴ�һ��ʱ�� ��50�Ļ����� 50*20ms = 1��
		if(Find(a))            //������ҪӦ����ַ��� a
		{
			return 0;
		}
		i++;
	}
	
	return 1;
}
void UART1_Send_Command(char* s)
{
	CLR_Buf(); //��ս������ݵ�buffer
	UART1_SendString(s); //�����ַ���
	UART1_SendString("\r\n"); //���Զ����� \r\n�����ַ�
}

void CLR_Buf(void)
{
	u16 k;
	for(k=0;k<Buf2_Max;k++)      //��������������
	{
		esp8266_buf[k] = 0x00;
	}
    First_Int = 0;              //�����ַ�������ʼ�洢λ��
}
u8 Find(char *a)
{ 
	if(strstr(esp8266_buf, a)!=NULL)
	{
		return 1;
	}	
	else
	{
		return 0;
		
	}
		
}
