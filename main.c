/*
* File:   main.c
* Author: lee
*
* Created on 2017�� 6�� 22�� (��), ���� 8:51
*/

#include <p33FJ128GP310A.h>
#include "lib/w5300.h"
#include <libpic30.h>
#include <string.h>
#include <math.h>
#include <timer.h>
#include <uart.h>
#include <stdio.h>
#include <stdlib.h>
#include "lib\init_drv.h"
#include "lib\data_define.h"

//Configuration Bits ����
//_FOSC(ECIO_PLL4);//_FOSC(ECIO_PLL8);
//_FWDT(WDT_OFF);
//_FBORPOR(MCLR_EN && PWRT_OFF);
void OperationSwitch();
void ButtonPrintValue(void);
void InitValue(void);
void MenuDisplay(void);
void InnerVoltTest(void);
void TestResultCheck(unsigned int Label);
void RelayControl() ;
void InitRelay(void);
void ResidualVoltTest(void);
void ResidualVoltTestResult(void);
void ResidualVoltTestResultDebug(void);
void ShortTest(void);
void ShortTestResult(void);
void ShortTestResultDebug(void);
void InsulationTest(void);
void InsulationTestResult(void);
void InsulationTestResultDebug(void);
unsigned int ReadADC7980(void);
void ADCValueCheck(int ChannelCount);
void ReadADCValue(unsigned int AverageCount);
void AlramPrint();
void UdpSetting();
void SendData(unsigned int Name);
unsigned char     socket(  unsigned char  protocol,  unsigned short port,  unsigned short mode);
void     close();
unsigned long   sendto( unsigned char  * buf, unsigned long len,  unsigned char  * addr,  unsigned short port);
unsigned long   recvfrom( unsigned char  * buf, unsigned long len,  unsigned char  * addr,  unsigned short  *port);
void     loopback_udp(  unsigned short port,   unsigned short mode);
// ���ͷ�Ʈ ���� ��************************************************
void __attribute__((interrupt, auto_psv)) _U1RXInterrupt(void);
void __attribute__((interrupt, auto_psv)) _T1Interrupt(void);
void __attribute__((interrupt, auto_psv)) _T2Interrupt(void);

//****************************************************************
// <editor-fold defaultstate="collapsed" desc="����������">
enum { MODE_SEL, VOLT_ERR}; // 2 ��� ȭ��, ���� ȭ��
enum { CHK_OK, CHK_NO, CHK_RESULT, CHK_HINT, CHK_HINT_1, CHK_LAN, CHK_VOLT, CHK_BIT}; // 7
enum { NONE, SELF, LINE, FIRE1, FIRE2 };
enum { SELF_TEST, SELF_RES_TEST, SELF_SHO_TEST, SELF_INS_TEST}; // 5
enum { SELF_RES_ING, SELF_SHO_ING, SELF_INS_ING, SELF_RES_RESULT, SELF_SHO_RESULT, SELF_INS_RESULT}; // 6
enum { LINE_RES_ING, LINE_SHO_ING, LINE_INS_ING, LINE_RES_RESULT, LINE_SHO_RESULT, LINE_INS_RESULT}; // 6
enum { LINE_TEST, LINE_RES_TEST, LINE_SHO_TEST, LINE_INS_TEST}; // 5
enum { FIRE_TEST, FIRE_TEST_EACH_1, FIRE_TEST_TOTAL_1, FIRE_TEST_EACH_2, FIRE_TEST_TOTAL_2 }; // 6
enum { MSL_RESULT, EXT_RESULT, EB_RESULT, BAT_RESULT, ABAT_RESULT, BDU_RESULT, ARMING_RESULT, INTARM_RESULT, INT_RESULT}; //9
enum { RESULT,DC_VREF, DC5V, DC12V_PLUS, DC12V_MINUS};//enum { RESULT,DC_VREF, DC3V, DC5V, DC12V_PLUS, DC12V_MINUS};
enum { A0, A1, A2, CLEAR}; // ��巹�� ����
enum { SEL_ADC, SEL_LED, SEL_DATA, SEL_ADDRESS, SEL_RELAY01, SEL_LEDADC};   // CS ����
enum { RESIDUAL, SHORT, INSULATION};    // �ܷ�, ����/�ܼ�, ����
enum { RESIDUAL_RELAY, INSUL_RELAY, OK_LED, INSUL_LED, INTEGRATION_LED, SP_LED, SHORT_LED, RESIDUAL_VOLT_LED};
// </editor-fold>
// ���� ���� ����************************
unsigned char g_RxBuffer[100];
unsigned char g_RxData;
unsigned char g_Buffer;
unsigned char g_UartFunCtionButton;
unsigned int g_AdcBuffer[5];
unsigned int g_AdcCount=0;
unsigned int g_RelayLedStatus=0;
unsigned long int g_AdcSample[5];
unsigned int g_RelayChannel = 0;
unsigned int g_InputValue=0 ; // �Է°� ���� ����
unsigned int g_TimerCount=0;
unsigned int g_OkButtonTimerCount =0;
unsigned int g_OkButtonTimerCountPre =0;
unsigned int g_TimerCounter =0;
unsigned short g_DataBuffer[6]={0,};  // 16��Ʈ�� ����
unsigned short g_TempBuffer[6]={0,};    // 16��Ʈ�� ����

unsigned char  Socket;

int g_Sw[7]={0};

int g_SwSum =0;
int g_MenuPage;

unsigned short   iinchip_source_port;
unsigned char     check_sendok_flag[8];
const float g_Error[3] = { 0.05f,  0.4f, 1.5f};//const float g_Error[4] = { 0.05f, 0.2f, 0.4f, 1.5f};
// <editor-fold defaultstate="collapsed" desc="����ü ����">
struct LOADER_STATUS{
	long	terminalSpeed;
	long	downloadSpeed;
} status;
struct ECTSSTATUS EctsStatus;
struct BUTTONVALUE{
	unsigned int s_OkButton;          // Ȯ��
	unsigned int s_BackButton;        // ����
	unsigned int s_MenuButton;        // �޴�
	unsigned int s_NextButton;        // >
	unsigned int s_BeforeButton;      // <
}ButtonValue;
struct AD7980VALUE Adc7980Value;
struct SELFTESTTYPE SelfTest;
struct PAGEVALUE{
	unsigned char s_First[2];
	unsigned char s_Check[8];
	unsigned char s_Self[5];
	unsigned char s_SelfResult[6];
	unsigned char s_Etc[6];
	unsigned char s_Line[5];
	unsigned char s_LineResult[6];
	unsigned char s_Fire[5];
	unsigned char s_FireRecvResult_1[9];
	unsigned char s_FireDetailResult_1[9];
	unsigned char s_FireRecvResult_2[9];
	unsigned char s_FireDetailResult_2[9];
} PageValue;
struct CONNECTCHECK{
	unsigned int s_Opcode;
	unsigned int s_Repeat;
	unsigned int s_SeqNo;
	unsigned int s_DataSize;

	unsigned int s_LchrId;
	unsigned int s_McuId;
}ConnectCheck;

struct CONNECTCHECKACK{
	unsigned int s_Opcode;
	unsigned int s_Repeat;
	unsigned int s_SeqNo;
	unsigned int s_DataSize;

	unsigned int s_LchrId;
	unsigned int s_McuId;
}ConnectCheckAck;

struct FIRETESTCHECK{
	unsigned int s_Opcode;
	unsigned int s_Repeat;
	unsigned int s_SeqNo;
	unsigned int s_DataSize;

	unsigned int s_LchrId;
	unsigned int s_McuId;
	unsigned int s_Time;
	unsigned int s_MSLPWR;
	unsigned int s_EXTPWR;
	unsigned int s_EBSQUIB;
	unsigned int s_BATSQUIB;
	unsigned int s_ABARSQUIB;
	unsigned int s_BDUSQUIB;
	unsigned int s_ARMING;
	unsigned int s_INTARM;
	unsigned int s_INTSQUIB;
}FireTestCheck;

struct FIRETESTCHECKRESULT{
	unsigned int s_Opcode;
	unsigned int s_Repeat;
	unsigned int s_SeqNo;
	unsigned int s_DataSize;

	unsigned int s_LchrId;
	unsigned int s_McuId;
	unsigned int s_MSLPWR;
	unsigned int s_EXTPWR;
	unsigned int s_EBSQUIB;
	unsigned int s_BATSQUIB;
	unsigned int s_ABATSQUIB;
	unsigned int s_BDUSQUIB;
	unsigned int s_ARMING;
	unsigned int s_INTARM;
	unsigned int s_INTSQUIB;
}FireTestCheckResult;

