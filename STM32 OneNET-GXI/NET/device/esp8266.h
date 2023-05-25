#ifndef _ESP8266_H_
#define _ESP8266_H_

#include "sys.h"

#define Buf2_Max 	  300 //串口2缓存长度
#define STABLE_TIMES  20 //等待系统上电后的稳定

#define REV_OK		0	//接收完成标志
#define REV_WAIT	1	//接收未完成标志


extern char esp8266_buf[Buf2_Max]	,temp1[200];
extern int GET_flag,GET_TEMP;
void ESP8266_Init(void);
void Usart1Init(unsigned int baud);
void ESP8266_SendData(int data);
void UART1_SendString(char* s);
void CLR_Buf(void);
void ESP8266_GetData(void);
void UART1_Send_Command(char* s);
u8 UART1_Send_AT_Command_End(char *b,char *a,u8 wait_time,u32 interval_time);
u8 Find(char *a);
#endif
