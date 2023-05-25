//单片机头文件
#include "stm32f10x.h"

//网络设备驱动
#include "esp8266.h"

//硬件驱动
#include "delay.h"
#include "usart.h"

//C库
#include <string.h>
#include <stdio.h>

#define ESP8266_WIFI_INFO		"AT+CWJAP=\"GXI\",\"66666666\"\r\n"

#define ESP8266_ONENET_INFO		"AT+CIPSTART=\"TCP\",\"183.230.40.33\",80\r\n"


char esp8266_buf[Buf2_Max];
unsigned short esp8266_cnt = 0, esp8266_cntPre = 0;
u8 First_Int = 0;
int GET_flag=0;//接收标志位



//==========================================================
//	函数名称：	ESP8266_Init
//
//	函数功能：	初始化ESP8266
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：		
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
	length=strlen(data_temp)-3;//这是由于单片机不知道\r\n作为换行符，多计算了几个字符
	sprintf(temp1,"POST /devices/652936501/datapoints?type=5 HTTP/1.1\r\napi-key:RiplobQgl=D=XOnlhXyo=H8h7HM=\r\nHost:api.heclouds.com\r\nContent-Length:%d\r\n\r\n,;TEMP3,%d",length,data);
	UART1_Send_Command(temp1);

}

void ESP8266_GetData(void)
{
	char temp2[200];
	GET_flag=1;//标志着串口中断需要处理了
	//TEMP是你的数据流名称
	sprintf(temp2,"GET /devices/652936501/datastreams/TEMP HTTP/1.1\r\napi-key:RiplobQgl=D=XOnlhXyo=H8h7HM=\r\nHost:api.heclouds.com\r\n");
	UART1_Send_Command(temp2);

}

/*
************************************************************
*	函数名称：	Usart1_Init
*
*	函数功能：	串口2初始化
*
*	入口参数：	baud：设定的波特率
*
*	返回参数：	无
*
*	说明：		TX-PA2		RX-PA3
************************************************************
*/
void Usart1Init(unsigned int baud)
{

	 GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//使能USART1，GPIOA时钟
  
	//USART1_TX   GPIOA.9
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.9
   
  //USART1_RX	  GPIOA.10初始化
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//PA10
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.10  

	//Usart1 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
  
	//USART 初始化设置

	USART_InitStructure.USART_BaudRate = baud;//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

  USART_Init(USART1, &USART_InitStructure); //初始化串口1
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启串口接受中断
  USART_Cmd(USART1, ENABLE);                    //使能串口1 

}


//==========================================================
//	函数名称：	USART2_IRQHandler
//
//	函数功能：	串口2收发中断
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：		
//==========================================================
int flag=0,GET_TEMP=0;
void USART1_IRQHandler(void)
{

	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) //接收中断
	{
		USART_ClearFlag(USART1, USART_FLAG_RXNE);
		esp8266_buf[First_Int] = USART_ReceiveData(USART1); 	 //将接收到的字符串存到缓存中
		if(First_Int >= Buf2_Max)       	  //如果缓存满,将缓存指针指向缓存的首地址
		{
			First_Int = 0;
		}
		if(GET_flag==1)//获取TEMP的数据
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
												GET_flag=0;//表示接收完毕，不再进入该if判断
											}
								else                                     flag=0;
}
		First_Int++;                 	  //缓存指针向后移动
	}

}


void UART1_SendString(char* s)
{
	while(*s)//检测字符串结束符
	{
		while(USART_GetFlagStatus(USART1, USART_FLAG_TC)==RESET); 
		USART_SendData(USART1 ,*s++);//发送当前字符
	}
}
/********************************************************************************
	功能：	串口发送指令
	参数：b  								要发送的字符串
				a  								期待返回的字符串
				wait_time 				发送次数
				interval_time			等待相应时间
	返回：	1 成功  0 失败
	备注：	无
*********************************************************************************/
u8 UART1_Send_AT_Command_End(char *b,char *a,u8 wait_time,u32 interval_time)         
{
	u8 i;
	i = 0;
	while(i < wait_time)    //如果没有找到 就继续再发一次指令 再进行查找目标字符串                
	{
		UART1_Send_Command(b);//串口发送 b 字符串 这里发送\r\n 
		delay_ms(interval_time); //等待一定时间 传50的话就是 50*20ms = 1秒
		if(Find(a))            //查找需要应答的字符串 a
		{
			return 0;
		}
		i++;
	}
	
	return 1;
}
void UART1_Send_Command(char* s)
{
	CLR_Buf(); //清空接收数据的buffer
	UART1_SendString(s); //发出字符串
	UART1_SendString("\r\n"); //再自动发送 \r\n两个字符
}

void CLR_Buf(void)
{
	u16 k;
	for(k=0;k<Buf2_Max;k++)      //将缓存内容清零
	{
		esp8266_buf[k] = 0x00;
	}
    First_Int = 0;              //接收字符串的起始存储位置
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