// </editor-fold>
//***************************************
void __attribute__((interrupt, auto_psv)) _U1RXInterrupt(void)
{

	unsigned char i;
	IFS0bits.U1RXIF = 0;

	g_RxBuffer[g_Buffer] = g_RxData = ReadUART1();
	g_Buffer++;
	ButtonPrintValue();

	if (U1STAbits.OERR == 1) // ���� ��Ŷ�� ������ �ʱ�ȭ �ϴ� �κ�
	{
		U1STAbits.OERR = 0;
		for(i=0; i<100; i++)
		{
			g_RxBuffer[i] = 0x00;
		}
	}
}
void __attribute__((interrupt, auto_psv)) _T1Interrupt(void)
{
	IFS0bits.T1IF = 0; // ���ͷ�Ʈ Ÿ�̸� �÷��� �ʱ�ȭ
	g_TimerCount++;
	if(g_TimerCount==2) {
		RelayControl(); // 200ms ���� ����
	}
	if(g_TimerCount==6) g_TimerCount=0; // 600ms ������ ī���� �ʱ�ȭ
}
void __attribute__((interrupt, auto_psv)) _T2Interrupt(void)
{
	IFS0bits.T2IF = 0;  //  ���ͷ�Ʈ Ÿ�̸� �÷��� �ʱ�ȭ

	g_TimerCounter++;

	if(PORTEbits.RE3 == 0)
	{
		g_OkButtonTimerCount = g_TimerCounter ;     // ��������
	}
	else
	{
		g_TimerCounter = 0;
	}
	if(g_OkButtonTimerCount > 12){
		T2CONbits.TON = 0;
		g_OkButtonTimerCount = 0;
		if(PageValue.s_SelfResult[SELF_RES_RESULT] == 1)   ResidualVoltTestResultDebug();
		else if(PageValue.s_SelfResult[SELF_SHO_RESULT] == 1)   ShortTestResultDebug();
		else if(PageValue.s_SelfResult[SELF_INS_RESULT] == 1)   InsulationTestResultDebug();

	}
	//            printf("R 140,100,200,200,255,255,255,1\r");
	//        	printf("f ���� : %d,140,100\r",g_OkButtonTimerCount);
	//            printf("R 140,200,200,200,255,255,255,1\r");
	//            printf("f �ȴ��� : %d,140,200\r",g_OkButtonTimerCountPre);


}
void __attribute__ ((interrupt, no_auto_psv)) _INT1Interrupt(void)
{
	if(PORTEbits.RE0 == 0) g_InputValue += 1;
	if(PORTEbits.RE1 == 0) g_InputValue = 2;
	if(PORTEbits.RE2 == 0) g_InputValue = 3;
	if((PORTEbits.RE3 == 0) && (g_InputValue >0))
	{
		ButtonValue.s_OkButton = 1;
		g_SwSum = g_InputValue;
	}
	ButtonValue.s_NextButton == 0 ? printf("R 80,240,104,264,255,255,255,1\r"):NULL;    // ���� ���� ��� ȭ�鿡�� ���� ��ư ��������
	if((g_InputValue != 0) && (ButtonValue.s_NextButton == 0))
	{
		printf("f %d,80,240\r",g_InputValue);
	}

	IFS1bits.INT1IF = 0;		// INT1 Interrupt Flag Clear;
	IFS1bits.INT2IF = 0;		// INT2 Interrupt Flag Clear
}
void __attribute__ ((interrupt, no_auto_psv)) _INT2Interrupt(void)
{

	if(PORTEbits.RE4 == 0)
	{
		ButtonValue.s_BackButton = (PORTEbits.RE4 == 0) ? 1 : 0;
		g_InputValue = 0;
		//printf("R 200,200,200,264,255,255,255,1\r");
		printf("f int�÷��� %d,999,999\r", ButtonValue.s_BackButton);
		printf("f int��Ʈ %d,999,999\r", PORTEbits.RE4);
	}
	if((PORTEbits.RE5 == 0) && (g_InputValue >0))
	{
		ButtonValue.s_NextButton = 1;
		g_SwSum = g_InputValue;
	}
	IFS1bits.INT1IF = 0;		// INT1 Interrupt Flag Clear;
	IFS1bits.INT2IF = 0;		// INT2 Interrupt Flag Clear
}
void OperationSwitch()
{
	g_Sw[0] =  PORTEbits.RE0;    // 1(Ȯ��)
	g_Sw[1] =  PORTEbits.RE1;    // 2(����)
	g_Sw[2] =  PORTEbits.RE2;    // 3(����)
	g_Sw[3] =  PORTEbits.RE3;    // ok(��ĭ)
	g_Sw[4] =  PORTEbits.RE4;    // ����(����)
	g_Sw[5] =  PORTEbits.RE5;    // >(�ܷ�)
}
void ButtonPrintValue(void) // ��ư�Է°� ȭ�� ��� SCI ����
{
	switch(g_RxData)
	{
	case 0x01:
		printf("R 80,240,104,264,255,255,255,1\r");
		printf("f %d,80,240\r",g_RxData);
		break;
	case 0x02:
		printf("R 80,240,104,264,255,255,255,1\r");
		printf("f %d,80,240\r",g_RxData);
		break;
	case 0x03:
		printf("R 80,240,104,264,255,255,255,1\r");
		printf("f %d,80,240\r",g_RxData);
		break;
	case 0x04:
		if((PageValue.s_Self[SELF_TEST]==1) || (PageValue.s_Line[LINE_TEST]==1) ||  (PageValue.s_Fire[FIRE_TEST]==1))
		{
			printf("R 80,240,104,264,255,255,255,1\r");
			printf("f %d,80,240\r",g_RxData);
		}
		break;
	case 0x05:
		if((PageValue.s_First[MODE_SEL]!=1) || (PageValue.s_Self[SELF_TEST]!=1) || (PageValue.s_Line[LINE_TEST]!=1) ||  (PageValue.s_Fire[FIRE_TEST]!=1))
		{
			printf("R 80,240,104,264,255,255,255,1\r");
			printf("f %d,80,240\r",g_RxData);
		}
		break;
	case 0x06:
		if((PageValue.s_First[MODE_SEL]!=1) || (PageValue.s_Self[SELF_TEST]!=1) || (PageValue.s_Line[LINE_TEST]!=1) ||  (PageValue.s_Fire[FIRE_TEST]!=1))
		{
			printf("R 80,240,104,264,255,255,255,1\r");
			printf("f %d,80,240\r",g_RxData);
		}
		break;
	case 0x07:
		if((PageValue.s_First[MODE_SEL]!=1) || (PageValue.s_Self[SELF_TEST]!=1) || (PageValue.s_Line[LINE_TEST]!=1) ||  (PageValue.s_Fire[FIRE_TEST]!=1))
		{
			printf("R 80,240,104,264,255,255,255,1\r");
			printf("f %d,80,240\r",g_RxData);
		}
		break;
	case 0x08:
		if((PageValue.s_First[MODE_SEL]!=1) || (PageValue.s_Self[SELF_TEST]!=1) || (PageValue.s_Line[LINE_TEST]!=1) ||  (PageValue.s_Fire[FIRE_TEST]!=1))
		{
			printf("R 80,240,104,264,255,255,255,1\r");
			printf("f %d,80,240\r",g_RxData);
		}
		break;
	case 0x09:
		if((PageValue.s_First[MODE_SEL]!=1) || (PageValue.s_Self[SELF_TEST]!=1) || (PageValue.s_Line[LINE_TEST]!=1) ||  (PageValue.s_Fire[FIRE_TEST]!=1))
		{
			printf("R 80,240,104,264,255,255,255,1\r");
			printf("f %d,80,240\r",g_RxData);
		}
		break;
	default:
		ButtonValue.s_OkButton = g_RxData == 0xFF ? 1:0 ;   // Ȯ�� ��ư ����
		ButtonValue.s_BackButton = g_RxData == 0xBB ? 1:0 ; // ���� ��ư ����
		ButtonValue.s_NextButton = g_RxData == 0xDD ? 1:0 ; // > ��ư ����
		break;
	}
	/*unsigned char compare=0;
	*     compare = g_RxData < 5 ? 1:2;
	if(compare == 1)
	{
	if((PageValue.s_First[MODE_SEL]==1)){
	if(g_RxData < 4){
	printf("R 80,240,104,264,255,255,255,1\r");
	printf("f %d,80,240\r",g_RxData);
	}
	}
	else if((PageValue.s_Self[SELF_TEST]==1) || (PageValue.s_Line[LINE_TEST]==1) ||  (PageValue.s_Fire[FIRE_TEST]==1)){
	if(g_RxData < 5){
	printf("R 80,240,104,264,255,255,255,1\r");
	printf("f %d,80,240\r",g_RxData);
	}
	}
	}
	else
	{
	if((PageValue.s_Fire[FIRE_TEST_EACH_1]==1) || (PageValue.s_Fire[FIRE_TEST_EACH_2]==1) || (PageValue.s_Fire[FIRE_TEST_TOTAL_1]==1) || (PageValue.s_Fire[FIRE_TEST_TOTAL_2]==1))
	{
	if(g_RxData < 10){
	printf("R 80,240,104,264,255,255,255,1\r");
	printf("f %d,80,240\r",g_RxData);
	}
	else
	{
	ButtonValue.s_OkButton = g_RxData == 0xFF ? 1:0 ;   // Ȯ�� ��ư ����
	ButtonValue.s_BackButton = g_RxData == 0xBB ? 1:0 ; // ���� ��ư ����
	ButtonValue.s_NextButton = g_RxData == 0xDD ? 1:0 ; // > ��ư ����
	}
	}
	else
	{
	ButtonValue.s_OkButton = g_RxData == 0xFF ? 1:0 ;   // Ȯ�� ��ư ����
	ButtonValue.s_BackButton = g_RxData == 0xBB ? 1:0 ; // ���� ��ư ����
	ButtonValue.s_NextButton = g_RxData == 0xDD ? 1:0 ; // > ��ư ����
	}
	}
	*/// 06�� ���� �ҽ�
	/*
	if(PageValue.s_First[MODE_SEL]==1){
	if(g_RxData < 0x04)
	{
	printf("R 80,240,104,264,255,255,255,1\r");
	printf("f %d,80,240\r",g_RxData);
	}
	}
	else if((PageValue.s_Self[SELF_TEST]==1) || (PageValue.s_Line[LINE_TEST]==1) || (PageValue.s_Fire[FIRE_TEST] ==1 ) )
	{
	if(g_RxData < 0x05)
	{
	if((PageValue.s_Etc[RES_ING]==1) || (PageValue.s_Etc[SHO_ING]==1) || (PageValue.s_Etc[INS_ING]==1) ||
	(PageValue.s_Etc[RES_RESULT]==1) || (PageValue.s_Etc[SHO_RESULT]==1) || (PageValue.s_Etc[INS_RESULT]==1))
	{

	}
	else{
	printf("R 80,240,104,264,255,255,255,1\r");
	printf("f %d,80,240\r",g_RxData);
	}
	}
	}
	else if(PageValue.s_Fire[FIRE_TEST_EACH_1]==1 || //�߻�������� �޴� ���ý�
	PageValue.s_Fire[FIRE_TEST_EACH_2]==1 ||
	PageValue.s_Fire[FIRE_TEST_TOTAL_1]==1 ||
	PageValue.s_Fire[FIRE_TEST_TOTAL_2]== 1)
	{
	if(g_RxData < 0x10)
	{
	if((PageValue.s_FireDetailResult_1[MSL_RESULT]==1) || (PageValue.s_FireDetailResult_1[EXT_RESULT]==1) || (PageValue.s_FireDetailResult_1[EB_RESULT]==1) || (PageValue.s_FireDetailResult_1[BAT_RESULT]==1)
	|| (PageValue.s_FireDetailResult_1[ABAT_RESULT]==1) || (PageValue.s_FireDetailResult_1[BDU_RESULT]==1) || (PageValue.s_FireDetailResult_1[ARMING_RESULT]==1) || (PageValue.s_FireDetailResult_1[INTARM_RESULT]==1)
	|| (PageValue.s_FireDetailResult_1[INT_RESULT]==1))
	{

	}
	else
	{
	printf("R 80,240,104,264,255,255,255,1\r");
	printf("f %d,80,240\r",g_RxData);
	}
	}
	}
	else
	{

	}*/// ���� �ҽ�
}
void InnerVoltTest(void)      // ���� ���� ���� (�Ϸ� vref. 5v, +15v, -15v ���� Ȯ�� �Ϸ�)
{
	unsigned char n;
	unsigned int *pADC_buf16;
	printf("R 0,0,480,272,255,255,255,1\r"); // ȭ�� �ʱ�ȭ
	printf("i TESTING/InnerVoltTesting.png,0,0\r");  // ���� ���� ������ ȭ�� ���
	// ���� ���� ����
	while(1)
	{
		if(AD1CON1bits.DONE == 1) // adc ��ȯ�� ������.
		{ // A/D CONTROL ????
			AD1CON1bits.DONE = 0;    // adc ��ȯ�� ������ �ʾҴ�.
		}
		else // adc ��ȯ�� �ȳ��������
		{
			pADC_buf16 = &ADCBUF0;	//ADCBUF0 = 0x0300 ~ 0x0309
			for(n=0; n<5; n++)
			{
				g_AdcBuffer[n] = *pADC_buf16++;
				g_AdcSample[n] += g_AdcBuffer[n];
			}
			if(g_AdcCount == 200)
			{
				for(n=0; n<5; n++)
				{
					g_AdcSample[n] = g_AdcSample[n] / g_AdcCount;
				}
				g_AdcCount = 1;
				//					SelfTest.s_Vref = (((float)g_AdcSample[DC_VREF])*VREF)/BIT10; // Vref �� ���ø�
				//					SelfTest.s_Dc3v = (((float)g_AdcSample[DC3V])*VREF)/BIT10*GAIN_p3V; // Dc3v �� ���ø�
				//					SelfTest.s_Dc5v = (((float)g_AdcSample[DC5V])*VREF)/BIT10*GAIN_p5V; // Dc5v �� ���ø�
				//					SelfTest.s_Dc12vPlus = (((float)g_AdcSample[DC12V_PLUS])*VREF)/BIT10*GAIN_p12V; // Dc12vPlus �� ���ø�
				//					SelfTest.s_Dc12vMinus = (-1)*((((float)g_AdcSample[DC12V_MINUS])*VREF)/BIT10*GAIN_m12V); // Dc12vMinus �� ���ø�
				SelfTest.s_Vref  = (((float)g_AdcSample[DC_VREF])*VREF)/BIT10;  //1
				SelfTest.s_Dc5v  = (((float)g_AdcSample[DC5V])*VREF)/BIT10*GAIN_p5V;    //3
				SelfTest.s_Dc12vPlus= (((float)g_AdcSample[DC12V_PLUS])*VREF)/BIT10*GAIN_p12V;  //4
				SelfTest.s_Dc12vMinus = (-1)*((((float)g_AdcSample[DC12V_MINUS])*VREF)/BIT10*GAIN_m12V);//5
				break;
			}
			g_AdcCount++;
		}
	}
	if((SelfTest.s_Vref >= (4.096f-g_Error[0]))&&(SelfTest.s_Vref <= (4.096f+g_Error[0]))){
		SET_BIT(SelfTest.s_Result,0);
	}
	else CLEAR_BIT(SelfTest.s_Result,0);

	if((SelfTest.s_Dc5v >= (5.0f-g_Error[1]))&&(SelfTest.s_Dc5v <= (5.0f+g_Error[1]))){
		SET_BIT(SelfTest.s_Result,1);
	}
	else CLEAR_BIT(SelfTest.s_Result,1);

	if((SelfTest.s_Dc12vPlus >= (15.0f-g_Error[2]))&&(SelfTest.s_Dc12vPlus <= (15.f+g_Error[2]))){
		SET_BIT(SelfTest.s_Result,2);
	}
	else CLEAR_BIT(SelfTest.s_Result,2);

	if((SelfTest.s_Dc12vMinus >= (-15.0f-g_Error[2]))&&(SelfTest.s_Dc12vMinus <= (-15.0f+g_Error[2]))){
		SET_BIT(SelfTest.s_Result,3);
	}
	else CLEAR_BIT(SelfTest.s_Result,3);
	//		if((SelfTest.s_Vref >= (4.096f-g_Error[0]))&&(SelfTest.s_Vref <= (4.096f+g_Error[0])))
	//		{     // ((Vref ���ð� >= (4.096f-������)) && (Vref ���ð� <= (4.096f+������))
	//			SET_BIT(SelfTest.s_Result,0);   // �����̸� ������� 0��Ʈ 1�� ����
	//		}
	//		else CLEAR_BIT(SelfTest.s_Result,0);    // �������̸� ������� 0��Ʈ�� 0���� ����
	//
	//		if((SelfTest.s_Dc3v >= (3.0f-g_Error[1]))&&(SelfTest.s_Dc3v <= (3.0f+g_Error[1])))
	//		{
	//			SET_BIT(SelfTest.s_Result,1);// �����̸� ������� 1��Ʈ 1�� ����
	//		}
	//            else CLEAR_BIT(SelfTest.s_Result,1);// �������̸� ������� 1��Ʈ�� 0���� ����
	//
	//		if((SelfTest.s_Dc5v >= (5.0f-g_Error[2]))&&(SelfTest.s_Dc5v <= (5.0f+g_Error[2])))
	//		{
	//			SET_BIT(SelfTest.s_Result,2);// �����̸� ������� 2��Ʈ 1�� ����
	//		}
	//		else CLEAR_BIT(SelfTest.s_Result,2);// �������̸� ������� 2��Ʈ�� 0���� ����
	//
	//		if((SelfTest.s_Dc12vPlus >= (15.0f-g_Error[3]))&&(SelfTest.s_Dc12vPlus <= (15.f+g_Error[3])))
	//		{
	//			SET_BIT(SelfTest.s_Result,3);// �����̸� ������� 3��Ʈ 1�� ����
	//		}
	//		else CLEAR_BIT(SelfTest.s_Result,3);// �������̸� ������� 3��Ʈ�� 0���� ����
	//
	//		if((SelfTest.s_Dc12vMinus >= (-15.0f-g_Error[3]))&&(SelfTest.s_Dc12vMinus <= (-15.0f+g_Error[3])))
	//		{
	//			SET_BIT(SelfTest.s_Result,4);// �����̸� ������� 4��Ʈ 1�� ����
	//		}
	//		else CLEAR_BIT(SelfTest.s_Result,4);// �������̸� ������� 4��Ʈ�� 0���� ����

	if(SelfTest.s_Result == 0x0F) // ����� ��� �����̸� ����
	{
		printf("i MENU/ModeMenu.png,0,0\r");
		PageValue.s_First[MODE_SEL] = 1;          // ��� ���� ȭ�� ���

		printf("i ETC/VoltGood.png,0,0\r");
		PageValue.s_Check[CHK_VOLT] = 1;            // ���� ���� ���� �˶� ���
		if(PORTEbits.RE6 == 0)// ��ü ���� �÷��� �ν� Ȯ��(��ü�����÷��� �� ����� =1, ��ü�����÷��� ����� = 0)
		{
			printf("i ETC/BitCheck.png,325,0\r");
			PageValue.s_Check[CHK_BIT] = 1;             // ��ü ���� ���� �˶� ���

			//������� �޽��� ������ �Լ� 
			SendData(0); // ������� �޽��� g_DataBuffer ����
			loopback_udp(3000,0); // �� ����Ȯ��
		}

	}
	else           // ���� ���� �������
	{
		printf("i ERROR/InnerVoltFail.png,0,0\r");  // ���� ���� ������
		PageValue.s_First[VOLT_ERR] = 1;              // ���� ���� ������ ���
		SelfTest.s_Result = 0; // ����� �ʱ�ȭ
	}

}
void InitValue(void)
{
	unsigned char n;

	for(n=0; n<100; n++){
		g_RxBuffer[n] = 0;
	}

	g_Buffer = 0;
	g_RxData = 0;

	memset(&PageValue, 0, sizeof(struct PAGEVALUE));    // ȭ�������� ���� �ʱ�ȭ
	memset(&ButtonValue, 0, sizeof(struct BUTTONVALUE));    // ��ư���ð� ���� �ʱ�ȭ
	memset(&Adc7980Value, 0, sizeof(struct AD7980VALUE));    // ADC�� ���� �ʱ�ȭ
	memset(&SelfTest, 0, sizeof(struct SELFTESTTYPE));    // ���� �������� ���� �ʱ�ȭ

}
void TestResultCheck(unsigned int Label)
{
	unsigned char n=0;        // ��Ʈ ��� ��
	switch(Label)
	{
	case 1 : // �ܷ�, ����/�ܼ�, ���� ���� üũ
		printf("i MENU/SelfTestMenu.png,0,0\r"); // ��ü ���� �޴� ���
		AlramPrint();
		if(PageValue.s_Self[SELF_RES_TEST]==1){ printf("i ETC/Check.png,233,73\r"); printf("i ETC/Check_.png,260,75\r"); n++;}
		if(PageValue.s_Self[SELF_SHO_TEST]==1){ printf("i ETC/Check.png,233,96\r"); printf("i ETC/Check_.png,260,97\r"); n++;}
		if(PageValue.s_Self[SELF_INS_TEST]==1){ printf("i ETC/Check.png,233,118\r"); printf("i ETC/Check_.png,260,120\r"); n++;}
		if(n>0) printf("i ETC/Hint.png,0,205\r");     // ��Ʈ ���

		break;
	case 2 : // �ܷ�, ����/�ܼ�, ���� ���� üũ
		printf("i MENU/LineTestMenu.png,0,0\r"); // ��ȭ ���� ���� �޴� ���
		AlramPrint();
		if(PageValue.s_Line[LINE_RES_TEST]==1){ printf("i ETC/Check.png,233,73\r"); printf("i ETC/Check_.png,260,75\r"); n++;}
		if(PageValue.s_Line[LINE_SHO_TEST]==1){ printf("i ETC/Check.png,233,96\r"); printf("i ETC/Check_.png,260,97\r"); n++;}
		if(PageValue.s_Line[LINE_INS_TEST]==1){ printf("i ETC/Check.png,233,118\r"); printf("i ETC/Check_.png,260,120\r"); n++;}
		if(n>0) printf("i ETC/Hint.png,0,205\r");     // ��Ʈ ���
		break;
	case 3:

		AlramPrint();
		if(PageValue.s_FireRecvResult_1[MSL_RESULT]==1)   { printf("i ETC/Check.png,197,73\r"); printf("i ETC/Check_.png,223,75\r"); n++;}
		if(PageValue.s_FireRecvResult_1[EXT_RESULT]==1)   { printf("i ETC/Check.png,197,96\r"); printf("i ETC/Check_.png,223,97\r"); n++;}
		if(PageValue.s_FireRecvResult_1[EB_RESULT]==1)    { printf("i ETC/Check.png,197,118\r"); printf("i ETC/Check_.png,223,120\r");n++;}
		if(PageValue.s_FireRecvResult_1[BAT_RESULT]==1)   { printf("i ETC/Check.png,197,140\r"); printf("i ETC/Check_.png,223,142\r"); n++;}
		if(PageValue.s_FireRecvResult_1[ABAT_RESULT]==1)  { printf("i ETC/Check.png,197,162\r"); printf("i ETC/Check_.png,223,164\r");n++;}
		if(PageValue.s_FireRecvResult_1[BDU_RESULT]==1)   { printf("i ETC/Check.png,197,184\r"); printf("i ETC/Check_.png,223,186\r"); n++;}
		if(PageValue.s_FireRecvResult_1[ARMING_RESULT]==1){ printf("i ETC/Check.png,197,207\r"); printf("i ETC/Check_.png,223,209\r"); n++;}
		if(PageValue.s_FireRecvResult_1[INTARM_RESULT]==1){ printf("i ETC/Check.png,440,73\r"); printf("i ETC/Check_.png,465,75\r"); n++;}
		if(PageValue.s_FireRecvResult_1[INT_RESULT]==1)   { printf("i ETC/Check.png,440,95\r"); printf("i ETC/Check_.png,465,97\r");n++;}
		if(n>0) printf("i ETC/Hint_1.png,240,175\r");     // ��Ʈ ���
		printf("R 80,240,104,264,255,255,255,1\r");

		break;
	case 4:

		AlramPrint();
		if(PageValue.s_FireRecvResult_2[MSL_RESULT]==1)   { printf("i ETC/Check.png,197,73\r"); printf("i ETC/Check_.png,223,75\r"); n++;}
		if(PageValue.s_FireRecvResult_2[EXT_RESULT]==1)   { printf("i ETC/Check.png,197,96\r"); printf("i ETC/Check_.png,223,97\r"); n++;}
		if(PageValue.s_FireRecvResult_2[EB_RESULT]==1)    { printf("i ETC/Check.png,197,118\r"); printf("i ETC/Check_.png,223,120\r");n++;}
		if(PageValue.s_FireRecvResult_2[BAT_RESULT]==1)   { printf("i ETC/Check.png,197,140\r"); printf("i ETC/Check_.png,223,142\r"); n++;}
		if(PageValue.s_FireRecvResult_2[ABAT_RESULT]==1)  { printf("i ETC/Check.png,197,162\r"); printf("i ETC/Check_.png,223,164\r");n++;}
		if(PageValue.s_FireRecvResult_2[BDU_RESULT]==1)   { printf("i ETC/Check.png,197,184\r"); printf("i ETC/Check_.png,223,186\r"); n++;}
		if(PageValue.s_FireRecvResult_2[ARMING_RESULT]==1){ printf("i ETC/Check.png,197,207\r"); printf("i ETC/Check_.png,223,209\r"); n++;}
		if(PageValue.s_FireRecvResult_2[INTARM_RESULT]==1){ printf("i ETC/Check.png,440,73\r"); printf("i ETC/Check_.png,465,75\r"); n++;}
		if(PageValue.s_FireRecvResult_2[INT_RESULT]==1)   { printf("i ETC/Check.png,440,95\r"); printf("i ETC/Check_.png,465,97\r");n++;}
		if(n>0) printf("i ETC/Hint_1.png,240,175\r");     // ��Ʈ ���
		printf("R 80,240,104,264,255,255,255,1\r");

		break;
	}
}
void RelayControl() // Ÿ�̸� ī���ͷ� ���� 600ms ���� �ݺ�������.
{
	unsigned int Count = 0;
	unsigned int Add = 0;
	InitRelay();
	Count = g_RelayChannel%8;
	Add = g_RelayChannel/8;

	PORTD = IOPortSelectParam[Count]; // DO0 ~ DO7 �ݺ�
	PORTG = ChipSelectParam[SEL_DATA];//PORTG = 0x0044; // CS4, CS3, CS2 DATA ����
	PORTG = ChipSelectParam[SEL_ADC];// PORTG = 0x0004; // CS3, CS2     DATA �ʱ�ȭ

	PORTD = RelaySelectParam[Add];//PORTD = 0x0008;// A3 - H -> ������ 1�� ����
	PORTG = ChipSelectParam[SEL_ADDRESS];//PORTG = 0x0006;// CS3, CS2, CS1    ADDRESS ����
	PORTG = ChipSelectParam[SEL_ADC];// PORTG = 0x0004; // CS3, CS2         ADDRESS �ʱ�ȭ

	PORTD = RelaySelectParam[CLEAR];//PORTD = 0x000f; //A0 - H, A1 - H, A2 - H, A3 - H  -> ������ ���� ����
	PORTG = ChipSelectParam[SEL_ADDRESS];//PORTG = 0x0006; // CS3, CS2, CS1    ADDRESS ����
	PORTG = ChipSelectParam[SEL_ADC];//PORTG = 0x0004; // CS3, CS2         ADDRESS �ʱ�ȭ
	ReadADCValue(50);
	g_RelayChannel++;
}
void InitRelay(void)
{
	PORTD = 0x0000;                 // DO 0~7�� ��� ���Ұ�� 0���� �ʱ�ȭ
	PORTG = ChipSelectParam[SEL_ADC];
}
void AlramPrint()
{
	if(PageValue.s_Check[CHK_VOLT] == 1) printf("i ETC/VoltGood.png,0,0\r");        // ���� ���� ����� �˶� ���
	if(PageValue.s_Check[CHK_BIT] == 1) printf("i ETC/BitCheck.png,325,0\r");        // ��ü ���� ���� �˶� ���
	if(PageValue.s_Check[CHK_LAN] == 1) printf("i ETC/LanConnect.png,165,0\r");        // LAN ������� Ȯ�� �˶�
}
void RelayLedControl(unsigned char Ch, unsigned char On_Off)
{
	// RESIDUAL_RELAY - ������ 0�� ��� , INSUL_RELAY - ������ 1�� ���
	if(On_Off == ON)
		SET_BIT(g_RelayLedStatus, Ch);
	else
		CLEAR_BIT(g_RelayLedStatus, Ch);
	PORTD = g_RelayLedStatus;
	PORTG = ChipSelectParam[SEL_LEDADC];   //  ADC(CS2) + LED(CS0)
	PORTG = ChipSelectParam[SEL_ADC];   // ADC(CS2)
}

