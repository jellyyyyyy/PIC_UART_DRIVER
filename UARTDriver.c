#include "UARTDriver.h"
#include "config.h"

uint16 Rx1_Out_Pos;				//从buffer获取数据，用该标志索引
volatile uint16 Rx1_In_Pos;		//当中断有新的数据，用该标志记录
volatile uint16 Rx1_Num_Bytes;	//当前接收buffer有多少个字节

uint16 Tx1_In_Pos;
volatile uint16 Tx1_Out_Pos;
uint8 UART1_Sendtimeout;

/*========================================================
* @function_name: _U1RXInterrupt
* @function_file: uart1.c
* @描述: UART1 RX Interrupt function
* 
* @param: 无 
* 
* @return: 无
* @作者: zhangguodong
* @备注: 
*----------------------------------------------------------
* @修改人: 
* @修改内容:  
============================================================*/
void __attribute__((interrupt, no_auto_psv)) _U1RXInterrupt(void)
{
    if(IFS0bits.U1RXIF==1)
    {
	    IFS0bits.U1RXIF = 0;	// clear UART1 RX interrupt flag  
	    Uart1_Start = 1;		//uart1接收标志
	    
	    while((U1STAbits.URXDA == TRUE) && (Rx1_Num_Bytes != UART1_MaxBufLen))
	    {
	    	++Rx1_Num_Bytes;
	    	gs_Uart1.rbuff[Rx1_In_Pos] = U1RXREG;
	    	Rx1_In_Pos = (Rx1_In_Pos + 1) % UART1_MaxBufLen;
	    }
	    
	    gs_Uart1.revtime =UART1_RevTimeOut;
	}    
}

/*========================================================
* @function_name: pushTx
* @function_file: uart1.c
* @描述: UART1 TX Interrupt function
* 
* @param: 无 
* 
* @return: 无
* @作者: zhangguodong
* @备注: 
*----------------------------------------------------------
* @修改人: 
* @修改内容:  
============================================================*/
void pushTx1()
{ 	
	while((U1STAbits.UTXBF == FALSE) && (Tx1_In_Pos != Tx1_Out_Pos))
	{
		U1TXREG = gs_Uart1.tbuff[Tx1_Out_Pos];
		Tx1_Out_Pos = (Tx1_Out_Pos + 1) % UART1_MaxBufLen;
	}
	
	IFS0bits.U1TXIF = 0;	// clear UART1 TX interrupt flag 
}

/*========================================================
* @function_name: UART1_StartSend
* @function_file: uart1.c
* @描述: Uart1启动发送
* 
* @param: 无 
* 
* @return: 无
* @作者: zhangguodong
* @备注: 
*----------------------------------------------------------
* @修改人: 
* @修改内容:  
============================================================*/
void UART1_StartSend(void)
{
	uint16 next = (Tx1_Out_Pos + 1) % UART1_MaxBufLen;
    
    U1TXREG = gs_Uart1.tbuff[Tx1_Out_Pos];
    Tx1_Out_Pos = next;
}

/*========================================================
* @function_name: _U1TXInterrupt
* @function_file: uart1.c
* @描述: UART1 TX Interrupt function
* 
* @param: 无 
* 
* @return: 无
* @作者: zhangguodong
* @备注: 
*----------------------------------------------------------
* @修改人: 
* @修改内容:  
============================================================*/
void __attribute__((interrupt, no_auto_psv)) _U1TXInterrupt(void)
{    
	if(IFS0bits.U1TXIF==1)
	{
		pushTx1();
		UART1_Sendtimeout = 2;
	}		
}

/*========================================================
* @function_name: UART1_Config
* @function_file: uart1.c
* @描述: UART1配置 45619200/2/8 =2851200 Hz
* 
* @param: SerialSets* ss 
* 
* @return: 无
* @作者:
* @备注: 
*----------------------------------------------------------
* @修改人: 
* @修改内容:  
============================================================*/
void UART1_Config(SerialSets* ss)
{
	// UART1 setup	
	TRISBbits.TRISB10 = 0;   //TX: set RB10 port output
	TRISBbits.TRISB15 = 1;   //RX: set RB15 port input
	RPOR4bits.RP42R = 1;	 //RP42连到TXD1
	RPINR18bits.U1RXR = 47;	 //RPI47连到RXD1
	
	U1MODEbits.UARTEN =1;//打开UART

	if(ss->dataparitbits<=3)
	{	
		U1MODEbits.PDSEL =ss->dataparitbits;//数据位和校验位
		guc_uart1databit =0;
	}else
	{	
		U1MODEbits.PDSEL =0;//7位数据位，此时最高位用作校验位			
		if(ss->dataparitbits==Data7bits_Parit_O)
		{
			guc_uart1databit =Data7bits_O;//奇校验
		}else
		{
			guc_uart1databit =Data7bits_E;//偶校验
		}
	}
	U1MODEbits.STSEL =ss->stopbits;//停止位	
	U1BRG =_CLOCK_FP/16/ss->baudrate-1;//波特率设置	
		
	U1MODEbits.BRGH =0;//1=高速，波特率为baudclk/4,0=低速，波特率为baudclk/16
	U1MODEbits.URXINV =0;//空闲时RX输出高		
	U1MODEbits.ABAUD =0;//
	U1MODEbits.UEN =0;
	U1MODEbits.RTSMD =0;//

	U1STAbits.UTXINV =0;//空闲时TX输出高	
	U1STAbits.UTXISEL1 =1;//发送缓冲区空设置中断
	U1STAbits.UTXISEL0 =0;//发送缓冲区空设置中断
	U1STAbits.URXISEL =0;//接收缓冲区有数据时触发中断
	
	IFS0bits.U1TXIF = 0;	// clear UART1 TX interrupt flag
	IFS0bits.U1RXIF = 0;	// clear UART1 RX interrupt flag
	IEC0bits.U1TXIE = 1;	//允许发送中断
	IEC0bits.U1RXIE = 1;	//允许接收中断
	IPC3bits.U1TXIP = 3;	//发送中断优先级
	IPC2bits.U1RXIP = 3;	//接收中断优先级
	
	U1STAbits.UTXEN =1;//发送使能
}
/*========================================================
* @function_name: UART1_Init
* @function_file: uart1.c
* @描述: UART1初始化 45619200/2/8 =2851200 Hz
* 
* @param: 无 
* 
* @return: 无
* @作者: 
* @备注: 
*----------------------------------------------------------
* @修改人: zhangguodong
* @修改内容: 串口参数可配
============================================================*/
void UART1_Init(void)
{
    SerialSets ss;
		
   	ss.baudrate = 9600;//波特率设置
    ss.dataparitbits = Data8bits_Parit_N;//数据位和校验位设置
    ss.stopbits = StopBits_1;//停止位设置
	UART1_Config((SerialSets *)&ss);//串口初始化配置
	
	gs_Uart1.rbuff =(uint8 *)uart1_rbuff;
	gs_Uart1.tbuff =(uint8 *)uart1_tbuff;
	gs_Uart1.rlen =UART1_MaxBufLen;
	gs_Uart1.tlen =UART1_MaxBufLen;
	Rx1_Num_Bytes = 0;
	Rx1_In_Pos = Rx1_Out_Pos = 0;
	Tx1_In_Pos = Tx1_Out_Pos = 0;
	Mem16Set(gs_Uart1.rbuff,0x00,gs_Uart1.rlen);//清接收缓存
	gs_Uart1.revtime =0;
}

