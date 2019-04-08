#ifndef __UARTDRIVER_H
#define	__UARTDRIVER_H

#include "System.h"

#define UART1_MaxBufLen 1024		//串口1缓存大小

#define UART1_RevTimeOut 5		//串口1接收超时时间，单位10ms

//7位数据宏定义
#define Databits_7 		0x80		//7位数据位时bit7置1
#define Paritbit_O 		0x40		//奇校验时bit6置1,此位bit7置1时才有效
#define Data7bits_O 	Databits_7+Paritbit_O		//7位数据位,奇校验
#define Data7bits_E 	Databits_7					//7位数据位,偶校验

/*******************************************************************************
**串口收发的数据位和校验位的枚举
*******************************************************************************/
typedef enum 
{
    Data8bits_Parit_N = 0,	//8位数据，无奇偶校验
    Data8bits_Parit_E = 1,	//8位数据，偶校验    
    Data8bits_Parit_O = 2,	//8位数据，奇校验
    Data9bits_Parit_N = 3,	//9位数据，无奇偶校验
	Data7bits_Parit_E = 4,	//7位数据，偶校验    
    Data7bits_Parit_O = 5,	//7位数据，奇校验
}SerialDataParitBits;

/*******************************************************************************
**串口收发的停止位的枚举
*******************************************************************************/
typedef enum 
{
    StopBits_1 = 0,
    StopBits_2,
}SerialStopBits;

/*******************************************************************************
**串口配置数据结构体
*******************************************************************************/
typedef struct 
{
    uint32  baudrate;
    SerialDataParitBits  dataparitbits;
    SerialStopBits  stopbits; 
}SerialSets;

typedef struct
{
    uint8* tbuff;                           //串口发送缓存指针
    uint8* rbuff;                           //串口接收缓存指针
    uint16 tlen;                            //串口发送缓存长度
    uint16 rlen;                            //串口接收缓存长度
    uint8  revtime;          				//接收超时 
}SerialBuffer;

SerialBuffer	gs_Uart1;    //串口数据
uint8 uart1_rbuff[UART1_MaxBufLen];//串口接收缓存
uint8 guc_uart1databit;//串口数据位，7代表7位数据，8代表8位数据

//函数声明
void UART1_Init(void);
void UART1_Config(SerialSets* ss);
void Uart1_Dy10ms(void);
uint16 UART1_Send_Data(uint8 *uart1_buffer, uint16 buflen);
uint16 UART1_Read_Data(uint8 *uart1_buffer, uint16 buflen);


#endif // __UARTDRIVER_H