// <editor-fold defaultstate="collapsed" desc="�ܷ���������">
void ResidualVoltTest(void)     // �ܷ� ���� ����(�Ϸ�  ADC�� ���� �ʿ�)FORI0�� �ǳ� �ٰ� ���ؾߵ�
{
	int fori;
	InitRelay();    // ������ �ʱ�ȭ

	IEC1bits.INT1IE = 0;	// ���ͷ�Ʈ 1����
	IEC1bits.INT2IE = 0;    // ���ͷ�Ʈ 2����
	RelayLedControl(RESIDUAL_RELAY,ON); // ON = 1 ������ 0��
	RelayLedControl(INSUL_RELAY,OFF);   // OFF = 0 ������ 1��
	// �����̷� ä�μ����� ADC�� ���� �а� ä�κ� ���� �����ؼ� ���� �������� ��
	memset(&EctsStatus.ResidualVoltTestResult[0],0,sizeof(EctsStatus.ResidualVoltTestResult[0])*20); // 20�� ä�� 0���� �ʱ�ȭ
	EctsStatus.ResidualVoltTestTotalResult = 0;     // �ܷ� ���� ���� ��ü ��� �ʱ�ȭ
	T1CONbits.TON = 1;  //Ÿ�̸� ���ͷ�Ʈ ����
	while(g_RelayChannel<20)
	{
		// g_RelayChannel �� 19�� �ɶ����� �ݺ� �ϸ鼭
		// Ÿ�̸� ���ͷ�Ʈ 600ms ���� g_RelayChannel ���� ���� ��Ŵ
	}
	if(g_RelayChannel == 20 )
	{
		T1CONbits.TON = 0;                          // Ÿ�̸� ���ͷ�Ʈ �̵���
		for(fori=1;fori<g_RelayChannel+1;fori++)
		{
			EctsStatus.ResidualVoltTestResult[fori].ResidualVoltTestValue = Adc7980Value.s_Value[fori];  // ä�κ��� ADC�� ���� ���� ����
			EctsStatus.ResidualVoltTestResult[fori].ResidualVoltTestValue = fabs(SelfTest.s_Vref - EctsStatus.ResidualVoltTestResult[fori].ResidualVoltTestValue);
			// ���� ���������� �������� - ADC ������ ä�ΰ��� ���밪�� = ADC�� ������ ���� �ٽ� ����
			// �������� �Ǵ� �ϴ� ������ ���ϴ� ���� : Vref - ( Vref - xV))
			// ����������� Vin - Vout ���� ��ʷ� �����Ǹ� �� ���� ���� �ؼ��� ���õȴ�. �׷��� �������а� ������ ���� �� ���� ä�ΰ��� ����
			// �׸��� �װ��� ���������� �Ǵ��ϴ� ���ذ��� ���Ͽ� ���� / ������ �Ǵ�.
			// �ܷ� ������ ���̺� ���ο� �����ִ� ���а��� �����ϴ� �˻� �� ���� ���ذ����� ������ ����/ ������ ������
			if(EctsStatus.ResidualVoltTestResult[fori].ResidualVoltTestValue < RESIDUALREFERENCE)
			{
				EctsStatus.ResidualVoltTestResult[fori].ResidualVoltTestStatus = 0;    // �ܷ� ���� ���� ��� ���� 0.1v �����ϰ�� ����
			}
			else
			{
				EctsStatus.ResidualVoltTestResult[fori].ResidualVoltTestStatus = 1;    // �ܷ� ���� ���� ��� ������ 0.1v �̻��ϰ�� ����
			}

			if(EctsStatus.ResidualVoltTestResult[fori].ResidualVoltTestStatus == 1)    // �ܷ� ���� ���� ��� �������ϰ�� ���� ��� ����
			{
				EctsStatus.ResidualVoltTestTotalResult++;
			}
		}
		g_RelayChannel = 0;
		IFS1bits.INT1IF = 0;
		IFS1bits.INT2IF = 0;
		IEC1bits.INT1IE = 1;        // ���ͷ�Ʈ1 ����
		IEC1bits.INT2IE = 1;        // ���ͷ�Ʈ2 ����
	}
}
void ResidualVoltTestResult(void)
{
	printf("i Result/ResidualVoltResult.png,0,0\r");
	AlramPrint();

	int ChannelCount;
	int x=136;
	int y=72;
	int z=400;
	for(ChannelCount=1; ChannelCount < 9 ; ChannelCount++)
	{
		if(EctsStatus.ResidualVoltTestResult[ChannelCount].ResidualVoltTestStatus == 0) printf("f O,%d,%d\r",x, y);
		else printf("f X,%d,%d\r",x, y);
		y= y + 24;
	}
	y= 72;
	for(ChannelCount=9; ChannelCount < 17 ; ChannelCount++)
	{
		if(EctsStatus.ResidualVoltTestResult[ChannelCount].ResidualVoltTestStatus == 0) printf("f O,%d,%d\r",z, y);
		else printf("f X,%d,%d\r",z, y);
		y= y + 24;
	}

}
void ResidualVoltTestResultDebug(void)
{
	printf("i Result_D/ResidualVoltResult_D.png,0,0\r");
	AlramPrint();
	int ChannelCount;
	int x=136;
	int y=72;
	int z=400;
	for(ChannelCount=0; ChannelCount < 8 ; ChannelCount++)
	{
		printf("f %.2f,%d,%d\r",EctsStatus.ResidualVoltTestResult[ChannelCount].ResidualVoltTestValue,x, y);
		y= y + 24;
	}
	y= 72;
	for(ChannelCount=8; ChannelCount < 16 ; ChannelCount++)
	{
		printf("f %.2f,%d,%d\r",EctsStatus.ResidualVoltTestResult[ChannelCount].ResidualVoltTestValue,z, y);
		y= y + 24;
	}
}
// </editor-fold>