/*========================================================
* @function_name: Uart1_Dy10ms
* @function_file: uart1.c
* @描述: UART1超时接收
* 
* @param: 无 
* 
* @return: 无
* @作者:
* @备注: 
*----------------------------------------------------------
* @修改人: 
* @修改内容:  
============================================================*/
void Uart1_Dy10ms(void)
{
	if(gs_Uart1.revtime >0)
	{
		gs_Uart1.revtime--;
		if(gs_Uart1.revtime ==0)
		{
			Uart1_Timeout = 1;
		}	
	}
}

/*========================================================
* @function_name: GetChar
* @function_file: uart1.c
* @描述: UART1 按字节接收数据
* 
* @param: 
* 
* @return: 无
* @作者: zhangguodong
* @备注: 
*----------------------------------------------------------
* @修改人: 
* @修改内容:  
============================================================*/
bool GetChar(uint8 *byte)
{
    if(Rx1_Num_Bytes != 0)
    {
    	--Rx1_Num_Bytes;
    	*byte = gs_Uart1.rbuff[Rx1_Out_Pos];
    	Rx1_Out_Pos = (Rx1_Out_Pos + 1) % UART1_MaxBufLen;
    	return TRUE;
    }
	
	return FALSE;
}

/*========================================================
* @function_name: PutChar
* @function_file: uart1.c
* @描述: UART1 按字节发送数据
* 
* @param: 发送字节
* 
* @return: 无
* @作者: zhangguodong
* @备注: 
*----------------------------------------------------------
* @修改人: 
* @修改内容:  
============================================================*/
bool PutChar(uint8 txChar)
{
    uint16 next = (Tx1_In_Pos + 1) % UART1_MaxBufLen;
    
    if(next == Tx1_Out_Pos)
    	return FALSE;
    gs_Uart1.tbuff[Tx1_In_Pos] = txChar;
    Tx1_In_Pos = next;
    
    pushTx1();
    return TRUE;
}

/*========================================================
* @function_name: UART1_Sead_Data
* @function_file: uart1.c
* @描述: UART1 发送数据
* 
* @param: 无 
* 
* @return: 无
* @作者: zhangguodong
* @备注: 
*----------------------------------------------------------
* @修改人: 
* @修改内容:  
============================================================*/
uint16 UART1_Send_Data(uint8 *uart1_buffer, uint16 buflen)
{
	uint16 i,size;
	
	size = (Tx1_In_Pos - Tx1_Out_Pos + UART1_MaxBufLen) % UART1_MaxBufLen;
	size = MIN(UART1_MaxBufLen - size, buflen);
	
	for(i=0;i<size;i++)
	{
		gs_Uart1.tbuff[Tx1_In_Pos] = uart1_buffer[i];
		Tx1_In_Pos = (Tx1_In_Pos + 1) % UART1_MaxBufLen;
	}
	
	UART1_StartSend();
	
	while(UART1_Sendtimeout > 0)
	{
		if(guc_TimeEvent&flgEtTim_10ms)                  
		{
			UART1_Sendtimeout--;
        	guc_TimeEvent &=~flgEtTim_10ms; 
		}		
	}	
	
	return size;
}


/*========================================================
* @function_name: UART1_Read_Data
* @function_file: uart1.c
* @描述: UART1 读数据
* 
* @param: 无 
* 
* @return: 无
* @作者: zhangguodong
* @备注: 
*----------------------------------------------------------
* @修改人: 
* @修改内容:  
============================================================*/
uint16 UART1_Read_Data(uint8 *uart1_buffer, uint16 buflen)
{
	uint16 i,size;
	size = Rx1_Num_Bytes;
	
	for(i=0;i < size;i++)
	{
		--Rx1_Num_Bytes;
		uart1_buffer[i] =gs_Uart1.rbuff[Rx1_Out_Pos];
		Rx1_Out_Pos = (Rx1_Out_Pos + 1) % UART1_MaxBufLen;
	}
	
	return size;
}