// <editor-fold defaultstate="collapsed" desc="���� ����">
void InsulationTest(void)       // ���� ����(�Ϸ� ADC�� ���� �ʿ�)FORI0�� �ǳ� �ٰ� ���ؾߵ�
{
	int fori;
	InitRelay();    // ������ �ʱ�ȭ
	IEC1bits.INT1IE = 0;
	IEC1bits.INT2IE = 0;
	RelayLedControl(RESIDUAL_RELAY,OFF); // OFF = 0 ������ 0��
	RelayLedControl(INSUL_RELAY,ON);   // ON = 1 ������ 1��

	memset(&EctsStatus.InsulTestResult[0],0,sizeof(EctsStatus.InsulTestResult[0])*20); // 20�� ä�� 0���� �ʱ�ȭ
	EctsStatus.InsulTestTotalResult = 0;    // ���� ���� ��ü ��� �ʱ�ȭ
	T1CONbits.TON = 1;  //Ÿ�̸� ���ͷ�Ʈ ����
	while(g_RelayChannel<20)
	{

	}
	if(g_RelayChannel == 20 )
	{
		T1CONbits.TON = 0;                          // Ÿ�̸� ���ͷ�Ʈ �̵���
		for(fori=1;fori<g_RelayChannel+1;fori++)
		{
			// <editor-fold defaultstate="collapsed" desc="���� ���� ADC ���� ������ �Ǵ�">
			EctsStatus.InsulTestResult[fori].InsulTestValue = Adc7980Value.s_Value[fori];  // ä�κ��� ADC�� ���� ���� ����
			EctsStatus.InsulTestResult[fori].InsulTestValue = fabs(SelfTest.s_Vref - EctsStatus.InsulTestResult[fori].InsulTestValue);

			if(EctsStatus.InsulTestResult[fori].InsulTestValue > INSULESTREFERENCE)
			{
				EctsStatus.InsulTestResult[fori].InsulTestStatus = 1;    // ���� ���� ��� ����
			}
			else
			{
				EctsStatus.InsulTestResult[fori].InsulTestStatus = 0;    // ���� ���� ��� ������
			}

			if(EctsStatus.InsulTestResult[fori].InsulTestStatus == 1)    // ���� ���� ��� �������ϰ�� ���� ��� ����
			{
				EctsStatus.InsulTestTotalResult++;
			}
			// </editor-fold>
		}
		g_RelayChannel = 0;
		IFS1bits.INT1IF = 0;
		IFS1bits.INT2IF = 0;
		IEC1bits.INT1IE = 1;        // ���ͷ�Ʈ1 ����
		IEC1bits.INT2IE = 1;        // ���ͷ�Ʈ2 ����
	}
}
void InsulationTestResult(void)
{
	printf("i Result/InsulResult.png,0,0\r");
	AlramPrint();

	int ChannelCount;
	int x=136;
	int y=72;
	int z=400;
	for(ChannelCount=1; ChannelCount < 9 ; ChannelCount++)
	{
		if(EctsStatus.InsulTestResult[ChannelCount].InsulTestStatus == 0) printf("f O,%d,%d\r",x, y);
		else printf("f X,%d,%d\r",x, y);
		y= y + 24;
	}
	y= 72;
	for(ChannelCount=9; ChannelCount < 17 ; ChannelCount++)
	{
		if(EctsStatus.InsulTestResult[ChannelCount].InsulTestStatus == 0) printf("f O,%d,%d\r",z, y);
		else printf("f X,%d,%d\r",z, y);
		y= y + 24;
	}

}
void InsulationTestResultDebug(void)
{
	printf("i Result_D/InsulResult_D.png,0,0\r");
	AlramPrint();

	int ChannelCount;
	int x=136;
	int y=72;
	int z=400;
	for(ChannelCount=1; ChannelCount < 9 ; ChannelCount++)
	{
		printf("f %.2f,%d,%d\r",EctsStatus.InsulTestResult[ChannelCount].InsulTestValue,x, y);
		y= y + 24;
	}
	y= 72;
	for(ChannelCount=9; ChannelCount < 17 ; ChannelCount++)
	{
		printf("f %.2f,%d,%d\r",EctsStatus.InsulTestResult[ChannelCount].InsulTestValue,z, y);
		y= y + 24;
	}
}
// </editor-fold>

// <editor-fold defaultstate="collapsed" desc="���� �ܼ� ����">
void ShortTest(void)            // ����ܼ�����(�Ϸ� ADC�� ���� �ʿ�)FORI0�� �ǳ� �ٰ� ���ؾߵ�
{
	int fori;
	InitRelay();    // ������ �ʱ�ȭ
	IEC1bits.INT1IE = 0;
	IEC1bits.INT2IE = 0;
	RelayLedControl(RESIDUAL_RELAY,OFF); // OFF = 0 ������ 0��
	RelayLedControl(INSUL_RELAY,OFF);   // OFF = 0 ������ 1��
	memset(&EctsStatus.ShortTestResult[0],0,sizeof(EctsStatus.ShortTestResult[0])*20); // 20�� ä�� 0���� �ʱ�ȭ
	EctsStatus.ShortTestTotalResult = 0 ;       // ����/�ܼ� ���� ��ü ��� �ʱ�ȭ
	T1CONbits.TON = 1;  //Ÿ�̸� ���ͷ�Ʈ ����
	while(g_RelayChannel<20)
	{

	}
	if(g_RelayChannel == 20 )
	{
		T1CONbits.TON = 0;                          // Ÿ�̸� ���ͷ�Ʈ �̵���
		for(fori=1;fori<g_RelayChannel+1;fori++)
		{
			// <editor-fold defaultstate="collapsed" desc="����/�ܼ� ADC ���� ������ �Ǵ�">
			EctsStatus.ShortTestResult[fori].ShortTestValue = Adc7980Value.s_Value[fori];  // ä�κ��� ADC�� ���� ���� ����

			if(EctsStatus.ShortTestResult[fori].ShortTestValue < SHORTESTREFERENCE)
			{
				EctsStatus.ShortTestResult[fori].ShortTestStatus = 0;    // ����/�ܼ� ���� ��� ���� 0.38v ���ϸ� ����
			}
			else
			{
				EctsStatus.ShortTestResult[fori].ShortTestStatus = 1;    // ����/�ܼ� ���� ��� ������ 0.38v �̻��̸� ����
			}

			if(EctsStatus.ShortTestResult[fori].ShortTestStatus == 1)    // ����/�ܼ� ���� ��� �������ϰ�� ���� ��� ����
			{
				EctsStatus.ShortTestTotalResult++;
			}
			// </editor-fold>
		}

		g_RelayChannel = 0;
		IFS1bits.INT1IF = 0;
		IFS1bits.INT2IF = 0;
		IEC1bits.INT1IE = 1;        // ���ͷ�Ʈ1 ����
		IEC1bits.INT2IE = 1;        // ���ͷ�Ʈ2 ����
	}
}
void ShortTestResult(void)
{
	printf("i Result/ShortResult.png,0,0\r");
	AlramPrint();
	int ChannelCount;
	int x=136;
	int y=72;
	int z=400;
	for(ChannelCount=1; ChannelCount < 9 ; ChannelCount++)
	{
		if(EctsStatus.ShortTestResult[ChannelCount].ShortTestStatus == 0) printf("f O,%d,%d\r",x, y);
		else printf("f X,%d,%d\r",x, y);
		y= y + 24;
	}
	y= 72;
	for(ChannelCount=9; ChannelCount < 17 ; ChannelCount++)
	{
		if(EctsStatus.ShortTestResult[ChannelCount].ShortTestStatus == 0) printf("f O,%d,%d\r",z, y);
		else printf("f X,%d,%d\r",z, y);
		y= y + 24;
	}
}
void ShortTestResultDebug(void)
{
	printf("i Result_D/ShortResult_D.png,0,0\r");
	AlramPrint();
	int ChannelCount;
	int x=136;
	int y=72;
	int z=400;
	for(ChannelCount=1; ChannelCount < 9 ; ChannelCount++)
	{
		printf("f %.2f,%d,%d\r",EctsStatus.ShortTestResult[ChannelCount].ShortTestValue,x, y);
		y= y + 24;
	}
	y= 72;
	for(ChannelCount=9; ChannelCount < 17 ; ChannelCount++)
	{
		printf("f %.2f,%d,%d\r",EctsStatus.ShortTestResult[ChannelCount].ShortTestValue,x, y);
		y= y + 24;
	}
}
// </editor-fold>

unsigned int ReadADC7980(void)
{
	unsigned int data;
	data = 0;

	PORTG = ChipSelectParam[SEL_ADC]; // RG9 ���� �� LED ���� RG0,1
	SPI1BUF = 0x00;
	while(SPI1STATbits.SPITBF);
	while(!SPI1STATbits.SPIRBF);
	data = SPI1BUF;
	PORTG = ChipSelectParam[SEL_LED]; // RG9 ���� �� LED ���� RG0,1

	return data;
}
void ReadADCValue(unsigned int AverageCount)
{
	unsigned char n;
	Adc7980Value.s_Sum[g_RelayChannel] = 0;
	for(n=0; n<AverageCount; n++){
		Adc7980Value.s_Sum[g_RelayChannel] += ReadADC7980();
	}                                                           // �� ���ø����� 50���� ���ϴ���?, �ǹ̰� �ִ°� - > ��Ȯ�� ����
	Adc7980Value.s_Data[g_RelayChannel] = Adc7980Value.s_Sum[g_RelayChannel] / AverageCount;    // ADC�� ���ø��� 50�����ؼ� ��հ� ����
	Adc7980Value.s_Value[g_RelayChannel] = (((float)Adc7980Value.s_Data[g_RelayChannel])*VREF)/BIT16;   // ���ø����� �������а� ���ؼ� 16��Ʈ ���ش����� ����
}
void MenuDisplay(void)
{
	if(PageValue.s_First[MODE_SEL] == 1) // ���� ����                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    5
	{
		switch(g_InputValue)
		{
			// <editor-fold defaultstate="collapsed" desc="��� �޴� ����">
		case  0x01 :
			if(ButtonValue.s_OkButton == 1)
			{
				TestResultCheck(1); // ��ü ���� �޴� ���
				PageValue.s_First[MODE_SEL] = 0;
				PageValue.s_Self[SELF_TEST] = 1;
				// ȭ�� ��ȯ �ϸ鼭 �޴� ��ư �÷��� �ʱ�ȭ
				ButtonValue.s_OkButton = 0;       // Ȯ�� ��ư �ʱ�ȭ
				ButtonValue.s_BackButton = 0;
				ButtonValue.s_NextButton = 0;
				g_MenuPage = 1;
			}
			break;
		case  0x02 :
			if(ButtonValue.s_OkButton == 1)
			{
				TestResultCheck(2);
				PageValue.s_First[MODE_SEL] = 0;
				PageValue.s_Line[LINE_TEST] = 1;
				ButtonValue.s_OkButton = 0;       // Ȯ�� ��ư �ʱ�ȭ
				ButtonValue.s_BackButton = 0;
				ButtonValue.s_NextButton = 0;
				g_MenuPage = 1;
			}
			break;
		case  0x03 :
			if(ButtonValue.s_OkButton == 1)
			{
				printf("i MENU/FireTestMenu.png,0,0\r");
				AlramPrint();
				PageValue.s_First[MODE_SEL] = 0;
				PageValue.s_Fire[FIRE_TEST] = 1;
				ButtonValue.s_OkButton = 0;
				ButtonValue.s_BackButton = 0;
				ButtonValue.s_NextButton = 0;

			}
			break;
			// </editor-fold>
		}
	}
	else if(PageValue.s_Self[SELF_TEST] == 1)  //&& (PageValue.s_Check[CHK_BIT] == 1)
	{
		// <editor-fold defaultstate="collapsed" desc="��ü ���� ����">
		if(ButtonValue.s_OkButton == 1)
		{

			switch(g_InputValue)
			{
				// <editor-fold defaultstate="collapsed" desc="����">
			case 1:
				printf("i TESTING/ResidualTesting.png,0,0\r");
				AlramPrint();
				ResidualVoltTest();
				PageValue.s_Self[SELF_RES_TEST] = 1;  // ��ü ���� �Ϸ� �÷���

				printf("i MENU/SelfTestMenu.png,0,0\r"); // ��ȭ���� ���� �޴� ���
				AlramPrint();
				TestResultCheck(1);                  // ���� ��� ǥ��
				// ���� �Ϸ�� ��ư �÷��� �ʱ�ȭ
				ButtonValue.s_OkButton = 0; // ������ �Ϸ� �Ǿ
				g_MenuPage =1;// g_MenuPage == 0 ? 1:0;   //1
				g_InputValue = 0;
				break;
			case 2:
				printf("i TESTING/ShortTesting.png,0,0\r");
				AlramPrint();
				ShortTest();
				PageValue.s_Self[SELF_SHO_TEST] = 1;  // ����/�ܼ� ����
				printf("i MENU/SelfTestMenu.png,0,0\r"); // ��ȭ���� ���� �޴� ���
				AlramPrint();
				TestResultCheck(1);                  // ���� ��� ǥ��
				ButtonValue.s_OkButton = 0;       // Ȯ�� ��ư �ʱ�ȭ
				ButtonValue.s_BackButton = 0;
				ButtonValue.s_NextButton = 0;
				g_MenuPage =1;// g_MenuPage == 0 ? 1:0;   //1
				g_InputValue = 0;
				break;
			case 3:
				printf("i TESTING/InsulTesting.png,0,0\r");
				AlramPrint();
				InsulationTest();
				PageValue.s_Self[SELF_INS_TEST] = 1;  // ���� ����
				printf("i MENU/SelfTestMenu.png,0,0\r"); // ��ȭ���� ���� �޴� ���
				AlramPrint();
				TestResultCheck(1);                  // ���� ��� ǥ��
				ButtonValue.s_OkButton = 0;       // Ȯ�� ��ư �ʱ�ȭ
				ButtonValue.s_BackButton = 0;
				ButtonValue.s_NextButton = 0;
				g_MenuPage =1;// g_MenuPage == 0 ? 1:0;   //1
				g_InputValue = 0;
				break;
			case 4:

				printf("i TESTING/ResidualTesting.png,0,0\r");
				AlramPrint();
				ResidualVoltTest();
				PageValue.s_Self[SELF_RES_TEST] = 1;  // ��ü ����

				printf("i TESTING/ShortTesting.png,0,0\r");
				AlramPrint();
				ShortTest();
				PageValue.s_Self[SELF_SHO_TEST] = 1;  // ����/�ܼ� ����

				printf("i TESTING/InsulTesting.png,0,0\r");
				AlramPrint();
				InsulationTest();
				PageValue.s_Self[SELF_INS_TEST] = 1;  // ���� ����
				TestResultCheck(1);
				ButtonValue.s_OkButton = 0; // ������ �Ϸ� �Ǿ
				ButtonValue.s_BackButton = 0;
				ButtonValue.s_NextButton = 0;
				g_MenuPage =1;// g_MenuPage == 0 ? 1:0;   //1
				g_InputValue = 0;
				break;
				// </editor-fold>
			}

		}
		else if(ButtonValue.s_NextButton == 1)
		{
			switch(g_InputValue)
			{
				// <editor-fold defaultstate="collapsed" desc="�������˰��">
			case 1:
				IEC1bits.INT1IE = 0;	// ���ͷ�Ʈ 1����
				ResidualVoltTestResult();
				PageValue.s_SelfResult[SELF_RES_RESULT] = 1;       // ��ü ���� - �ܷ� ���� ��� Ȯ��
				g_MenuPage = 0;//g_MenuPage == 0 ? 1:0;   //0
				g_InputValue = 0;
				ButtonValue.s_NextButton = 0;
				T2CONbits.TON = 1;
				break;
			case 2:
				IEC1bits.INT1IE = 0;	// ���ͷ�Ʈ 1����
				ShortTestResult();
				PageValue.s_SelfResult[SELF_SHO_RESULT] = 1;       // ��ü ���� - ����ܼ� ���� ��� Ȯ��
				g_MenuPage = 0;//g_MenuPage == 0 ? 1:0;   //0
				g_InputValue = 0;
				ButtonValue.s_NextButton = 0;
				T2CONbits.TON = 1;
				break;
			case 3:
				IEC1bits.INT1IE = 0;	// ���ͷ�Ʈ 1����
				InsulationTestResult();
				PageValue.s_SelfResult[SELF_INS_RESULT] = 1;       // ��ü ���� - ���� ���� ��� Ȯ��
				g_MenuPage = 0;//g_MenuPage == 0 ? 1:0;   //0
				g_InputValue = 0;
				ButtonValue.s_NextButton = 0;
				T2CONbits.TON = 1;
				break;
				// </editor-fold>
			}
		}

		else if((ButtonValue.s_BackButton == 1) && (PORTEbits.RE4 == 0))
		{
			switch(g_MenuPage)
			{
				// <editor-fold defaultstate="collapsed" desc="����ȭ��">

			case 0:
				TestResultCheck(1);                 // ��ü ���� ��� ȭ�� ���
				if(PageValue.s_SelfResult[SELF_RES_RESULT] == 1)	PageValue.s_SelfResult[SELF_RES_RESULT] = 0;
				else if(PageValue.s_SelfResult[SELF_SHO_RESULT] == 1)	PageValue.s_SelfResult[SELF_SHO_RESULT] = 0;
				else if(PageValue.s_SelfResult[SELF_INS_RESULT] == 1)	PageValue.s_SelfResult[SELF_INS_RESULT] = 0;
				IEC1bits.INT1IE = 1;	// ���ͷ�Ʈ 1����
				// ��ü ���� ��� ȭ�� ��½� ��ư �÷��� �ʱ�ȭ
				ButtonValue.s_BackButton = 0;
				g_MenuPage = 1; //g_MenuPage == 0 ? 1:0;   //1
				break;
			case 1:
				printf("i MENU/ModeMenu.png,0,0\r");    // ��� ���� ȭ�� ���
				AlramPrint();
				PageValue.s_First[MODE_SEL] = 1;           // ��� ���� ȭ�� ���
				PageValue.s_Self[SELF_TEST] = 0;           // ��ü ���� ���(�ʱ�ȭ)
				ButtonValue.s_BackButton = 0;
				break;
				// </editor-fold>
			}
		}

		// </editor-fold>
	}

	else if(PageValue.s_Line[LINE_TEST] == 1)
	{
		// <editor-fold defaultstate="collapsed" desc="��ȭ ���� ���� ����">
		if(ButtonValue.s_OkButton == 1)
		{
			switch(g_InputValue)
			{
				// <editor-fold defaultstate="collapsed" desc="����">
			case 1:
				printf("i TESTING/ResidualTesting.png,0,0\r");
				AlramPrint();
				ResidualVoltTest();
				PageValue.s_Line[LINE_RES_TEST] = 1;  // ��ü ���� �Ϸ� �÷���
				printf("i MENU/LineTestMenu.png,0,0\r"); // ��ü ���� �޴� ���
				AlramPrint();
				TestResultCheck(2);                  // ���� ��� ǥ��
				// ���� �Ϸ�� ��ư �÷��� �ʱ�ȭ
				ButtonValue.s_OkButton = 0; // ������ �Ϸ� �Ǿ
				g_MenuPage =1;// g_MenuPage == 0 ? 1:0;   //1
				g_InputValue = 0;
				break;
			case 2:
				printf("i TESTING/ShortTesting.png,0,0\r");
				AlramPrint();
				ShortTest();
				PageValue.s_Line[LINE_SHO_TEST] = 1;  // ����/�ܼ� ����
				printf("i MENU/LineTestMenu.png,0,0\r"); // ��ü ���� �޴� ���
				AlramPrint();
				TestResultCheck(2);                  // ���� ��� ǥ��
				ButtonValue.s_OkButton = 0;       // Ȯ�� ��ư �ʱ�ȭ
				ButtonValue.s_BackButton = 0;
				ButtonValue.s_NextButton = 0;
				g_MenuPage =1;// g_MenuPage == 0 ? 1:0;   //1
				g_InputValue = 0;
				break;
			case 3:
				printf("i TESTING/InsulTesting.png,0,0\r");
				AlramPrint();
				InsulationTest();
				PageValue.s_Line[LINE_INS_TEST] = 1;  // ���� ����
				printf("i MENU/LineTestMenu.png,0,0\r"); // ��ü ���� �޴� ���
				AlramPrint();
				TestResultCheck(2);                  // ���� ��� ǥ��

				ButtonValue.s_OkButton = 0;       // Ȯ�� ��ư �ʱ�ȭ
				ButtonValue.s_BackButton = 0;
				ButtonValue.s_NextButton = 0;
				g_MenuPage =1;// g_MenuPage == 0 ? 1:0;   //1
				g_InputValue = 0;
				break;
			case 4:
				PageValue.s_Line[LINE_RES_TEST] = 1;  // ��ü ����
				printf("i TESTING/ResidualTesting.png,0,0\r");
				AlramPrint();
				ResidualVoltTest();
				PageValue.s_Line[LINE_SHO_TEST] = 1;  // ����/�ܼ� ����
				printf("i TESTING/ShortTesting.png,0,0\r");
				AlramPrint();
				ShortTest();
				PageValue.s_Line[LINE_INS_TEST] = 1;  // ���� ����
				printf("i TESTING/InsulTesting.png,0,0\r");
				AlramPrint();
				InsulationTest();
				TestResultCheck(2);                  // ���� ��� ǥ��
				ButtonValue.s_OkButton = 0; // ������ �Ϸ� �Ǿ
				ButtonValue.s_BackButton = 0;
				ButtonValue.s_NextButton = 0;
				g_MenuPage =1;// g_MenuPage == 0 ? 1:0;   //1
				g_InputValue = 0;
				break;
				// </editor-fold>
			}
		}
		else if(ButtonValue.s_NextButton == 1)
		{
			switch(g_InputValue)
			{
				// <editor-fold defaultstate="collapsed" desc="�������˰��">
			case 1:
				IEC1bits.INT1IE = 0;	// ���ͷ�Ʈ 1����
				ResidualVoltTestResult();
				PageValue.s_LineResult[LINE_RES_RESULT] = 1;       // ��ü ���� - �ܷ� ���� ��� Ȯ��
				g_MenuPage = 0;//g_MenuPage == 0 ? 1:0;   //0
				g_InputValue = 0;
				ButtonValue.s_NextButton = 0;
				break;
			case 2:
				IEC1bits.INT1IE = 0;	// ���ͷ�Ʈ 1����
				ShortTestResult();
				PageValue.s_LineResult[LINE_SHO_RESULT] = 1;       // ��ü ���� - ����ܼ� ���� ��� Ȯ��
				g_MenuPage = 0;//g_MenuPage == 0 ? 1:0;   //0
				g_InputValue = 0;
				ButtonValue.s_NextButton = 0;
				break;
			case 3:
				IEC1bits.INT1IE = 0;	// ���ͷ�Ʈ 1����
				InsulationTestResult();
				PageValue.s_LineResult[LINE_INS_RESULT] = 1;       // ��ü ���� - ���� ���� ��� Ȯ��
				g_MenuPage = 0;//g_MenuPage == 0 ? 1:0;   //0
				g_InputValue = 0;
				ButtonValue.s_NextButton = 0;
				break;
				// </editor-fold>
			}
		}
		else if((ButtonValue.s_BackButton == 1) && (PORTEbits.RE4 == 0))
		{
			switch(g_MenuPage)
			{
				// <editor-fold defaultstate="collapsed" desc="����ȭ��">

			case 0:
				TestResultCheck(2);                 // ��ü ���� ��� ȭ�� ���
				if(PageValue.s_LineResult[LINE_RES_RESULT] == 1)	PageValue.s_LineResult[LINE_RES_RESULT] = 0;
				else if(PageValue.s_LineResult[LINE_SHO_RESULT] == 1)	PageValue.s_LineResult[LINE_SHO_RESULT] = 0;
				else if(PageValue.s_LineResult[LINE_INS_RESULT] == 1)	PageValue.s_LineResult[LINE_INS_RESULT] = 0;
				IEC1bits.INT1IE = 1;	// ���ͷ�Ʈ 1����
				// ��ü ���� ��� ȭ�� ��½� ��ư �÷��� �ʱ�ȭ
				ButtonValue.s_BackButton = 0;
				g_MenuPage = 1; //g_MenuPage == 0 ? 1:0;   //1
				break;
			case 1:
				printf("i MENU/ModeMenu.png,0,0\r");    // ��� ���� ȭ�� ���
				AlramPrint();
				PageValue.s_First[MODE_SEL] = 1;           // ��� ���� ȭ�� ���
				PageValue.s_Line[LINE_TEST] = 0;           // ��ü ���� ���(�ʱ�ȭ)
				ButtonValue.s_BackButton = 0;
				break;
				// </editor-fold>
			}
		}
		// </editor-fold>
	}
	else if((PageValue.s_Fire[FIRE_TEST] == 1) )//&& (PageValue.s_Check[CHK_LAN]==1) // �߻� ���� ���� ���� �޴� ���� 1,2ȣź ����
	{
		if(ButtonValue.s_OkButton == 1)
		{
			switch(g_InputValue)
			{
				// <editor-fold defaultstate="collapsed" desc="�߻� ���� ���� ���� �޴� ����">
			case  0x01 :
				printf("i MENU/FireEachTestMenu_1.png,0,0\r");                              // 1ȣź ���� ����
				TestResultCheck(3);             // ���� ��� üũ ǥ��
				PageValue.s_Fire[FIRE_TEST_EACH_1] = 1;
				PageValue.s_Fire[FIRE_TEST] = 0;
				ButtonValue.s_OkButton = 0;       // Ȯ�� ��ư �ʱ�ȭ
				g_InputValue = 0;
				break;
			case  0x02 :
				printf("i MENU/FireTotalTestMenu_1.png,0,0\r");                              // 1ȣź ���� ����
				TestResultCheck(3);             // ���� ��� üũ ǥ��
				PageValue.s_Fire[FIRE_TEST_TOTAL_1] = 1;
				PageValue.s_Fire[FIRE_TEST] = 0;
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				break;
			case  0x03 :
				printf("i MENU/FireEachTestMenu_2.png,0,0\r");                              // 2ȣź ���� ����
				TestResultCheck(4);             // ���� ��� üũ ǥ��
				PageValue.s_Fire[FIRE_TEST_EACH_2] = 1;
				PageValue.s_Fire[FIRE_TEST] = 0;
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				break;
			case  0x04 :
				printf("i MENU/FireTotalTestMenu_2.png,0,0\r");                              // 2ȣź ���� ����
				TestResultCheck(4);             // ���� ��� üũ ǥ��
				PageValue.s_Fire[FIRE_TEST_TOTAL_2] = 1;
				PageValue.s_Fire[FIRE_TEST] = 0;
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				break;

				// </editor-fold>
			}
		}

		else if((ButtonValue.s_BackButton == 1) && (PORTEbits.RE4 == 0))
		{
			// <editor-fold defaultstate="collapsed" desc="����">
			printf("i MENU/ModeMenu.png,0,0\r");        // ��� ���� ȭ�� ���
			AlramPrint();

			PageValue.s_Fire[FIRE_TEST] = 0;
			PageValue.s_First[MODE_SEL] = 1;
			if(PageValue.s_Fire[FIRE_TEST_EACH_1] == 1)PageValue.s_Fire[FIRE_TEST_EACH_1] = 0;
			else if(PageValue.s_Fire[FIRE_TEST_TOTAL_1] == 1)PageValue.s_Fire[FIRE_TEST_TOTAL_1] = 0;
			else if(PageValue.s_Fire[FIRE_TEST_EACH_2] == 1)PageValue.s_Fire[FIRE_TEST_EACH_2] = 0;
			else if(PageValue.s_Fire[FIRE_TEST_TOTAL_2] == 1)PageValue.s_Fire[FIRE_TEST_TOTAL_2] = 0;
			ButtonValue.s_BackButton = 0;                  // ���� ��ư ���(�ʱ�ȭ)
			g_InputValue = 0;
			// </editor-fold>
		}
	}
	else if(PageValue.s_Fire[FIRE_TEST_EACH_1]==1) //&& (PageValue.s_Check[CHK_LAN])) // 1ȣź ���� ���� ���� �޽��� ���� �� ����
	{
		// <editor-fold defaultstate="collapsed" desc="1ȣź ���� ����">
		if(ButtonValue.s_OkButton == 1)
		{
			// <editor-fold defaultstate="collapsed" desc="1ȣź ���� ����">
			switch(g_InputValue)
			{
			case 1:
				PageValue.s_FireRecvResult_1[MSL_RESULT] = 1; // ����ź ���� ���� Ȯ��
				TestResultCheck(3);         // ���� ��� ǥ��
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 2:
				PageValue.s_FireRecvResult_1[EXT_RESULT] = 1; // ����ź ���� ���� Ȯ��
				TestResultCheck(3);
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 3:

				PageValue.s_FireRecvResult_1[EB_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				TestResultCheck(3);
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 4:
				PageValue.s_FireRecvResult_1[BAT_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				TestResultCheck(3);
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 5:
				PageValue.s_FireRecvResult_1[ABAT_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				TestResultCheck(3);
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 6:
				PageValue.s_FireRecvResult_1[BDU_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				TestResultCheck(3);
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 7:
				PageValue.s_FireRecvResult_1[ARMING_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				TestResultCheck(3);
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 8:
				PageValue.s_FireRecvResult_1[INTARM_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				TestResultCheck(3);
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 9:
				PageValue.s_FireRecvResult_1[INT_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				TestResultCheck(3);
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			}
			// </editor-fold>
		}
		else if(ButtonValue.s_NextButton == 1)
		{
			// <editor-fold defaultstate="collapsed" desc="1ȣź ���� ���� ���� ���">
			switch(g_InputValue)
			{
			case 1:
				printf("i Result/MSLEXTPWR.png,0,0\r");
				AlramPrint();

				PageValue.s_FireDetailResult_1[MSL_RESULT] = 1;
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;
			case 2:
				printf("i Result/TESTEXTPWR.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_1[EXT_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;

			case 3:
				printf("i Result/EBSQB.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_1[EB_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;
			case 4:
				printf("i Result/BATSQB.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_1[BAT_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;

			case 5:
				printf("i Result/ABATSQB.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_1[ABAT_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;
			case 6:
				printf("i Result/BDUSQB.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_1[BDU_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;

			case 7:
				printf("i Result/ARMING.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_1[ARMING_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;
			case 8:
				printf("i Result/INTARMING.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_1[INTARM_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;

			case 9:
				printf("i Result/INTSQB.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_1[INT_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;

			}
			// </editor-fold>
		}
		else if((ButtonValue.s_BackButton == 1) && (PORTEbits.RE4 == 0))
		{
			// <editor-fold defaultstate="collapsed" desc="����">
			switch(g_MenuPage)
			{
			case 0:
				printf("i MENU/FireEachTestMenu_1.png,0,0\r");
				TestResultCheck(3);                 // ��ü ���� ��� ȭ�� ���
				if(PageValue.s_FireDetailResult_1[MSL_RESULT]==1) PageValue.s_FireDetailResult_1[MSL_RESULT]=0;
				else if(PageValue.s_FireDetailResult_1[MSL_RESULT]==1) PageValue.s_FireDetailResult_1[MSL_RESULT]=0;
				else if(PageValue.s_FireDetailResult_1[EXT_RESULT]==1) PageValue.s_FireDetailResult_1[EXT_RESULT]=0;
				else if(PageValue.s_FireDetailResult_1[EB_RESULT]==1) PageValue.s_FireDetailResult_1[EB_RESULT]=0;
				else if(PageValue.s_FireDetailResult_1[BAT_RESULT]==1) PageValue.s_FireDetailResult_1[BAT_RESULT]=0;
				else if(PageValue.s_FireDetailResult_1[ABAT_RESULT]==1) PageValue.s_FireDetailResult_1[ABAT_RESULT]=0;
				else if(PageValue.s_FireDetailResult_1[BDU_RESULT]==1) PageValue.s_FireDetailResult_1[BDU_RESULT]=0;
				else if(PageValue.s_FireDetailResult_1[ARMING_RESULT]==1) PageValue.s_FireDetailResult_1[ARMING_RESULT]=0;
				else if(PageValue.s_FireDetailResult_1[INTARM_RESULT]==1) PageValue.s_FireDetailResult_1[INTARM_RESULT]=0;
				else if(PageValue.s_FireDetailResult_1[INT_RESULT]==1) PageValue.s_FireDetailResult_1[INT_RESULT]=0;
				IEC1bits.INT1IE = 1;	// ���ͷ�Ʈ 1����
				// ��ü ���� ��� ȭ�� ��½� ��ư �÷��� �ʱ�ȭ
				ButtonValue.s_BackButton = 0;
				g_MenuPage = 1; //g_MenuPage == 0 ? 1:0;   //1
				break;
			case 1:
				printf("i MENU/FireTestMenu.png,0,0\r");
				AlramPrint();
				PageValue.s_Fire[FIRE_TEST] = 1;
				PageValue.s_Fire[FIRE_TEST_EACH_1] = 0;
				ButtonValue.s_BackButton = 0;
				break;
			}
			// </editor-fold>
		}
		// </editor-fold>
	}

	else if(PageValue.s_Fire[FIRE_TEST_TOTAL_1]==1) //&& (PageValue.s_Check[CHK_LAN]))
	{
		// <editor-fold defaultstate="collapsed" desc="1ȣź ���� ����">
		if(ButtonValue.s_OkButton == 1)
		{
			// <editor-fold defaultstate="collapsed" desc="1ȣź ���� ����">
			switch(g_InputValue)
			{
			case 1:
				PageValue.s_FireRecvResult_1[MSL_RESULT] = 1; // ����ź ���� ���� Ȯ��
				TestResultCheck(3);         // ���� ��� ǥ��
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 2:
				PageValue.s_FireRecvResult_1[EXT_RESULT] = 1; // ����ź ���� ���� Ȯ��
				TestResultCheck(3);
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 3:

				PageValue.s_FireRecvResult_1[EB_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				TestResultCheck(3);
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 4:
				PageValue.s_FireRecvResult_1[BAT_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				TestResultCheck(3);
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 5:
				PageValue.s_FireRecvResult_1[ABAT_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				TestResultCheck(3);
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 6:
				PageValue.s_FireRecvResult_1[BDU_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				TestResultCheck(3);
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 7:
				PageValue.s_FireRecvResult_1[ARMING_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				TestResultCheck(3);
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 8:
				PageValue.s_FireRecvResult_1[INTARM_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				TestResultCheck(3);
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 9:
				PageValue.s_FireRecvResult_1[INT_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				TestResultCheck(3);
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			}
			// </editor-fold>
		}
		else if(ButtonValue.s_NextButton == 1)
		{
			// <editor-fold defaultstate="collapsed" desc="1ȣź ���� ���� ���� ���">
			switch(g_InputValue)
			{
			case 1:
				printf("i Result/MSLEXTPWR.png,0,0\r");
				AlramPrint();

				PageValue.s_FireDetailResult_1[MSL_RESULT] = 1;
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;
			case 2:
				printf("i Result/TESTEXTPWR.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_1[EXT_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;

			case 3:
				printf("i Result/EBSQB.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_1[EB_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;
			case 4:
				printf("i Result/BATSQB.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_1[BAT_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;

			case 5:
				printf("i Result/ABATSQB.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_1[ABAT_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;
			case 6:
				printf("i Result/BDUSQB.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_1[BDU_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;

			case 7:
				printf("i Result/ARMING.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_1[ARMING_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;
			case 8:
				printf("i Result/INTARMING.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_1[INTARM_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;

			case 9:
				printf("i Result/INTSQB.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_1[INT_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;

			}
			// </editor-fold>
		}
		else if((ButtonValue.s_BackButton == 1) && (PORTEbits.RE4 == 0))
		{
			// <editor-fold defaultstate="collapsed" desc="����">
			switch(g_MenuPage)
			{
			case 0:
				printf("i MENU/FireTotalTestMenu_1.png,0,0\r");
				TestResultCheck(3);                 // ��ü ���� ��� ȭ�� ���
				if(PageValue.s_FireDetailResult_1[MSL_RESULT]==1) PageValue.s_FireDetailResult_1[MSL_RESULT]=0;
				else if(PageValue.s_FireDetailResult_1[MSL_RESULT]==1) PageValue.s_FireDetailResult_1[MSL_RESULT]=0;
				else if(PageValue.s_FireDetailResult_1[EXT_RESULT]==1) PageValue.s_FireDetailResult_1[EXT_RESULT]=0;
				else if(PageValue.s_FireDetailResult_1[EB_RESULT]==1) PageValue.s_FireDetailResult_1[EB_RESULT]=0;
				else if(PageValue.s_FireDetailResult_1[BAT_RESULT]==1) PageValue.s_FireDetailResult_1[BAT_RESULT]=0;
				else if(PageValue.s_FireDetailResult_1[ABAT_RESULT]==1) PageValue.s_FireDetailResult_1[ABAT_RESULT]=0;
				else if(PageValue.s_FireDetailResult_1[BDU_RESULT]==1) PageValue.s_FireDetailResult_1[BDU_RESULT]=0;
				else if(PageValue.s_FireDetailResult_1[ARMING_RESULT]==1) PageValue.s_FireDetailResult_1[ARMING_RESULT]=0;
				else if(PageValue.s_FireDetailResult_1[INTARM_RESULT]==1) PageValue.s_FireDetailResult_1[INTARM_RESULT]=0;
				else if(PageValue.s_FireDetailResult_1[INT_RESULT]==1) PageValue.s_FireDetailResult_1[INT_RESULT]=0;
				IEC1bits.INT1IE = 1;	// ���ͷ�Ʈ 1����
				// ��ü ���� ��� ȭ�� ��½� ��ư �÷��� �ʱ�ȭ
				ButtonValue.s_BackButton = 0;
				g_MenuPage = 1; //g_MenuPage == 0 ? 1:0;   //1
				break;
			case 1:
				printf("i MENU/FireTestMenu.png,0,0\r");
				AlramPrint();
				PageValue.s_Fire[FIRE_TEST] = 1;
				PageValue.s_Fire[FIRE_TEST_EACH_1] = 0;
				ButtonValue.s_BackButton = 0;
				break;
			}
			// </editor-fold>
		}
		// </editor-fold>
	}


	else if(PageValue.s_Fire[FIRE_TEST_EACH_2]==1) //&& (PageValue.s_Check[CHK_LAN])) // 1ȣź ���� ���� ���� �޽��� ���� �� ����
	{
		// <editor-fold defaultstate="collapsed" desc="2ȣź ���� ����">
		if(ButtonValue.s_OkButton == 1)
		{
			// <editor-fold defaultstate="collapsed" desc="2ȣź ���� ����">
			switch(g_InputValue)
			{
			case 1:
				PageValue.s_FireRecvResult_2[MSL_RESULT] = 1; // ����ź ���� ���� Ȯ��
				TestResultCheck(4);         // ���� ��� ǥ��
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 2:
				PageValue.s_FireRecvResult_2[EXT_RESULT] = 1; // ����ź ���� ���� Ȯ��
				TestResultCheck(4);
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 3:

				PageValue.s_FireRecvResult_2[EB_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				TestResultCheck(4);
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 4:
				PageValue.s_FireRecvResult_2[BAT_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				TestResultCheck(4);
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 5:
				PageValue.s_FireRecvResult_2[ABAT_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				TestResultCheck(4);
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 6:
				PageValue.s_FireRecvResult_2[BDU_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				TestResultCheck(4);
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 7:
				PageValue.s_FireRecvResult_2[ARMING_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				TestResultCheck(4);
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 8:
				PageValue.s_FireRecvResult_2[INTARM_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				TestResultCheck(4);
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 9:
				PageValue.s_FireRecvResult_2[INT_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				TestResultCheck(4);
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			}
			// </editor-fold>
		}
		else if(ButtonValue.s_NextButton == 1)
		{
			// <editor-fold defaultstate="collapsed" desc="2ȣź ���� ���� ���� ���">
			switch(g_InputValue)
			{
			case 1:
				printf("i Result/MSLEXTPWR.png,0,0\r");
				AlramPrint();

				PageValue.s_FireDetailResult_2[MSL_RESULT] = 1;
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;
			case 2:
				printf("i Result/TESTEXTPWR.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_2[EXT_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;

			case 3:
				printf("i Result/EBSQB.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_2[EB_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;
			case 4:
				printf("i Result/BATSQB.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_2[BAT_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;

			case 5:
				printf("i Result/ABATSQB.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_2[ABAT_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;
			case 6:
				printf("i Result/BDUSQB.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_2[BDU_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;

			case 7:
				printf("i Result/ARMING.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_2[ARMING_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;
			case 8:
				printf("i Result/INTARMING.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_2[INTARM_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;

			case 9:
				printf("i Result/INTSQB.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_2[INT_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;

			}
			// </editor-fold>
		}
		else if((ButtonValue.s_BackButton == 1) && (PORTEbits.RE4 == 0))
		{
			// <editor-fold defaultstate="collapsed" desc="����">
			switch(g_MenuPage)
			{
			case 0:
				printf("i MENU/FireEachTestMenu_2.png,0,0\r");
				TestResultCheck(4);                 // ��ü ���� ��� ȭ�� ���
				if(PageValue.s_FireDetailResult_2[MSL_RESULT]==1) PageValue.s_FireDetailResult_2[MSL_RESULT]=0;
				else if(PageValue.s_FireDetailResult_2[EXT_RESULT]==1) PageValue.s_FireDetailResult_2[EXT_RESULT]=0;
				else if(PageValue.s_FireDetailResult_2[EB_RESULT]==1) PageValue.s_FireDetailResult_2[EB_RESULT]=0;
				else if(PageValue.s_FireDetailResult_2[BAT_RESULT]==1) PageValue.s_FireDetailResult_2[BAT_RESULT]=0;
				else if(PageValue.s_FireDetailResult_2[ABAT_RESULT]==1) PageValue.s_FireDetailResult_2[ABAT_RESULT]=0;
				else if(PageValue.s_FireDetailResult_2[BDU_RESULT]==1) PageValue.s_FireDetailResult_2[BDU_RESULT]=0;
				else if(PageValue.s_FireDetailResult_2[ARMING_RESULT]==1) PageValue.s_FireDetailResult_2[ARMING_RESULT]=0;
				else if(PageValue.s_FireDetailResult_2[INTARM_RESULT]==1) PageValue.s_FireDetailResult_2[INTARM_RESULT]=0;
				else if(PageValue.s_FireDetailResult_2[INT_RESULT]==1) PageValue.s_FireDetailResult_2[INT_RESULT]=0;
				IEC1bits.INT1IE = 1;	// ���ͷ�Ʈ 1����
				// ��ü ���� ��� ȭ�� ��½� ��ư �÷��� �ʱ�ȭ
				ButtonValue.s_BackButton = 0;
				g_MenuPage = 1; //g_MenuPage == 0 ? 1:0;   //1
				break;
			case 1:
				printf("i MENU/FireTestMenu.png,0,0\r");
				AlramPrint();
				PageValue.s_Fire[FIRE_TEST] = 1;
				PageValue.s_Fire[FIRE_TEST_EACH_2] = 0;
				ButtonValue.s_BackButton = 0;
				break;
			}
			// </editor-fold>
		}
		// </editor-fold>
	}
	else if(PageValue.s_Fire[FIRE_TEST_TOTAL_2]==1) //&& (PageValue.s_Check[CHK_LAN])) // 2ȣź ���� ���� ���� �޽��� ���� �� ����
	{
		// <editor-fold defaultstate="collapsed" desc="2ȣź ���� ����">
		if(ButtonValue.s_OkButton == 1)
		{
			// <editor-fold defaultstate="collapsed" desc="2ȣź ���� ����">
			switch(g_InputValue)
			{
			case 1:
				PageValue.s_FireRecvResult_2[MSL_RESULT] = 1; // ����ź ���� ���� Ȯ��
				TestResultCheck(4);         // ���� ��� ǥ��
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 2:
				PageValue.s_FireRecvResult_2[EXT_RESULT] = 1; // ����ź ���� ���� Ȯ��
				TestResultCheck(4);
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 3:

				PageValue.s_FireRecvResult_2[EB_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				TestResultCheck(4);
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 4:
				PageValue.s_FireRecvResult_2[BAT_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				TestResultCheck(4);
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 5:
				PageValue.s_FireRecvResult_2[ABAT_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				TestResultCheck(4);
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 6:
				PageValue.s_FireRecvResult_2[BDU_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				TestResultCheck(4);
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 7:
				PageValue.s_FireRecvResult_2[ARMING_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				TestResultCheck(4);
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 8:
				PageValue.s_FireRecvResult_2[INTARM_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				TestResultCheck(4);
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 9:
				PageValue.s_FireRecvResult_2[INT_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				TestResultCheck(4);
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			}
			// </editor-fold>
		}
		else if(ButtonValue.s_NextButton == 1)
		{
			// <editor-fold defaultstate="collapsed" desc="2ȣź ���� ���� ���� ���">
			switch(g_InputValue)
			{
			case 1:
				printf("i Result/MSLEXTPWR.png,0,0\r");
				AlramPrint();

				PageValue.s_FireDetailResult_2[MSL_RESULT] = 1;
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;
			case 2:
				printf("i Result/TESTEXTPWR.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_2[EXT_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;

			case 3:
				printf("i Result/EBSQB.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_2[EB_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;
			case 4:
				printf("i Result/BATSQB.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_2[BAT_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;

			case 5:
				printf("i Result/ABATSQB.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_2[ABAT_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;
			case 6:
				printf("i Result/BDUSQB.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_2[BDU_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;

			case 7:
				printf("i Result/ARMING.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_2[ARMING_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;
			case 8:
				printf("i Result/INTARMING.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_2[INTARM_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;

			case 9:
				printf("i Result/INTSQB.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_2[INT_RESULT] = 1;              // �߻� ���� ���� ���� �޴� ���
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;

			}
			// </editor-fold>
		}
		else if((ButtonValue.s_BackButton == 1) && (PORTEbits.RE4 == 0))
		{
			// <editor-fold defaultstate="collapsed" desc="����">
			switch(g_MenuPage)
			{
			case 0:
				printf("i MENU/FireTotalTestMenu_2.png,0,0\r");
				TestResultCheck(4);                 // ��ü ���� ��� ȭ�� ���
				if(PageValue.s_FireDetailResult_2[MSL_RESULT]==1) PageValue.s_FireDetailResult_2[MSL_RESULT]=0;
				else if(PageValue.s_FireDetailResult_2[EXT_RESULT]==1) PageValue.s_FireDetailResult_2[EXT_RESULT]=0;
				else if(PageValue.s_FireDetailResult_2[EB_RESULT]==1) PageValue.s_FireDetailResult_2[EB_RESULT]=0;
				else if(PageValue.s_FireDetailResult_2[BAT_RESULT]==1) PageValue.s_FireDetailResult_2[BAT_RESULT]=0;
				else if(PageValue.s_FireDetailResult_2[ABAT_RESULT]==1) PageValue.s_FireDetailResult_2[ABAT_RESULT]=0;
				else if(PageValue.s_FireDetailResult_2[BDU_RESULT]==1) PageValue.s_FireDetailResult_2[BDU_RESULT]=0;
				else if(PageValue.s_FireDetailResult_2[ARMING_RESULT]==1) PageValue.s_FireDetailResult_2[ARMING_RESULT]=0;
				else if(PageValue.s_FireDetailResult_2[INTARM_RESULT]==1) PageValue.s_FireDetailResult_2[INTARM_RESULT]=0;
				else if(PageValue.s_FireDetailResult_2[INT_RESULT]==1) PageValue.s_FireDetailResult_2[INT_RESULT]=0;
				IEC1bits.INT1IE = 1;	// ���ͷ�Ʈ 1����
				// ��ü ���� ��� ȭ�� ��½� ��ư �÷��� �ʱ�ȭ
				ButtonValue.s_BackButton = 0;
				g_MenuPage = 1; //g_MenuPage == 0 ? 1:0;   //1
				break;
			case 1:
				printf("i MENU/FireTestMenu.png,0,0\r");
				AlramPrint();
				PageValue.s_Fire[FIRE_TEST] = 1;
				PageValue.s_Fire[FIRE_TEST_TOTAL_2] = 0;
				ButtonValue.s_BackButton = 0;
				break;
			}
			// </editor-fold>
		}
		// </editor-fold>
	}

}

void SendData(unsigned int Name)    // 16��Ʈ �迭�� ���۵�
{
	switch(Name)
	{
	case 0: //  ��� ����
		ConnectCheck.s_Opcode = 0x4005;
		ConnectCheck.s_Repeat = 0;
		ConnectCheck.s_SeqNo = 0;
		ConnectCheck.s_DataSize = 0x0004;
		ConnectCheck.s_LchrId = 0x0001;
		ConnectCheck.s_McuId = 0x0001;

		g_DataBuffer[0] =  ConnectCheck.s_Opcode;
		g_DataBuffer[1] =  ConnectCheck.s_Repeat;
		g_DataBuffer[2] =  ConnectCheck.s_SeqNo;
		g_DataBuffer[3] =  ConnectCheck.s_DataSize;
		g_DataBuffer[4] =  ConnectCheck.s_LchrId;
		g_DataBuffer[5] =  ConnectCheck.s_McuId;
		break;
	case 1: // 1ȣź �߻� ���� ����
		if(PageValue.s_Fire[FIRE_TEST_EACH_1]==1) FireTestCheck.s_Opcode = 0x4100;
		else if(PageValue.s_Fire[FIRE_TEST_EACH_1]==1) FireTestCheck.s_Opcode = 0x4200;
		FireTestCheck.s_Repeat = 0;
		FireTestCheck.s_SeqNo = 0;
		FireTestCheck.s_DataSize = 0x0008;
		FireTestCheck.s_LchrId = 0x0001;
		FireTestCheck.s_McuId = 0x0001;
		FireTestCheck.s_Time = 0;
		g_DataBuffer[0] = FireTestCheck.s_Opcode;
		g_DataBuffer[1] =FireTestCheck.s_Repeat;
		g_DataBuffer[2] =FireTestCheck.s_SeqNo;
		g_DataBuffer[3] =FireTestCheck.s_DataSize;
		g_DataBuffer[4] =(FireTestCheck.s_LchrId << 7);
		g_DataBuffer[4] =FireTestCheck.s_McuId ;
		g_DataBuffer[5] =FireTestCheck.s_Time;
		if(PageValue.s_FireRecvResult_1[MSL_RESULT] == 1)
		{
			FireTestCheck.s_MSLPWR = 0x02;
			FireTestCheck.s_EXTPWR = 0;
			FireTestCheck.s_EBSQUIB = 0;
			FireTestCheck.s_BATSQUIB = 0;
			FireTestCheck.s_ABARSQUIB = 0;
			FireTestCheck.s_BDUSQUIB = 0;
			FireTestCheck.s_ARMING = 0;
			FireTestCheck.s_INTARM = 0;
			FireTestCheck.s_INTSQUIB = 0;
			g_DataBuffer[6] =  (FireTestCheck.s_MSLPWR << 13);
			g_DataBuffer[7] = 0;
		}
		else if(PageValue.s_FireRecvResult_1[EXT_RESULT] == 1)
		{
			FireTestCheck.s_MSLPWR = 0;
			FireTestCheck.s_EXTPWR = 0x01;
			FireTestCheck.s_EBSQUIB = 0;
			FireTestCheck.s_BATSQUIB = 0;
			FireTestCheck.s_ABARSQUIB = 0;
			FireTestCheck.s_BDUSQUIB = 0;
			FireTestCheck.s_ARMING = 0;
			FireTestCheck.s_INTARM = 0;
			FireTestCheck.s_INTSQUIB = 0;
			g_DataBuffer[6] =  (FireTestCheck.s_EXTPWR << 11);
			g_DataBuffer[7] = 0;
		}
		else if(PageValue.s_FireRecvResult_1[EB_RESULT] == 1)
		{
			FireTestCheck.s_MSLPWR = 0;
			FireTestCheck.s_EXTPWR = 0;
			FireTestCheck.s_EBSQUIB = 0x01;
			FireTestCheck.s_BATSQUIB = 0;
			FireTestCheck.s_ABARSQUIB = 0;
			FireTestCheck.s_BDUSQUIB = 0;
			FireTestCheck.s_ARMING = 0;
			FireTestCheck.s_INTARM = 0;
			FireTestCheck.s_INTSQUIB = 0;
			g_DataBuffer[6] =  (FireTestCheck.s_EBSQUIB << 9);
			g_DataBuffer[7] = 0;
		}
		else if(PageValue.s_FireRecvResult_1[BAT_RESULT] == 1)
		{
			FireTestCheck.s_MSLPWR = 0;
			FireTestCheck.s_EXTPWR = 0;
			FireTestCheck.s_EBSQUIB = 0;
			FireTestCheck.s_BATSQUIB = 0x01;
			FireTestCheck.s_ABARSQUIB = 0;
			FireTestCheck.s_BDUSQUIB = 0;
			FireTestCheck.s_ARMING = 0;
			FireTestCheck.s_INTARM = 0;
			FireTestCheck.s_INTSQUIB = 0;
			g_DataBuffer[6] =  (FireTestCheck.s_BATSQUIB << 7);
			g_DataBuffer[7] = 0;
		}
		else if(PageValue.s_FireRecvResult_1[ABAT_RESULT] == 1)
		{
			FireTestCheck.s_MSLPWR = 0;
			FireTestCheck.s_EXTPWR = 0;
			FireTestCheck.s_EBSQUIB = 0;
			FireTestCheck.s_BATSQUIB = 0;
			FireTestCheck.s_ABARSQUIB = 0x01;
			FireTestCheck.s_BDUSQUIB = 0;
			FireTestCheck.s_ARMING = 0;
			FireTestCheck.s_INTARM = 0;
			FireTestCheck.s_INTSQUIB = 0;
			g_DataBuffer[6] =  (FireTestCheck.s_ABARSQUIB << 5);
			g_DataBuffer[7] = 0;
		}
		else if(PageValue.s_FireRecvResult_1[BDU_RESULT] == 1)
		{
			FireTestCheck.s_MSLPWR = 0;
			FireTestCheck.s_EXTPWR = 0;
			FireTestCheck.s_EBSQUIB = 0;
			FireTestCheck.s_BATSQUIB = 0;
			FireTestCheck.s_ABARSQUIB = 0;
			FireTestCheck.s_BDUSQUIB = 0x01;
			FireTestCheck.s_ARMING = 0;
			FireTestCheck.s_INTARM = 0;
			FireTestCheck.s_INTSQUIB = 0;
			g_DataBuffer[6] =  (FireTestCheck.s_BDUSQUIB << 3);
			g_DataBuffer[7] = 0;
		}
		else if(PageValue.s_FireRecvResult_1[ARMING_RESULT] == 1)
		{
			FireTestCheck.s_MSLPWR = 0;
			FireTestCheck.s_EXTPWR = 0;
			FireTestCheck.s_EBSQUIB = 0;
			FireTestCheck.s_BATSQUIB = 0;
			FireTestCheck.s_ABARSQUIB = 0;
			FireTestCheck.s_BDUSQUIB = 0;
			FireTestCheck.s_ARMING = 0x01;
			FireTestCheck.s_INTARM = 0;
			FireTestCheck.s_INTSQUIB = 0;
			g_DataBuffer[6] =  (FireTestCheck.s_ARMING << 1);
			g_DataBuffer[7] = 0;
		}
		else if(PageValue.s_FireRecvResult_1[INTARM_RESULT] == 1)
		{
			FireTestCheck.s_MSLPWR = 0;
			FireTestCheck.s_EXTPWR = 0;
			FireTestCheck.s_EBSQUIB = 0;
			FireTestCheck.s_BATSQUIB = 0;
			FireTestCheck.s_ABARSQUIB = 0;
			FireTestCheck.s_BDUSQUIB = 0;
			FireTestCheck.s_ARMING = 0;
			FireTestCheck.s_INTARM = 0x01;
			FireTestCheck.s_INTSQUIB = 0;
			g_DataBuffer[6] =  FireTestCheck.s_INTARM << 0;
			g_DataBuffer[7] = 0;
		}
		else if(PageValue.s_FireRecvResult_1[INT_RESULT] == 1)
		{
			FireTestCheck.s_MSLPWR = 0;
			FireTestCheck.s_EXTPWR = 0;
			FireTestCheck.s_EBSQUIB = 0;
			FireTestCheck.s_BATSQUIB = 0;
			FireTestCheck.s_ABARSQUIB = 0;
			FireTestCheck.s_BDUSQUIB = 0;
			FireTestCheck.s_ARMING = 0;
			FireTestCheck.s_INTARM = 0;
			FireTestCheck.s_INTSQUIB = 0x01;
			g_DataBuffer[6] = 0;
			g_DataBuffer[7] = (FireTestCheck.s_INTSQUIB << 13);
		}

		break;

	}
}

void RecvData(unsigned int Name)                     // 16��Ʈ �迭�� ���ŵ�
{
	switch(Name)
	{
	case 0: // ��� ���� ���� 
		ConnectCheckAck.s_Opcode = g_TempBuffer[0];
		ConnectCheckAck.s_Repeat = g_TempBuffer[1];
		ConnectCheckAck.s_SeqNo = g_TempBuffer[2];
		ConnectCheckAck.s_DataSize = g_TempBuffer[3];
		ConnectCheckAck.s_LchrId = g_TempBuffer[4];
		ConnectCheckAck.s_McuId = g_TempBuffer[5];
		if( ConnectCheckAck.s_Opcode == 0x4005) PageValue.s_Check[CHK_LAN] = 1; // �� ����� 
		break;
	case 1:
		FireTestCheckResult.s_Opcode = g_TempBuffer[0]; 
		FireTestCheckResult.s_Repeat = g_TempBuffer[1];
		FireTestCheckResult.s_SeqNo = g_TempBuffer[2];
		FireTestCheckResult.s_DataSize = g_TempBuffer[3];
		FireTestCheckResult.s_LchrId = (g_TempBuffer[4] & 0xFF00);
		FireTestCheckResult.s_McuId = (g_TempBuffer[4] & 0x00FF);

		FireTestCheckResult.s_MSLPWR = (g_TempBuffer[5] & 0xF000); 
		FireTestCheckResult.s_EXTPWR = (g_TempBuffer[5] & 0x0F00);
		FireTestCheckResult.s_EBSQUIB = (g_TempBuffer[5] & 0x00FF);

		FireTestCheckResult.s_BATSQUIB = (g_TempBuffer[6] & 0xFF00);
		FireTestCheckResult.s_ABATSQUIB = (g_TempBuffer[6] & 0x00FF);

		FireTestCheckResult.s_BDUSQUIB = (g_TempBuffer[7] & 0xF000);
		FireTestCheckResult.s_ARMING = g_TempBuffer[7] & 0x0F00;
		FireTestCheckResult.s_INTARM = (g_TempBuffer[7] & 0x00F0);
		FireTestCheckResult.s_INTSQUIB = (g_TempBuffer[7] & 0x000F);
		break;
	}
}


unsigned char     socket(  unsigned char  protocol,  unsigned short port,  unsigned short mode)
{
	IINCHIP_WRITE(Sn_MR(7),( unsigned short)(protocol | mode)); // set Sn_MR with protocol & flag 0x0000 0000 0800 03C0, 0x02--1
	if (port != 0) IINCHIP_WRITE(Sn_PORTR(7),port); // 0x0000 0000 0800 03CA, 3000--2
	else
	{
		iinchip_source_port++;     // if don't set the source port, set local_port number.
		IINCHIP_WRITE(Sn_PORTR(7),iinchip_source_port);  
	}
	setSn_CR(7, Sn_CR_OPEN);      // open s-th SOCKET--3

	check_sendok_flag[7] = 1;     // initialize the sendok flag.

#ifdef __DEF_IINCHIP_DBG__
	printf("%d : Sn_MR=0x%04x,Sn_PORTR=0x%04x(%d),Sn_SSR=%04x\r\n",s,IINCHIP_READ(Sn_MR(s)),IINCHIP_READ(Sn_PORTR(s)),IINCHIP_READ(Sn_PORTR(s)),getSn_SSR(s));
#endif
	return 1;
}

void     close()
{
	// M_08082008 : It is fixed the problem that Sn_SSR cannot be changed a undefined value to the defined value.
	//              Refer to Errata of W5300
	//Check if the transmit data is remained or not.
	if( ((getSn_MR(7)& 0x0F) == Sn_MR_TCP) && (getSn_TX_FSR(7) != getIINCHIP_TxMAX(7)) )
	{
		unsigned short loop_cnt =0;
		while(getSn_TX_FSR(7) != getIINCHIP_TxMAX(7))
		{
			if(loop_cnt++ > 10)
			{
				unsigned char  destip[4];
				// M_11252008 : modify dest ip address
				//getSIPR(destip);
				destip[0] = 0;destip[1] = 0;destip[2] = 0;destip[3] = 1;
				socket(Sn_MR_UDP,0x3000,0);//0x02
				sendto(( unsigned char *)"x",1,destip,0x3000); // send the dummy data to an unknown destination(0.0.0.1).
				break; // M_11252008 : added break statement
			}
			wait_10ms(10);
		}
	};
	////////////////////////////////
	setSn_IR(7 ,0x00FF);          // Clear the remained interrupt bits.
	setSn_CR(7 ,Sn_CR_CLOSE);     // Close s-th SOCKET
}

unsigned long   sendto( unsigned char  * buf, unsigned long len,  unsigned char  * addr,  unsigned short port)
{
	unsigned char  status=0;
	unsigned char  isr=0;
	unsigned long ret=0;

#ifdef __DEF_IINCHIP_DBG__
	printf("%d : sendto():%d.%d.%d.%d(%d), len=%d\r\n",s, addr[0], addr[1], addr[2], addr[3] , port, len);
#endif

	if
		(
		((addr[0] == 0x00) && (addr[1] == 0x00) && (addr[2] == 0x00) && (addr[3] == 0x00)) ||
		((port == 0x00)) ||(len == 0)
		)
	{
#ifdef __DEF_IINCHIP_DBG__
		printf("%d : Fail[%d.%d.%d.%d, %.d, %d]\r\n",s, addr[0], addr[1], addr[2], addr[3] , port, len);
#endif
		return 0;
	}


	if (len > getIINCHIP_TxMAX(7)) ret = getIINCHIP_TxMAX(7); // check size not to exceed MAX size.
	else ret = len;

	// set destination IP address
	IINCHIP_WRITE(Sn_DIPR(7),((( unsigned short)addr[0])<<8) + ( unsigned short) addr[1]);
	IINCHIP_WRITE(Sn_DIPR2(7),((( unsigned short)addr[2])<<8) + ( unsigned short) addr[3]);
	// set destination port number
	IINCHIP_WRITE(Sn_DPORTR(7),port);

	wiz_write_buf(7, buf, ret);                              // copy data
	// send
	setSn_TX_WRSR(7,ret);
	clearSUBR();
	setSn_CR(7, Sn_CR_SEND);

	while (!((isr = getSn_IR(7)) & Sn_IR_SENDOK))            // wait SEND command completion
	{
		status = getSn_SSR(7);                                // warning ---------------------------------------
		if ((status == SOCK_CLOSED) || (isr & Sn_IR_TIMEOUT)) // Sn_IR_TIMEOUT causes the decrement of Sn_TX_FSR
		{                                                     // -----------------------------------------------
#ifdef __DEF_IINCHIP_DBG__
			printf("%d: send fail.status=0x%02x,isr=%02x\r\n",status,isr);
#endif
			setSn_IR(7,Sn_IR_TIMEOUT);
			return 0;
		}
	}
	applySUBR();
	setSn_IR(7, Sn_IR_SENDOK); // Clear Sn_IR_SENDOK


#ifdef __DEF_IINCHIP_DBG__
	printf("%d : send()end\r\n",s);
#endif

	return ret;
}

unsigned long   recvfrom( unsigned char  * buf, unsigned long len,  unsigned char  * addr,  unsigned short  *port)
{
	unsigned short head[4];
	unsigned long data_len=0;


#ifdef __DEF_IINCHIP_DBG__
	printf("recvfrom()\r\n");
#endif

	if ( len > 0 )
	{
		switch (IINCHIP_READ(Sn_MR(7)) & 0x07)       // check the mode of s-th SOCKET
		{                                            // -----------------------------
		case Sn_MR_UDP :                          // UDP mode
			wiz_read_buf(7, ( unsigned char *)head, 8);      // extract the PACKET-INFO
			// read peer's IP address, port number.
			if(*(( volatile unsigned short*)MR) & MR_FS)            // check FIFO swap bit
			{
				head[0] = ((((head[0] << 8 ) & 0xFF00)) | ((head[0] >> 8)& 0x00FF));
				head[1] = ((((head[1] << 8 ) & 0xFF00)) | ((head[1] >> 8)& 0x00FF));
				head[2] = ((((head[2] << 8 ) & 0xFF00)) | ((head[2] >> 8)& 0x00FF));
				head[3] = ((((head[3] << 8 ) & 0xFF00)) | ((head[3] >> 8)& 0x00FF));
			}
			addr[0] = ( unsigned char )(head[0] >> 8);       // destination IP address
			addr[1] = ( unsigned char )head[0];
			addr[2] = ( unsigned char )(head[1]>>8);
			addr[3] = ( unsigned char )head[1];
			*port = head[2];                       // destination port number
			data_len = (unsigned long)head[3];            // DATA packet length

#ifdef __DEF_IINCHIP_DBG__
			printf("UDP msg arrived:%d(0x%04x)\r\n",data_len,data_len);
			printf("source Port : %d\r\n", *port);
			printf("source IP : %d.%d.%d.%d\r\n", addr[0], addr[1], addr[2], addr[3]);
#endif

			wiz_read_buf(7, buf, data_len);        // data copy.
			break;
		default :
			break;
		}
		setSn_CR(7,Sn_CR_RECV);                      // recv
	}
#ifdef __DEF_IINCHIP_DBG__
	printf("recvfrom() end ..\r\n");
#endif

	return data_len;
}

void     loopback_udp(  unsigned short port,   unsigned short mode)
{
	unsigned long len;
	unsigned char  destip[4];
	unsigned short destport;

	switch(getSn_SSR(7))
	{
	case SOCK_UDP:                                     //
		if((len=getSn_RX_RSR(7)) > 0)                   // check the size of received data
		{
			len = recvfrom(&g_TempBuffer,len,destip,&destport);  // receive data from a destination
			if(len !=sendto(&g_DataBuffer,len,destip,destport))  // send the data to the destination
			{
				printf("%d : Sendto Fail.len=%d,",7,len);
				printf("%d.%d.%d.%d(%d)\r\n",destip[0],destip[1],destip[2],destip[3],destport);
			}
		}
		break;
		// -----------------
	case SOCK_CLOSED:                                  // CLOSED
		close();                                       // close the SOCKET
		socket(Sn_MR_UDP,port,mode);                  // open the SOCKET with UDP mode
		break;
	default:
		break;
	}
}

void UdpSetting()
{
	unsigned char  tx_mem_conf[8] = {8,8,8,8,8,8,8,8};          // for setting TMSR regsiter
	unsigned char  rx_mem_conf[8] = {8,8,8,8,8,8,8,8};          // for setting RMSR regsiter


	unsigned char  ip[4] = {192,168,111,200};                   // for setting SIP register
	unsigned char  gw[4] = {192,168,111,1};                     // for setting GAR register
	unsigned char  sn[4] = {255,255,255,0};                     // for setting SUBR register
	unsigned char  mac[6] = {0x00,0x08,0xDC,0x00,111,200};      // for setting SHAR register

	unsigned char  serverip[4] = {192,168,111,78};              // "TCP SERVER" IP address for loopback_tcpc()

	status.terminalSpeed = 0x08;
	status.downloadSpeed = 0x08;
	/* initiate W5300 */
	iinchip_init();

	/* allocate internal TX/RX Memory of W5300 */
	if(!sysinit(tx_mem_conf,rx_mem_conf))
	{
		printf("MEMORY CONFIG ERR.\r\n");
		while(1);
	}

	/* verify network information */
	getSHAR(mac);                                      // get source hardware address
	getGAR(gw);                                        // get gateway IP address
	getSUBR(sn);                                       // get subnet mask address
	getSIPR(ip);                                       // get source IP address

	printf("SHAR : %02x:%02x:%02x:%02x:%02x:%02x\r\n",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	printf("GWR  : %d.%d.%d.%d\r\n",gw[0],gw[1],gw[2],gw[3]);
	printf("SUBR : %d.%d.%d.%d\r\n",sn[0],sn[1],sn[2],sn[3]);
	printf("SIPR : %d.%d.%d.%d\r\n",ip[0],ip[1],ip[2],ip[3]);
}
int main(void)
{
	InitGPIO();          // gpio �ʱ�ȭ
	InitValue();           // ���� �ʱ�ȭ
	InitUart1();           // uart �ʱ�ȭ
	InitSpi1();            // spi �ʱ�ȭ
	InitAdc();
	InitRelay();            // ������ ��Ʈ �ʱ�ȭ
	InitTimer1();
	InitTimer2();
	InnerVoltTest();      // ���� ���� ����
	InitInterrupt();       // ���ͷ�Ʈ �ʱ�ȭ
	UdpSetting();           // udp ��� ����
	IFS1bits.INT1IF = 0;		// INT1 Interrupt Flag Clear
	IFS1bits.INT2IF = 0;		// INT2 Interrupt Flag Clear
	IFS0bits.T1IF = 0;			// T1 Interrupt Flag Clear
	IFS0bits.T2IF = 0;			// T2 Interrupt Flag Clear


	if( PageValue.s_First[VOLT_ERR] == 0 )
	{
		while(1)
		{
			if(PageValue.s_Check[CHK_VOLT] == 1)   MenuDisplay();// ȭ�� ��� �κ�

		}
	}
	return(0);
}
//:DONE: