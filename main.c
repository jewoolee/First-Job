/*
* File:   main.c
* Author: lee
*
* Created on 2017년 6월 22일 (목), 오전 8:51
*/

#include <p33FJ128GP310A.h>
#include <libpic30.h>
#include <string.h>
#include <math.h>
#include <timer.h>
#include <uart.h>
#include <stdio.h>
#include <stdlib.h>
#include "lib/init_drv.h"
#include "lib/data_define.h"
#include "lib/socket.h"

//Configuration Bits 설정
//_FOSC(ECIO_PLL4);//_FOSC(ECIO_PLL8);
//_FWDT(WDT_OFF);
//_FBORPOR(MCLR_EN && PWRT_OFF);
void InitXHyper255A(void);       // Intialize MCU
void     loopback_udp(SOCKET s, uint16 port, uint8* buf, uint16 mode);
void W5300_Setting();
void     loopback_udp(SOCKET s, uint16 port, uint8* buf, uint16 mode);

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
void ReadADCValue(unsigned int AverageCount);
void AlramPrint();
void SendData(unsigned int Name);
void RecvDataParsing(unsigned int Name);
void RecvDataSort(unsigned int Name, unsigned int Num);

// 인터럽트 선언 부************************************************
void __attribute__((interrupt, auto_psv)) _U1RXInterrupt(void);
void __attribute__((interrupt, auto_psv)) _T1Interrupt(void);
void __attribute__((interrupt, auto_psv)) _T2Interrupt(void);
void __attribute__ ((interrupt, no_auto_psv)) _INT0Interrupt(void);
void __attribute__ ((interrupt, no_auto_psv)) _INT1Interrupt(void);
void __attribute__ ((interrupt, no_auto_psv)) _INT2Interrupt(void);
//****************************************************************
// <editor-fold defaultstate="collapsed" desc="열거형선언">
enum { MODE_SEL, VOLT_ERR}; // 2 모드 화면, 에러 화면
enum { CHK_OK, CHK_NO, CHK_RESULT, CHK_HINT, CHK_HINT_1, CHK_LAN, CHK_VOLT, CHK_BIT}; // 7
enum { NONE, SELF, LINE, FIRE1, FIRE2 };
enum { SELF_TEST, SELF_RES_TEST, SELF_SHO_TEST, SELF_INS_TEST}; // 5
enum { SELF_RES_RESULT, SELF_SHO_RESULT, SELF_INS_RESULT}; // 3
enum { LINE_RES_RESULT, LINE_SHO_RESULT, LINE_INS_RESULT}; // 3
enum { LINE_TEST, LINE_RES_TEST, LINE_SHO_TEST, LINE_INS_TEST}; // 5
enum { FIRE_TEST, FIRE_TEST_EACH_1, FIRE_TEST_TOTAL_1, FIRE_TEST_EACH_2, FIRE_TEST_TOTAL_2 }; // 6
enum { MSL_RESULT, EXT_RESULT, EB_RESULT, BAT_RESULT, ABAT_RESULT, BDU_RESULT, ARMING_RESULT, INTARM_RESULT, INT_RESULT}; //9
enum { CON, MSL, EXT, EB, BAT, ABAT, BDU, ARMING, INTARM, INT}; //9
enum { RESULT,DC_VREF, DC5V, DC12V_PLUS, DC12V_MINUS};//enum { RESULT,DC_VREF, DC3V, DC5V, DC12V_PLUS, DC12V_MINUS};
enum { A0, A1, A2, CLEAR}; // 어드레스 제어
enum { SEL_ADC, SEL_LED, SEL_DATA, SEL_ADDRESS, SEL_RELAY01, SEL_LEDADC};   // CS 선택
enum { RESIDUAL, SHORT, INSULATION};    // 잔류, 도통/단선, 절연
enum { RESIDUAL_RELAY, INSUL_RELAY, OK_LED, INSUL_LED, INTEGRATION_LED, SP_LED, SHORT_LED, RESIDUAL_VOLT_LED};
enum { FIRST, SECOND } ;
enum { EB1_SQUIB1, EB1_SQUIB2, EB2_SQUIB1, EB2_SQUIB2, EB_STATUS };
enum { BAT1_SQUIB1, BAT1_SQUIB2, BAT2_SQUIB1, BAT2_SQUIB2, BAT_STATUS };
enum { ABAT1_SQUIB1, ABAT1_SQUIB2, ABAT2_SQUIB1, ABAT2_SQUIB2, ABAT_STATUS };
enum { BDU_SQUIB1, BDU_SQUIB2, BDU_STATUS };
enum { INT_SQUIB1, INT_SQUIB2, INT_STATUS };
enum { FAIL, GOOD_1 };
enum { NONE_0, OFF_1, ON_2 } ;
enum { CON_ACK, FIRE_ACK };
// </editor-fold>
// 전역 변수 선언************************
unsigned char g_RxBuffer[100];
unsigned char g_RxData;
unsigned char g_Buffer;

unsigned int g_AdcBuffer[5];            // ADC 측정값
unsigned int g_AdcCount=0;              // ADC 카운터값
unsigned long int g_AdcSample[5];       // ADC 측정 샘플값

unsigned int g_RelayChannel = 0;        // 릴레이 채널값
unsigned int g_InputValue=0 ;           // 숫자 키패드값
unsigned int g_TimerCount=0;            // 타이머1 카운터값
unsigned int g_OkButtonTimerCount =0;   // "확인"버튼 눌렀을때 측정되는 값 
unsigned int g_TimerCounter =0;         // 타이머2 카운터값
unsigned short g_DataBuffer[7]={0,};    // 16비트 전송 데이터 저장 배열
unsigned short g_TempBuffer[7]={0,};    // 16비트 수신 데이터 임시 배열 버퍼
unsigned int g_TotalSel = 0 ;           // 발사 계통점검-통합점검에 대한 순서값
//int g_SwSum =0;
unsigned int g_MenuPage;                // 메뉴 화면 페이지 값

const float g_Error[4] = { 0.05f, 0.2f, 0.4f, 1.5f}; //g_Error[3] = { 0.05f,  0.4f, 1.5f};//const float 
// <editor-fold defaultstate="collapsed" desc="구조체 모음">
struct LOADER_STATUS{
	long	terminalSpeed;
	long	downloadSpeed;
} status;
struct ECTSSTATUS EctsStatus;
struct BUTTONVALUE{
	unsigned int s_OkButton;          // 확인
	unsigned int s_BackButton;        // 이전
	unsigned int s_MenuButton;        // 메뉴
	unsigned int s_NextButton;        // >
	unsigned int s_BeforeButton;      // <
}ButtonValue;
struct AD7980VALUE Adc7980Value;
struct SELFTESTTYPE SelfTest;
struct PAGEVALUE{
	unsigned char s_First[2];
	unsigned char s_Check[8];
	unsigned char s_Self[5];
	unsigned char s_SelfResult[3];
	unsigned char s_Etc[6];
	unsigned char s_Line[5];
	unsigned char s_LineResult[3];
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
	unsigned int s_ABATSQUIB;
	unsigned int s_BDUSQUIB;
	unsigned int s_ARMING;
	unsigned int s_INTARM;
	unsigned int s_INTSQUIB;
}FireTestCheck;
struct EBSQUIB
{
	unsigned int s_Status;
	unsigned int s_EB2SQUIB2;
	unsigned int s_EB2SQUIB1;
	unsigned int s_EB1SQUIB2;
	unsigned int s_EB1SQUIB1;

};
struct BATSQUIB
{
	unsigned int s_Status;
	unsigned int s_BAT2SQUIB2;
	unsigned int s_BAT2SQUIB1;
	unsigned int s_BAT1SQUIB2;
	unsigned int s_BAT1SQUIB1;

};
struct ABATSQUIB
{
	unsigned int s_Status;
	unsigned int s_ABAT2SQUIB2;
	unsigned int s_ABAT2SQUIB1;
	unsigned int s_ABAT1SQUIB2;
	unsigned int s_ABAT1SQUIB1;

};
struct BDUSQUIB
{
	unsigned int s_Status;
	unsigned int s_BDUSQUIB2;
	unsigned int s_BDUSQUIB1;
};
struct INTSQUIB
{
	unsigned int s_Status;
	unsigned int s_INTSQUIB2;
	unsigned int s_INTSQUIB1;
};
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
	struct EBSQUIB EB;
	unsigned int s_BATSQUIB;
	struct BATSQUIB BAT;
	unsigned int s_ABATSQUIB;
	struct ABATSQUIB ABAT;
	unsigned int s_BDUSQUIB;
	struct BDUSQUIB BDU;
	unsigned int s_ARMING;
	unsigned int s_INTARM;
	unsigned int s_INTSQUIB;
	struct INTSQUIB INT;
}FireTestCheckResult;

struct FIRESTATUS{
	unsigned char s_MSL[2];
	unsigned char s_EXT[2];
	unsigned char s_EB[2][6];
	unsigned char s_BAT[2][6];
	unsigned char s_ABAT[2][6];
	unsigned char s_BDU[2][6];
	unsigned char s_ARMING[2];
	unsigned char s_INTARM[2];
	unsigned char s_INT[2][6];
}FireStatus;
// </editor-fold>
//***************************************
void __attribute__((interrupt, auto_psv)) _U1RXInterrupt(void)
{

	unsigned char i;
	IFS0bits.U1RXIF = 0;

	g_RxBuffer[g_Buffer] = g_RxData = ReadUART1();
	g_Buffer++;
	ButtonPrintValue();

	if (U1STAbits.OERR == 1) // 수신 패킷이 꽉차면 초기화 하는 부분
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
	IFS0bits.T1IF = 0; // 인터럽트 타이머 플래그 초기화
	g_TimerCount++;
	if(g_TimerCount==2) {
		RelayControl(); // 200ms 마다 동작
	}
	if(g_TimerCount==6) g_TimerCount=0; // 600ms 지나면 카운터 초기화
}
void __attribute__((interrupt, auto_psv)) _T2Interrupt(void)
{
	IFS0bits.T2IF = 0;  //  인터럽트 타이머 플래그 초기화

	g_TimerCounter++;

	if(PORTEbits.RE3 == 0)
	{
		g_OkButtonTimerCount = g_TimerCounter ;     // 눌렀을때
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
	//        	printf("f 누름 : %d,140,100\r",g_OkButtonTimerCount);
	//            printf("R 140,200,200,200,255,255,255,1\r");
	//            printf("f 안누름 : %d,140,200\r",g_OkButtonTimerCountPre);


}
void __attribute__ ((interrupt, no_auto_psv)) _INT0Interrupt(void)
{
//loopback_udp(3000,0); // 메시지 전송 및 수신 동작이라 가정하고
	if(PageValue.s_Fire[FIRE_TEST_TOTAL_1]==1)
	{
		if(g_TotalSel == 10) g_TotalSel = 0;
		g_TotalSel+=1;// 수신 완료되었을경우 값을 1씩 증가 시킨다.
	}
	else if(PageValue.s_Fire[FIRE_TEST_TOTAL_2]==1)
	{
		if(g_TotalSel == 10) g_TotalSel = 0;
		g_TotalSel+=1;// 수신 완료되었을경우 값을 1씩 증가 시킨다.
	}

	IFS0bits.INT0IF = 0;		// INT0 Interrupt Flag Clear;
}
void __attribute__ ((interrupt, no_auto_psv)) _INT1Interrupt(void)
{
	if(KEYPAD_1)
    {
        g_InputValue = 1;    
    }
    if(KEYPAD_2) g_InputValue = 2;    
    if(KEYPAD_3) g_InputValue = 3;    
    if(KEYPAD_MENU) // 모드선택 화면으로 초기화
    if(KEYPAD_4) g_InputValue = 4;    
    if(KEYPAD_5) g_InputValue = 5;    
    if(KEYPAD_6) g_InputValue = 6;    
    if(KEYPAD_BACK) // 이전 버튼 선택
    {
        ButtonValue.s_BackButton = (KEYPAD_BACK) ? 1 : 0;
		g_InputValue = 0;
		printf("f int플래그 %d,999,999\r", ButtonValue.s_BackButton);
		printf("f int포트 %d,999,999\r", PORTAbits.RA3);
    }
    if(KEYPAD_7) g_InputValue = 7;    

   	ButtonValue.s_NextButton == 0 ? printf("R 80,240,104,264,255,255,255,1\r"):NULL;    // 세부 점검 결과 화면에서 숫자 버튼 못쓰도록
	if((g_InputValue != 0) && (ButtonValue.s_NextButton == 0))
	{
        int num;
     //   num  = strcspn(g_InputValue,"0123456789");
        printf("i STRING/%d.jpg,80,240\r",num);
		//printf("f %d,80,240\r",g_InputValue);   // 키패드 입력하는부분
	}

	IFS1bits.INT1IF = 0;		// INT1 Interrupt Flag Clear;
	IFS1bits.INT2IF = 0;		// INT2 Interrupt Flag Clear
    
    
    
//	if(PORTEbits.RE0 == 0) g_InputValue += 1;
//	if(PORTEbits.RE1 == 0) g_InputValue = 2;
//	if(PORTEbits.RE2 == 0) g_InputValue = 3;
//	if((PORTEbits.RE3 == 0) && (g_InputValue >0))
//	{
//		ButtonValue.s_OkButton = 1;
//		//g_SwSum = g_InputValue;
//	}
//	ButtonValue.s_NextButton == 0 ? printf("R 80,240,104,264,255,255,255,1\r"):NULL;    // 세부 점검 결과 화면에서 숫자 버튼 못쓰도록
//	if((g_InputValue != 0) && (ButtonValue.s_NextButton == 0))
//	{
//		printf("f %d,80,240\r",g_InputValue);   // 키패드 입력하는부분
//	}
//
//	IFS1bits.INT1IF = 0;		// INT1 Interrupt Flag Clear;
//	IFS1bits.INT2IF = 0;		// INT2 Interrupt Flag Clear
}
void __attribute__ ((interrupt, no_auto_psv)) _INT2Interrupt(void)
{
if(KEYPAD_8) g_InputValue = 8;    
    if(KEYPAD_9) g_InputValue = 9;    
    if(KEYPAD_S_BACK) 
    {
        
    }
    if(KEYPAD_0) g_InputValue = 0;    
    if(KEYPAD_OK) 
    {
        ButtonValue.s_OkButton = 1;
    }
    if(KEYPAD_S_NEXT) 
    {
        	ButtonValue.s_NextButton = 1;
    }

    if((g_InputValue != 0) && (ButtonValue.s_NextButton == 0))
	{
        int num;
       // num  = strcspn(g_InputValue,"0123456789");
        printf("i STRING/%d.jpg,80,240\r",num);
		//printf("f %d,80,240\r",g_InputValue);   // 키패드 입력하는부분
	}
    IFS1bits.INT1IF = 0;		// INT1 Interrupt Flag Clear;
    IFS1bits.INT2IF = 0;		// INT2 Interrupt Flag Clear
//	if(PORTEbits.RE4 == 0)
//	{
//		ButtonValue.s_BackButton = (PORTEbits.RE4 == 0) ? 1 : 0;
//		g_InputValue = 0;
//		//printf("R 200,200,200,264,255,255,255,1\r");
//		printf("f int플래그 %d,999,999\r", ButtonValue.s_BackButton);
//		printf("f int포트 %d,999,999\r", PORTEbits.RE4);
//	}
//	if((PORTEbits.RE5 == 0) && (g_InputValue >0))
//	{
//		ButtonValue.s_NextButton = 1;
//		//g_SwSum = g_InputValue;
//	}
//	IFS1bits.INT1IF = 0;		// INT1 Interrupt Flag Clear;
//	IFS1bits.INT2IF = 0;		// INT2 Interrupt Flag Clear
}

void ButtonPrintValue(void)      // 버튼입력값 화면 출력 SCI 연동
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
		ButtonValue.s_OkButton = g_RxData == 0xFF ? 1:0 ;   // 확인 버튼 모의
		ButtonValue.s_BackButton = g_RxData == 0xBB ? 1:0 ; // 이전 버튼 모의
		ButtonValue.s_NextButton = g_RxData == 0xDD ? 1:0 ; // > 버튼 모의
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
	ButtonValue.s_OkButton = g_RxData == 0xFF ? 1:0 ;   // 확인 버튼 모의
	ButtonValue.s_BackButton = g_RxData == 0xBB ? 1:0 ; // 이전 버튼 모의
	ButtonValue.s_NextButton = g_RxData == 0xDD ? 1:0 ; // > 버튼 모의
	}
	}
	else
	{
	ButtonValue.s_OkButton = g_RxData == 0xFF ? 1:0 ;   // 확인 버튼 모의
	ButtonValue.s_BackButton = g_RxData == 0xBB ? 1:0 ; // 이전 버튼 모의
	ButtonValue.s_NextButton = g_RxData == 0xDD ? 1:0 ; // > 버튼 모의
	}
	}
	*/// 06일 변경 소스
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
	else if(PageValue.s_Fire[FIRE_TEST_EACH_1]==1 || //발사계통점검 메뉴 선택시
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

	}*/// 기존 소스
}
void InnerVoltTest(void)         // 내부 전원 점검 (완료 vref. 5v, +15v, -15v 정상값 확인 완료)
{
	unsigned char n;
	unsigned int *pADC_buf16;
	printf("R 0,0,480,272,255,255,255,1\r"); // 화면 초기화
	printf("i TESTING/InnerVoltTesting.jpg,0,0\r");  // 내부 전압 점검중 화면 출력
	// 내부 전압 점검
	while(1)
	{
		if(AD1CON1bits.DONE == 1) // adc 변환이 끝났다.
		{ // A/D CONTROL ????
			AD1CON1bits.DONE = 0;    // adc 변환이 끝나지 않았다.
		}
		else // adc 변환이 안끝났을경우
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
				//					SelfTest.s_Vref = (((float)g_AdcSample[DC_VREF])*VREF)/BIT10; // Vref 값 샘플링
				//					SelfTest.s_Dc3v = (((float)g_AdcSample[DC3V])*VREF)/BIT10*GAIN_p3V; // Dc3v 값 샘플링
				//					SelfTest.s_Dc5v = (((float)g_AdcSample[DC5V])*VREF)/BIT10*GAIN_p5V; // Dc5v 값 샘플링
				//					SelfTest.s_Dc12vPlus = (((float)g_AdcSample[DC12V_PLUS])*VREF)/BIT10*GAIN_p12V; // Dc12vPlus 값 샘플링
				//					SelfTest.s_Dc12vMinus = (-1)*((((float)g_AdcSample[DC12V_MINUS])*VREF)/BIT10*GAIN_m12V); // Dc12vMinus 값 샘플링
				SelfTest.s_Vref  = (((float)g_AdcSample[DC_VREF])*VREF)/BIT10;  //1
				SelfTest.s_Dc5v  = (((float)g_AdcSample[DC5V])*VREF)/BIT10*GAIN_p5V;    //3
				SelfTest.s_Dc12vPlus= (((float)g_AdcSample[DC12V_PLUS])*VREF)/BIT10*GAIN_p12V;  //4
				SelfTest.s_Dc12vMinus = (-1)*((((float)g_AdcSample[DC12V_MINUS])*VREF)/BIT10*GAIN_m12V);//5
				break;
			}
			g_AdcCount++;
		}
	}
    
			if((SelfTest.s_Vref >= (4.096f-g_Error[0]))&&(SelfTest.s_Vref <= (4.096f+g_Error[0])))
			{     // ((Vref 샘플값 >= (4.096f-에러율)) && (Vref 샘플값 <= (4.096f+에러율))
				SET_BIT(SelfTest.s_Result,0);   // 정상이면 결과값의 0비트 1로 변경
			}
			else CLEAR_BIT(SelfTest.s_Result,0);    // 비정상이면 결과값의 0비트를 0으로 변경
	
			if((SelfTest.s_Dc3v >= (3.0f-g_Error[1]))&&(SelfTest.s_Dc3v <= (3.0f+g_Error[1])))
			{
				SET_BIT(SelfTest.s_Result,1);// 정상이면 결과값의 1비트 1로 변경
			}
	            else CLEAR_BIT(SelfTest.s_Result,1);// 비정상이면 결과값의 1비트를 0으로 변경
	
			if((SelfTest.s_Dc5v >= (5.0f-g_Error[2]))&&(SelfTest.s_Dc5v <= (5.0f+g_Error[2])))
			{
				SET_BIT(SelfTest.s_Result,2);// 정상이면 결과값의 2비트 1로 변경
			}
			else CLEAR_BIT(SelfTest.s_Result,2);// 비정상이면 결과값의 2비트를 0으로 변경
	
			if((SelfTest.s_Dc12vPlus >= (15.0f-g_Error[3]))&&(SelfTest.s_Dc12vPlus <= (15.f+g_Error[3])))
			{
				SET_BIT(SelfTest.s_Result,3);// 정상이면 결과값의 3비트 1로 변경
			}
			else CLEAR_BIT(SelfTest.s_Result,3);// 비정상이면 결과값의 3비트를 0으로 변경
	
			if((SelfTest.s_Dc12vMinus >= (-15.0f-g_Error[3]))&&(SelfTest.s_Dc12vMinus <= (-15.0f+g_Error[3])))
			{
				SET_BIT(SelfTest.s_Result,4);// 정상이면 결과값의 4비트 1로 변경
			}
			else CLEAR_BIT(SelfTest.s_Result,4);// 비정상이면 결과값의 4비트를 0으로 변경

	if(SelfTest.s_Result == 0x1F) // 결과값 모두 정상이면 동작
	{
		printf("i MENU/ModeMenu.jpg,0,0\r");
		PageValue.s_First[MODE_SEL] = 1;          // 모드 선택 화면 토글

		printf("i ETC/VoltGood.jpg,0,2\r");
		PageValue.s_Check[CHK_VOLT] = 1;            // 내부 전압 정상 알람 토글
		if(PORTEbits.RE6 == 0)// 자체 점검 플러그 인식 확인(자체점검플러그 미 연결시 =1, 자체점검플러그 연결시 = 0)
		{
			printf("i ETC/BitCheck.jpg,325,2\r");
			PageValue.s_Check[CHK_BIT] = 1;             // 자체 점검 가능 알람 토글

			//통신점검 메시지 날리는 함수 
			SendData(0); // 통신점검 메시지 g_DataBuffer 설정
			// 인터럽트 0번 동작
			// 점검 메시지 전송, 수신 결과 확인
			if(PageValue.s_Check[CHK_LAN] == 1) printf("i ETC/LanConnect.jpg,165,2\r"); // 랜 연결상태가 정상일경우 랜 알람 메시지 출력
		}

	}
	else           // 내부 전압 비정상시
	{
		printf("i ERROR/InnerVoltFail.jpg,0,0\r");  // 내부 전압 비정상
		PageValue.s_First[VOLT_ERR] = 1;              // 내부 전압 비정상 토글
		SelfTest.s_Result = 0; // 결과값 초기화
	}

}
void InitValue(void)             // 변수 초기화
{
	unsigned char n;

	for(n=0; n<100; n++){
		g_RxBuffer[n] = 0;
	}

	g_Buffer = 0;
	g_RxData = 0;

	memset(&PageValue, 0, sizeof(struct PAGEVALUE));    // 화면페이지 변수 초기화
	memset(&ButtonValue, 0, sizeof(struct BUTTONVALUE));    // 버튼선택값 변수 초기화
	memset(&Adc7980Value, 0, sizeof(struct AD7980VALUE));    // ADC값 변수 초기화
	memset(&SelfTest, 0, sizeof(struct SELFTESTTYPE));    // 내부 전원점검 변수 초기화

}
void TestResultCheck(unsigned int Label)    // 자체, 점화, 도통 점검 ●, ▷ 체크 표시
{
	unsigned char n=0;        // 힌트 출력 비교
	switch(Label)
	{
	case 1 : // 잔류, 도통/단선, 절연 점검 체크
		printf("i MENU/SelfTestMenu.jpg,0,0\r"); // 자체 점검 메뉴 출력
		AlramPrint();
		if(PageValue.s_Self[SELF_RES_TEST]==1){ printf("i ETC/Check.jpg,196,69\r"); printf("i ETC/Check_.jpg,222,69\r"); n++;}
		if(PageValue.s_Self[SELF_SHO_TEST]==1){ printf("i ETC/Check.jpg,196,93\r"); printf("i ETC/Check_.jpg,222,93\r"); n++;}
		if(PageValue.s_Self[SELF_INS_TEST]==1){ printf("i ETC/Check.jpg,196,117\r"); printf("i ETC/Check_.jpg,222,117\r"); n++;}
		if(n>0) printf("i ETC/Hint.png,0,205\r");     // 힌트 출력

		break;
	case 2 : // 잔류, 도통/단선, 절연 점검 체크
		printf("i MENU/LineTestMenu.jpg,0,0\r"); // 점화 계통 점검 메뉴 출력
		AlramPrint();
        if(PageValue.s_Line[LINE_RES_TEST]==1){ printf("i ETC/Check.jpg,196,69\r"); printf("i ETC/Check_.jpg,222,69\r"); n++;}
		if(PageValue.s_Line[LINE_SHO_TEST]==1){ printf("i ETC/Check.jpg,196,93\r"); printf("i ETC/Check_.jpg,222,93\r"); n++;}
		if(PageValue.s_Line[LINE_INS_TEST]==1){ printf("i ETC/Check.jpg,196,117\r"); printf("i ETC/Check_.jpg,222,117\r"); n++;}
		if(n>0) printf("i ETC/Hint.png,0,205\r");     // 힌트 출력
		break;
	case 3: // 1호탄 점검 체크

		AlramPrint();
		if(PageValue.s_FireRecvResult_1[MSL_RESULT]==1)   { printf("i ETC/Check.jpg,176,70\r"); printf("i ETC/Check_.jpg,205,70\r"); n++;}
		if(PageValue.s_FireRecvResult_1[EXT_RESULT]==1)   { printf("i ETC/Check.jpg,176,94\r"); printf("i ETC/Check_.jpg,205,94\r"); n++;}
		if(PageValue.s_FireRecvResult_1[EB_RESULT]==1)    { printf("i ETC/Check.jpg,176,118\r"); printf("i ETC/Check_.jpg,205,118\r");n++;}
		if(PageValue.s_FireRecvResult_1[BAT_RESULT]==1)   { printf("i ETC/Check.jpg,176,142\r"); printf("i ETC/Check_.jpg,205,142\r"); n++;}
		if(PageValue.s_FireRecvResult_1[ABAT_RESULT]==1)  { printf("i ETC/Check.jpg,176,166\r"); printf("i ETC/Check_.jpg,205,166\r");n++;}
		if(PageValue.s_FireRecvResult_1[BDU_RESULT]==1)   { printf("i ETC/Check.jpg,176,190\r"); printf("i ETC/Check_.jpg,205,190\r"); n++;}
		if(PageValue.s_FireRecvResult_1[ARMING_RESULT]==1){ printf("i ETC/Check.jpg,176,214\r"); printf("i ETC/Check_.jpg,205,214\r"); n++;}
		if(PageValue.s_FireRecvResult_1[INTARM_RESULT]==1){ printf("i ETC/Check.jpg,411,69\r"); printf("i ETC/Check_.jpg,440,69\r"); n++;}
		if(PageValue.s_FireRecvResult_1[INT_RESULT]==1)   { printf("i ETC/Check.jpg,411,93\r"); printf("i ETC/Check_.jpg,440,93\r");n++;}
		if(n>0) printf("i ETC/Hint_1.png,242,175\r");     // 힌트 출력
		printf("R 80,240,104,264,255,255,255,1\r");
		break;
	
        case 4: // 2호탄 점검 체크
		AlramPrint();
		if(PageValue.s_FireRecvResult_2[MSL_RESULT]==1)   { printf("i ETC/Check.jpg,176,70\r"); printf("i ETC/Check_.jpg,205,70\r"); n++;}
		if(PageValue.s_FireRecvResult_2[EXT_RESULT]==1)   { printf("i ETC/Check.jpg,176,94\r"); printf("i ETC/Check_.jpg,205,94\r"); n++;}
		if(PageValue.s_FireRecvResult_2[EB_RESULT]==1)    { printf("i ETC/Check.jpg,176,118\r"); printf("i ETC/Check_.jpg,205,118\r");n++;}
		if(PageValue.s_FireRecvResult_2[BAT_RESULT]==1)   { printf("i ETC/Check.jpg,176,142\r"); printf("i ETC/Check_.jpg,205,142\r"); n++;}
		if(PageValue.s_FireRecvResult_2[ABAT_RESULT]==1)  { printf("i ETC/Check.jpg,176,166\r"); printf("i ETC/Check_.jpg,205,166\r");n++;}
		if(PageValue.s_FireRecvResult_2[BDU_RESULT]==1)   { printf("i ETC/Check.jpg,176,190\r"); printf("i ETC/Check_.jpg,205,190\r"); n++;}
		if(PageValue.s_FireRecvResult_2[ARMING_RESULT]==1){ printf("i ETC/Check.jpg,176,214\r"); printf("i ETC/Check_.jpg,205,214\r"); n++;}
		if(PageValue.s_FireRecvResult_2[INTARM_RESULT]==1){ printf("i ETC/Check.jpg,411,69\r"); printf("i ETC/Check_.jpg,440,69\r"); n++;}
		if(PageValue.s_FireRecvResult_2[INT_RESULT]==1)    { printf("i ETC/Check.jpg,411,93\r"); printf("i ETC/Check_.jpg,440,93\r");n++;}
		if(n>0) printf("i ETC/Hint_1.png,242,175\r");       // 힌트 출력
		printf("R 80,240,104,264,255,255,255,1\r");
		break;
	}
}
void RelayControl() // 타이머 카운터로 인해 600ms 마다 반복동작함.
{
    unsigned int Count = 0;
	unsigned int Add = 0;
	InitRelay();
	Count = g_RelayChannel%8;
	Add = g_RelayChannel/8;

	PORTD = IOPortSelectParam[Count]; // DO0 ~ DO7 반복
	PORTG = INIT_CS + CS2;  //  DATA 선택
	PORTG = INIT_CS;        //  DATA 초기화

	PORTD = RelaySelectParam[Add];//PORTD = 0x0008;// A3 - H -> 릴레이 1번 제어
	PORTG = INIT_CS + CS3;  //  ADD  선택
	PORTG = INIT_CS;        //  ADD 초기화

	PORTD = RelaySelectParam[CLEAR];//PORTD = 0x000f; //A0 - H, A1 - H, A2 - H, A3 - H  -> 릴레이 동작 해제
	PORTG = INIT_CS + CS3;  // ADD 선택
	PORTG = INIT_CS;        // ADD 초기화
	ReadADCValue(50);
	g_RelayChannel++;
//	unsigned int Count = 0;
//	unsigned int Add = 0;
//	InitRelay();
//	Count = g_RelayChannel%8;
//	Add = g_RelayChannel/8;
//
//	PORTD = IOPortSelectParam[Count]; // DO0 ~ DO7 반복
//	PORTG = ChipSelectParam[SEL_DATA];//PORTG = 0x0044; // CS4, CS3, CS2 DATA 선택
//	PORTG = ChipSelectParam[SEL_ADC];// PORTG = 0x0004; // CS3, CS2     DATA 초기화
//
//	PORTD = RelaySelectParam[Add];//PORTD = 0x0008;// A3 - H -> 릴레이 1번 제어
//	PORTG = ChipSelectParam[SEL_ADDRESS];//PORTG = 0x0006;// CS3, CS2, CS1    ADDRESS 선택
//	PORTG = ChipSelectParam[SEL_ADC];// PORTG = 0x0004; // CS3, CS2         ADDRESS 초기화
//
//	PORTD = RelaySelectParam[CLEAR];//PORTD = 0x000f; //A0 - H, A1 - H, A2 - H, A3 - H  -> 릴레이 동작 해제
//	PORTG = ChipSelectParam[SEL_ADDRESS];//PORTG = 0x0006; // CS3, CS2, CS1    ADDRESS 선택
//	PORTG = ChipSelectParam[SEL_ADC];//PORTG = 0x0004; // CS3, CS2         ADDRESS 초기화
//	ReadADCValue(50);
//	g_RelayChannel++;
}
void InitRelay(void)    // 릴레이 레지스터 초기화
{
	PORTD = 0x0000;                 // DO 0~7번 사용 안할경우 0으로 초기화
	PORTG = INIT_CS;
//    PORTD = 0x0000;                 // DO 0~7번 사용 안할경우 0으로 초기화
//	PORTG = ChipSelectParam[SEL_ADC];
}
void AlramPrint()       // 자체점검가능, 전원정상, LAN연결 상태알람메시지 출력
{
	if(PageValue.s_Check[CHK_VOLT] == 1) printf("i ETC/VoltGood.jpg,0,2\r");        // 내부 전원 정상시 알람 출력
	if(PageValue.s_Check[CHK_BIT] == 1) printf("i ETC/BitCheck.jpg,325,2\r");        // 자체 점검 가능 알람 출력
	if(PageValue.s_Check[CHK_LAN] == 1) printf("i ETC/LanConnect.jpg,165,2\r");        // LAN 연결상태 확인 알람
}
void RelayLedControl(unsigned char Ch, unsigned char On_Off)    // 잔류전압 릴레이, 절연점압 릴레이 제어
{
    unsigned int RelayLedStatus=0;    // ADC 츠
    unsigned char a;
    if(Ch == 0) a = 6;
    else if(Ch == 1) a=7;
	// RESIDUAL_RELAY - 릴레이 0번 사용 , INSUL_RELAY - 릴레이 1번 사용
	if(On_Off == ON)
		SET_BIT(RelayLedStatus, a);    // ON 이면 CH번째 1로 변경
	else
		CLEAR_BIT(RelayLedStatus, a);  // OFF면 CH번째 0으로 변경

	PORTE = RelayLedStatus;             // 0001 OR 0002
	PORTG = INIT_CS;   //  ADC(CS2) + LED(CS0)
	PORTG = CE_CNV;   // ADC(CS2)
//     unsigned int RelayLedStatus=0;    // ADC 츠
//	// RESIDUAL_RELAY - 릴레이 0번 사용 , INSUL_RELAY - 릴레이 1번 사용
//	if(On_Off == ON)
//		SET_BIT(RelayLedStatus, Ch);    // ON 이면 CH번째 1로 변경
//	else
//		CLEAR_BIT(RelayLedStatus, Ch);  // OFF면 CH번째 0으로 변경
//	PORTD = RelayLedStatus;             // 0001 OR 0002
//	PORTG = ChipSelectParam[SEL_LEDADC];   //  ADC(CS2) + LED(CS0)
//	PORTG = ChipSelectParam[SEL_ADC];   // ADC(CS2)
}

// <editor-fold defaultstate="collapsed" desc="잔류전압점검">
void ResidualVoltTest(void)     // 잔류 전압 점검(완료  ADC값 검증 필요)FORI0번 건너 뛰고 비교해야됨
{
	int fori;
	InitRelay();    // 릴레이 초기화

	IEC1bits.INT1IE = 0;	// 인터럽트 1중지
	IEC1bits.INT2IE = 0;    // 인터럽트 2중지
    Relay0 = 1;     //  잔류 전압 점검 릴레이 동작
	Relay1 = 0;     // 절연 점검 릴레이 해제
//	RelayLedControl(RESIDUAL_RELAY,ON); // ON = 1 릴레이 0번
//	RelayLedControl(INSUL_RELAY,OFF);   // OFF = 0 릴레이 1번
	// 릴레이로 채널선택후 ADC로 값을 읽고 채널별 값을 저장해서 정상 범위값과 비교
	memset(&EctsStatus.ResidualVoltTestResult[0],0,sizeof(EctsStatus.ResidualVoltTestResult[0])*20); // 20개 채널 0으로 초기화
	EctsStatus.ResidualVoltTestTotalResult = 0;     // 잔류 전압 점검 전체 결과 초기화
	T1CONbits.TON = 1;  //타이머 인터럽트 동작
	while(g_RelayChannel<20)
	{
		// g_RelayChannel 가 19번 될때까지 반복 하면서
		// 타이머 인터럽트 600ms 마다 g_RelayChannel 값을 증가 시킴
	}
	if(g_RelayChannel == 20 )
	{
		T1CONbits.TON = 0;                          // 타이머 인터럽트 미동작
		for(fori=1;fori<g_RelayChannel+1;fori++)
		{
			EctsStatus.ResidualVoltTestResult[fori].ResidualVoltTestValue = Adc7980Value.s_Value[fori];  // 채널별로 ADC로 읽은 값을 저장
			EctsStatus.ResidualVoltTestResult[fori].ResidualVoltTestValue = fabs(SelfTest.s_Vref - EctsStatus.ResidualVoltTestResult[fori].ResidualVoltTestValue);
			// 내부 전원점검의 기준전압 - ADC 측정된 채널값의 절대값을 = ADC의 측정된 값에 다시 저장
			// 비정상을 판단 하는 기준을 구하는 공식 : Vref - ( Vref - xV))
			// 차동증폭기는 Vin - Vout 값의 비례로 증폭되며 이 값에 따라 극성이 선택된다. 그래서 기준전압과 측정된 값을 뺀 값을 채널값에 저장
			// 그리고 그값과 비정상으로 판단하는 기준값과 비교하여 정상 / 비정상 판단.
			// 잔류 전압은 케이블 내부에 남아있는 전압값을 측정하는 검사 즉 정상 기준값보다 낮으면 정상/ 높으면 비정상
			if(EctsStatus.ResidualVoltTestResult[fori].ResidualVoltTestValue < RESIDUALREFERENCE)
			{
				EctsStatus.ResidualVoltTestResult[fori].ResidualVoltTestStatus = 0;    // 잔류 전압 점검 결과 정상 0.1v 이하일경우 정상
			}
			else
			{
				EctsStatus.ResidualVoltTestResult[fori].ResidualVoltTestStatus = 1;    // 잔류 전압 점검 결과 비정상 0.1v 이상일경우 정상
			}

			if(EctsStatus.ResidualVoltTestResult[fori].ResidualVoltTestStatus == 1)    // 잔류 전압 점검 결과 비정상일경우 통합 결과 증가
			{
				EctsStatus.ResidualVoltTestTotalResult++;
			}
		}
		g_RelayChannel = 0;
		IFS1bits.INT1IF = 0;
		IFS1bits.INT2IF = 0;
		IEC1bits.INT1IE = 1;        // 인터럽트1 동작
		IEC1bits.INT2IE = 1;        // 인터럽트2 동작
	}
}
void ResidualVoltTestResult(void)
{
	printf("i Result/ResidualVoltResult.jpg,0,0\r");
	AlramPrint();

	int ChannelCount;
	int x=172;
	int y=71;
	int z=432;
	for(ChannelCount=1; ChannelCount < 9 ; ChannelCount++)
	{
		if(EctsStatus.ResidualVoltTestResult[ChannelCount].ResidualVoltTestStatus == 0) printf("i ETC/Check_OK.jpg,%d,%d\r",x,y);
		else  printf("i ETC/Check_FAIL.jpg,%d,%d\r",x,y);
		y= y + 24;
	}
	y= 71;
	for(ChannelCount=9; ChannelCount < 17 ; ChannelCount++)
	{
		if(EctsStatus.ResidualVoltTestResult[ChannelCount].ResidualVoltTestStatus == 0) printf("i ETC/Check_OK.jpg,%d,%d\r",z,y);
		else printf("i ETC/Check_FAIL.jpg,%d,%d\r",z,y);
		y= y + 24;
	}

}
void ResidualVoltTestResultDebug(void)
{
	printf("i Result_D/ResidualVoltResult_D.jpg,0,0\r");
	AlramPrint();
	int ChannelCount;
    char buf[10];
    int num[10];
   
	int x[5]= {140, 155, 170, 190, 205 };
	int y = 72;
	int z[5]= {400, 415, 430, 450, 465 };
	for(ChannelCount=0; ChannelCount < 8 ; ChannelCount++)
	{
        sprintf(buf, "%0.2f",EctsStatus.ResidualVoltTestResult[ChannelCount].ResidualVoltTestValue);    // 실수 문자열 배열에 저장
        num[0] = strcspn(buf[0],"0123456789");
        printf("i STRING/%d.jpg,%d,%d\r",num[0],x[0],y);
        if(buf[1]=='.') printf("i STRING/POT.jpg,%d,%d\r",x[1],y);
        num[1] = strcspn(buf[2],"0123456789");
        printf("i STRING/%d.jpg,%d,%d\r",num[1],x[2],y);
        num[2] = strcspn(buf[3],"0123456789");
        printf("i STRING/%d.jpg,%d,%d\r",num[2],x[3],y);
        printf("i STRING/V.jpg,%d,%d\r",x[4],y);
        y=y+24;
	}
	y= 72;
	for(ChannelCount=8; ChannelCount < 16 ; ChannelCount++)
	{
		sprintf(buf, "%0.2f",EctsStatus.ResidualVoltTestResult[ChannelCount].ResidualVoltTestValue);    // 실수 문자열 배열에 저장
        num[3] = strcspn(buf[0],"0123456789");
        printf("i STRING/%d.jpg,%d,%d\r",num[0],z[0],y);
        if(buf[1]=='.') printf("i STRING/POT.jpg,%d,%d\r",x[1],y);
        num[4] = strcspn(buf[2],"0123456789");
        printf("i STRING/%d.jpg,%d,%d\r",num[1],z[2],y);
        num[5] = strcspn(buf[3],"0123456789");
        printf("i STRING/%d.jpg,%d,%d\r",num[2],z[3],y);
        printf("i STRING/V.jpg,%d,%d\r",z[4],y);
        y=y+24;
	}
}
// </editor-fold>

// <editor-fold defaultstate="collapsed" desc="절연 점검">
void InsulationTest(void)       // 절연 점검(완료 ADC값 검증 필요)FORI0번 건너 뛰고 비교해야됨
{
	int fori;
	InitRelay();    // 릴레이 초기화
	IEC1bits.INT1IE = 0;
	IEC1bits.INT2IE = 0;
	
    Relay0 = 0;     //  잔류 전압 점검 릴레이 해제
	Relay1 = 1;     // 절연 점검 릴레이 동작
//    (RESIDUAL_RELAY,OFF); // OFF = 0 릴레이 0번
//	RelayLedControl(INSUL_RELAY,ON);   // ON = 1 릴레이 1번

	memset(&EctsStatus.InsulTestResult[0],0,sizeof(EctsStatus.InsulTestResult[0])*20); // 20개 채널 0으로 초기화
	EctsStatus.InsulTestTotalResult = 0;    // 절연 점검 전체 결과 초기화
	T1CONbits.TON = 1;  //타이머 인터럽트 동작
	while(g_RelayChannel<20)
	{

	}
	if(g_RelayChannel == 20 )
	{
		T1CONbits.TON = 0;                          // 타이머 인터럽트 미동작
		for(fori=1;fori<g_RelayChannel+1;fori++)
		{
			// <editor-fold defaultstate="collapsed" desc="절연 점검 ADC 정상 비정상 판단">
			EctsStatus.InsulTestResult[fori].InsulTestValue = Adc7980Value.s_Value[fori];  // 채널별로 ADC로 읽은 값을 저장
			EctsStatus.InsulTestResult[fori].InsulTestValue = fabs(SelfTest.s_Vref - EctsStatus.InsulTestResult[fori].InsulTestValue);

			if(EctsStatus.InsulTestResult[fori].InsulTestValue > INSULESTREFERENCE)
			{
				EctsStatus.InsulTestResult[fori].InsulTestStatus = 1;    // 절연 점검 결과 정상
			}
			else
			{
				EctsStatus.InsulTestResult[fori].InsulTestStatus = 0;    // 절연 점검 결과 비정상
			}

			if(EctsStatus.InsulTestResult[fori].InsulTestStatus == 1)    // 절연 점검 결과 비정상일경우 통합 결과 증가
			{
				EctsStatus.InsulTestTotalResult++;
			}
			// </editor-fold>
		}
		g_RelayChannel = 0;
		IFS1bits.INT1IF = 0;
		IFS1bits.INT2IF = 0;
		IEC1bits.INT1IE = 1;        // 인터럽트1 동작
		IEC1bits.INT2IE = 1;        // 인터럽트2 동작
	}
}
void InsulationTestResult(void)
{
	printf("i Result/InsulResult.jpg,0,0\r");
	AlramPrint();

	int ChannelCount;
	int x=172;
	int y=71;
	int z=432;
	for(ChannelCount=1; ChannelCount < 9 ; ChannelCount++)
	{
		if(EctsStatus.InsulTestResult[ChannelCount].InsulTestStatus == 0) printf("i ETC/Check_OK.jpg,%d,%d\r",x,y);
		else printf("i ETC/Check_FAIL.jpg,%d,%d\r",x,y);
		y= y + 24;
	}
	y= 71;
	for(ChannelCount=9; ChannelCount < 17 ; ChannelCount++)
	{
		if(EctsStatus.InsulTestResult[ChannelCount].InsulTestStatus == 0) printf("i ETC/Check_OK.jpg,%d,%d\r",z,y);
		else printf("i ETC/Check_FAIL.jpg,%d,%d\r",z,y);
		y= y + 24;
	}

}
void InsulationTestResultDebug(void)
{
	printf("i Result_D/InsulResult_D.jpg,0,0\r");
	AlramPrint();
int ChannelCount;
   
     char buf[10];
    int num[10];
   
	int x[5]= {140, 155, 170, 190, 205 };
	int y = 72;
	int z[5]= {400, 415, 430, 450, 465 };
	for(ChannelCount=0; ChannelCount < 8 ; ChannelCount++)
	{
        sprintf(buf, "%0.2f",EctsStatus.InsulTestResult[ChannelCount].InsulTestValue);    // 실수 문자열 배열에 저장
        num[0] = strcspn(buf[0],"0123456789");
        printf("i STRING/%d.jpg,%d,%d\r",num[0],x[0],y);
        if(buf[1]=='.') printf("i STRING/POT.jpg,%d,%d\r",x[1],y);
        num[1] = strcspn(buf[2],"0123456789");
        printf("i STRING/%d.jpg,%d,%d\r",num[1],x[2],y);
        num[2] = strcspn(buf[3],"0123456789");
        printf("i STRING/%d.jpg,%d,%d\r",num[2],x[3],y);
        printf("i STRING/V.jpg,%d,%d\r",x[4],y);
        y=y+24;
	}
	y= 72;
	for(ChannelCount=8; ChannelCount < 16 ; ChannelCount++)
	{
		sprintf(buf, "%0.2f",EctsStatus.InsulTestResult[ChannelCount].InsulTestValue);    // 실수 문자열 배열에 저장
        num[3] = strcspn(buf[0],"0123456789");
        printf("i STRING/%d.jpg,%d,%d\r",num[0],z[0],y);
        if(buf[1]=='.') printf("i STRING/POT.jpg,%d,%d\r",x[1],y);
        num[4] = strcspn(buf[2],"0123456789");
        printf("i STRING/%d.jpg,%d,%d\r",num[1],z[2],y);
        num[5] = strcspn(buf[3],"0123456789");
        printf("i STRING/%d.jpg,%d,%d\r",num[2],z[3],y);
        printf("i STRING/V.jpg,%d,%d\r",z[4],y);
        y=y+24;
	}
    
	
}
// </editor-fold>

// <editor-fold defaultstate="collapsed" desc="도통 단선 점검">
void ShortTest(void)            // 도통단선점검(완료 ADC값 검증 필요)FORI0번 건너 뛰고 비교해야됨
{
	int fori;
	InitRelay();    // 릴레이 초기화
	IEC1bits.INT1IE = 0;
	IEC1bits.INT2IE = 0;
	Relay0 = 0;     //  잔류 전압 점검 릴레이 해제
	Relay1 = 0;     // 절연 점검 릴레이 해제
    
//    (RESIDUAL_RELAY,OFF); // OFF = 0 릴레이 0번
//	RelayLedControl(INSUL_RELAY,OFF);   // OFF = 0 릴레이 1번
	memset(&EctsStatus.ShortTestResult[0],0,sizeof(EctsStatus.ShortTestResult[0])*20); // 20개 채널 0으로 초기화
	EctsStatus.ShortTestTotalResult = 0 ;       // 도통/단선 점검 전체 결과 초기화
	T1CONbits.TON = 1;  //타이머 인터럽트 동작
	while(g_RelayChannel<20)
	{

	}
	if(g_RelayChannel == 20 )
	{
		T1CONbits.TON = 0;                          // 타이머 인터럽트 미동작
		for(fori=1;fori<g_RelayChannel+1;fori++)
		{
			// <editor-fold defaultstate="collapsed" desc="도통/단선 ADC 정상 비정상 판단">
			EctsStatus.ShortTestResult[fori].ShortTestValue = Adc7980Value.s_Value[fori];  // 채널별로 ADC로 읽은 값을 저장

			if(EctsStatus.ShortTestResult[fori].ShortTestValue < SHORTESTREFERENCE)
			{
				EctsStatus.ShortTestResult[fori].ShortTestStatus = 0;    // 도통/단선 점검 결과 정상 0.38v 이하면 정상
			}
			else
			{
				EctsStatus.ShortTestResult[fori].ShortTestStatus = 1;    // 도통/단선 점검 결과 비정상 0.38v 이상이면 정상
			}

			if(EctsStatus.ShortTestResult[fori].ShortTestStatus == 1)    // 도통/단선 점검 결과 비정상일경우 통합 결과 증가
			{
				EctsStatus.ShortTestTotalResult++;
			}
			// </editor-fold>
		}

		g_RelayChannel = 0;
		IFS1bits.INT1IF = 0;
		IFS1bits.INT2IF = 0;
		IEC1bits.INT1IE = 1;        // 인터럽트1 동작
		IEC1bits.INT2IE = 1;        // 인터럽트2 동작
	}
}
void ShortTestResult(void)
{
	printf("i Result/ShortResult.jpg,0,0\r");
	AlramPrint();
	int ChannelCount;
	int x=136;
	int y=72;
	int z=400;
	for(ChannelCount=1; ChannelCount < 9 ; ChannelCount++)
	{
		if(EctsStatus.ShortTestResult[ChannelCount].ShortTestStatus == 0) printf("i ETC/Check_OK.jpg,%d,%d\r",x,y);
		else printf("i ETC/Check_FAIL.jpg,%d,%d\r",x,y);
		y= y + 24;
	}
	y= 71;
	for(ChannelCount=9; ChannelCount < 17 ; ChannelCount++)
	{
		if(EctsStatus.ShortTestResult[ChannelCount].ShortTestStatus == 0) printf("i ETC/Check_OK.jpg,%d,%d\r",z,y);
		else  printf("i ETC/Check_FAIL.jpg,%d,%d\r",z,y);
		y= y + 24;
	}
}
void ShortTestResultDebug(void)
{
	printf("i Result_D/ShortResult_D.jpg,0,0\r");
	AlramPrint();
   
    char buf[10];
    int num[10];
	int ChannelCount;
    int x[5]= {140, 155, 170, 190, 205 };
	int y = 72;
	int z[5]= {400, 415, 430, 450, 465 };
	for(ChannelCount=0; ChannelCount < 8 ; ChannelCount++)
	{
        sprintf(buf, "%0.2f",EctsStatus.ShortTestResult[ChannelCount].ShortTestValue);    // 실수 문자열 배열에 저장
        num[0] = strcspn(buf[0],"0123456789");
        printf("i STRING/%d.jpg,%d,%d\r",num[0],x[0],y);
        if(buf[1]=='.') printf("i STRING/POT.jpg,%d,%d\r",x[1],y);
        num[1] = strcspn(buf[2],"0123456789");
        printf("i STRING/%d.jpg,%d,%d\r",num[1],x[2],y);
        num[2] = strcspn(buf[3],"0123456789");
        printf("i STRING/%d.jpg,%d,%d\r",num[2],x[3],y);
        printf("i STRING/V.jpg,%d,%d\r",x[4],y);
        y=y+24;
	}
	y= 72;
	for(ChannelCount=8; ChannelCount < 16 ; ChannelCount++)
	{
		sprintf(buf, "%0.2f",EctsStatus.ShortTestResult[ChannelCount].ShortTestValue);    // 실수 문자열 배열에 저장
        num[3] = strcspn(buf[0],"0123456789");
        printf("i STRING/%d.jpg,%d,%d\r",num[0],z[0],y);
        if(buf[1]=='.') printf("i STRING/POT.jpg,%d,%d\r",x[1],y);
        num[4] = strcspn(buf[2],"0123456789");
        printf("i STRING/%d.jpg,%d,%d\r",num[1],z[2],y);
        num[5] = strcspn(buf[3],"0123456789");
        printf("i STRING/%d.jpg,%d,%d\r",num[2],z[3],y);
        printf("i STRING/V.jpg,%d,%d\r",z[4],y);
        y=y+24;
	}
}
// </editor-fold>

unsigned int ReadADC7980(void)      // ADC7980 SPI통신이용 측정값 반환
{
	unsigned int data;
	data = 0;

	PORTG = CS0 + CS1; // RG9 제어 및 LED 동작 RG0,1
	SPI2BUF = 0x00;
	while(SPI2STATbits.SPITBF);
	while(!SPI2STATbits.SPIRBF);
	data = SPI2BUF;
	PORTG = INIT_CS; // RG9 제어 및 LED 동작 RG0,1

	return data;
//    unsigned int data;
//	data = 0;
//
//	PORTG = ChipSelectParam[SEL_ADC]; // RG9 제어 및 LED 동작 RG0,1
//	SPI1BUF = 0x00;
//	while(SPI1STATbits.SPITBF);
//	while(!SPI1STATbits.SPIRBF);
//	data = SPI1BUF;
//	PORTG = ChipSelectParam[SEL_LED]; // RG9 제어 및 LED 동작 RG0,1
//
//	return data;
}
void ReadADCValue(unsigned int AverageCount)    // ADC 측정값 구하기
{
	unsigned char n;
	Adc7980Value.s_Sum[g_RelayChannel] = 0;
	for(n=0; n<AverageCount; n++){
		Adc7980Value.s_Sum[g_RelayChannel] += ReadADC7980();
	}                                                           // 왜 샘플링값을 50개로 정하는지?, 의미가 있는가 - > 정확도 증가
	Adc7980Value.s_Data[g_RelayChannel] = Adc7980Value.s_Sum[g_RelayChannel] / AverageCount;    // ADC의 샘플링값 50번더해서 평균값 산출
	Adc7980Value.s_Value[g_RelayChannel] = (((float)Adc7980Value.s_Data[g_RelayChannel])*VREF)/BIT16;   // 샘플링값을 기준전압과 곱해서 16비트 분해능으로 나눔
}
void MenuDisplay(void)
{
	if(PageValue.s_First[MODE_SEL] == 1) // 점검 선택                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    5
	{
		switch(g_InputValue)
		{
			// <editor-fold defaultstate="collapsed" desc="모드 메뉴 선택">
		case  0x01 :
			if(ButtonValue.s_OkButton == 1)
			{
				TestResultCheck(1); // 자체 점검 메뉴 출력
				PageValue.s_First[MODE_SEL] = 0;
				PageValue.s_Self[SELF_TEST] = 1;
				// 화면 전환 하면서 메뉴 버튼 플래그 초기화
				ButtonValue.s_OkButton = 0;       // 확인 버튼 초기화
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
				ButtonValue.s_OkButton = 0;       // 확인 버튼 초기화
				ButtonValue.s_BackButton = 0;
				ButtonValue.s_NextButton = 0;
				g_MenuPage = 1;
			}
			break;
		case  0x03 :
			if(ButtonValue.s_OkButton == 1)
			{
				printf("i MENU/FireTestMenu.jpg,0,0\r");
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
		// <editor-fold defaultstate="collapsed" desc="자체 점검 선택">
		if(ButtonValue.s_OkButton == 1)
		{

			switch(g_InputValue)
			{
				// <editor-fold defaultstate="collapsed" desc="점검">
			case 1:
				printf("i TESTING/ResidualTesting.jpg,0,0\r");
				AlramPrint();
				ResidualVoltTest();
				PageValue.s_Self[SELF_RES_TEST] = 1;  // 자체 점검 완료 플래그

				printf("i MENU/SelfTestMenu.jpg,0,0\r"); // 점화계통 점검 메뉴 출력
				AlramPrint();
				TestResultCheck(1);                  // 점검 결과 표시
				// 점검 완료시 버튼 플래그 초기화
				ButtonValue.s_OkButton = 0; // 점검이 완료 되어서
				g_MenuPage =1;// g_MenuPage == 0 ? 1:0;   //1
				g_InputValue = 0;
				break;
			case 2:
				printf("i TESTING/ShortTesting.jpg,0,0\r");
				AlramPrint();
				ShortTest();
				PageValue.s_Self[SELF_SHO_TEST] = 1;  // 도통/단선 점검
				printf("i MENU/SelfTestMenu.jpg,0,0\r"); // 점화계통 점검 메뉴 출력
				AlramPrint();
				TestResultCheck(1);                  // 점검 결과 표시
				ButtonValue.s_OkButton = 0;       // 확인 버튼 초기화
				ButtonValue.s_BackButton = 0;
				ButtonValue.s_NextButton = 0;
				g_MenuPage =1;// g_MenuPage == 0 ? 1:0;   //1
				g_InputValue = 0;
				break;
			case 3:
				printf("i TESTING/InsulTesting.jpg,0,0\r");
				AlramPrint();
				InsulationTest();
				PageValue.s_Self[SELF_INS_TEST] = 1;  // 절연 점검
				printf("i MENU/SelfTestMenu.jpg,0,0\r"); // 점화계통 점검 메뉴 출력
				AlramPrint();
				TestResultCheck(1);                  // 점검 결과 표시
				ButtonValue.s_OkButton = 0;       // 확인 버튼 초기화
				ButtonValue.s_BackButton = 0;
				ButtonValue.s_NextButton = 0;
				g_MenuPage =1;// g_MenuPage == 0 ? 1:0;   //1
				g_InputValue = 0;
				break;
			case 4:

				printf("i TESTING/ResidualTesting.jpg,0,0\r");
				AlramPrint();
				ResidualVoltTest();
				PageValue.s_Self[SELF_RES_TEST] = 1;  // 자체 점검

				printf("i TESTING/ShortTesting.jpg,0,0\r");
				AlramPrint();
				ShortTest();
				PageValue.s_Self[SELF_SHO_TEST] = 1;  // 도통/단선 점검

				printf("i TESTING/InsulTesting.jpg,0,0\r");
				AlramPrint();
				InsulationTest();
				PageValue.s_Self[SELF_INS_TEST] = 1;  // 절연 점검
				TestResultCheck(1);
				ButtonValue.s_OkButton = 0; // 점검이 완료 되어서
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
				// <editor-fold defaultstate="collapsed" desc="세부점검결과">
			case 1:
				IEC1bits.INT1IE = 0;	// 인터럽트 1중지
				ResidualVoltTestResult();
				PageValue.s_SelfResult[SELF_RES_RESULT] = 1;       // 자체 점검 - 잔류 전압 결과 확인
				g_MenuPage = 0;//g_MenuPage == 0 ? 1:0;   //0
				g_InputValue = 0;
				ButtonValue.s_NextButton = 0;
				T2CONbits.TON = 1;
				break;
			case 2:
				IEC1bits.INT1IE = 0;	// 인터럽트 1중지
				ShortTestResult();
				PageValue.s_SelfResult[SELF_SHO_RESULT] = 1;       // 자체 점검 - 도통단선 점검 결과 확인
				g_MenuPage = 0;//g_MenuPage == 0 ? 1:0;   //0
				g_InputValue = 0;
				ButtonValue.s_NextButton = 0;
				T2CONbits.TON = 1;
				break;
			case 3:
				IEC1bits.INT1IE = 0;	// 인터럽트 1중지
				InsulationTestResult();
				PageValue.s_SelfResult[SELF_INS_RESULT] = 1;       // 자체 점검 - 절연 점검 결과 확인
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
				// <editor-fold defaultstate="collapsed" desc="이전화면">

			case 0:
				TestResultCheck(1);                 // 자체 점검 결과 화면 출력
				if(PageValue.s_SelfResult[SELF_RES_RESULT] == 1)	PageValue.s_SelfResult[SELF_RES_RESULT] = 0;
				else if(PageValue.s_SelfResult[SELF_SHO_RESULT] == 1)	PageValue.s_SelfResult[SELF_SHO_RESULT] = 0;
				else if(PageValue.s_SelfResult[SELF_INS_RESULT] == 1)	PageValue.s_SelfResult[SELF_INS_RESULT] = 0;
				IEC1bits.INT1IE = 1;	// 인터럽트 1동작
				// 자체 점검 결과 화면 출력시 버튼 플래그 초기화
				ButtonValue.s_BackButton = 0;
				g_MenuPage = 1; //g_MenuPage == 0 ? 1:0;   //1
				break;
			case 1:
				printf("i MENU/ModeMenu.jpg,0,0\r");    // 모드 선택 화면 출력
				AlramPrint();
				PageValue.s_First[MODE_SEL] = 1;           // 모드 선택 화면 토글
				PageValue.s_Self[SELF_TEST] = 0;           // 자체 점검 토글(초기화)
				ButtonValue.s_BackButton = 0;
				break;
				// </editor-fold>
			}
		}

		// </editor-fold>
	}

	else if(PageValue.s_Line[LINE_TEST] == 1)
	{
		// <editor-fold defaultstate="collapsed" desc="점화 계통 점검 선택">
		if(ButtonValue.s_OkButton == 1)
		{
			switch(g_InputValue)
			{
				// <editor-fold defaultstate="collapsed" desc="점검">
			case 1:
				printf("i TESTING/ResidualTesting.jpg,0,0\r");
				AlramPrint();
				ResidualVoltTest();
				PageValue.s_Line[LINE_RES_TEST] = 1;  // 자체 점검 완료 플래그
				printf("i MENU/LineTestMenu.jpg,0,0\r"); // 자체 점검 메뉴 출력
				AlramPrint();
				TestResultCheck(2);                  // 점검 결과 표시
				// 점검 완료시 버튼 플래그 초기화
				ButtonValue.s_OkButton = 0; // 점검이 완료 되어서
				g_MenuPage =1;// g_MenuPage == 0 ? 1:0;   //1
				g_InputValue = 0;
				break;
			case 2:
				printf("i TESTING/ShortTesting.jpg,0,0\r");
				AlramPrint();
				ShortTest();
				PageValue.s_Line[LINE_SHO_TEST] = 1;  // 도통/단선 점검
				printf("i MENU/LineTestMenu.jpg,0,0\r"); // 자체 점검 메뉴 출력
				AlramPrint();
				TestResultCheck(2);                  // 점검 결과 표시
				ButtonValue.s_OkButton = 0;       // 확인 버튼 초기화
				ButtonValue.s_BackButton = 0;
				ButtonValue.s_NextButton = 0;
				g_MenuPage =1;// g_MenuPage == 0 ? 1:0;   //1
				g_InputValue = 0;
				break;
			case 3:
				printf("i TESTING/InsulTesting.jpg,0,0\r");
				AlramPrint();
				InsulationTest();
				PageValue.s_Line[LINE_INS_TEST] = 1;  // 절연 점검
				printf("i MENU/LineTestMenu.jpg,0,0\r"); // 자체 점검 메뉴 출력
				AlramPrint();
				TestResultCheck(2);                  // 점검 결과 표시

				ButtonValue.s_OkButton = 0;       // 확인 버튼 초기화
				ButtonValue.s_BackButton = 0;
				ButtonValue.s_NextButton = 0;
				g_MenuPage =1;// g_MenuPage == 0 ? 1:0;   //1
				g_InputValue = 0;
				break;
			case 4:
				PageValue.s_Line[LINE_RES_TEST] = 1;  // 자체 점검
				printf("i TESTING/ResidualTesting.jpg,0,0\r");
				AlramPrint();
				ResidualVoltTest();
				PageValue.s_Line[LINE_SHO_TEST] = 1;  // 도통/단선 점검
				printf("i TESTING/ShortTesting.jpg,0,0\r");
				AlramPrint();
				ShortTest();
				PageValue.s_Line[LINE_INS_TEST] = 1;  // 절연 점검
				printf("i TESTING/InsulTesting.jpg,0,0\r");
				AlramPrint();
				InsulationTest();
				TestResultCheck(2);                  // 점검 결과 표시
				ButtonValue.s_OkButton = 0; // 점검이 완료 되어서
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
				// <editor-fold defaultstate="collapsed" desc="세부점검결과">
			case 1:
				IEC1bits.INT1IE = 0;	// 인터럽트 1중지
				ResidualVoltTestResult();
				PageValue.s_LineResult[LINE_RES_RESULT] = 1;       // 자체 점검 - 잔류 전압 결과 확인
				g_MenuPage = 0;//g_MenuPage == 0 ? 1:0;   //0
				g_InputValue = 0;
				ButtonValue.s_NextButton = 0;
				break;
			case 2:
				IEC1bits.INT1IE = 0;	// 인터럽트 1중지
				ShortTestResult();
				PageValue.s_LineResult[LINE_SHO_RESULT] = 1;       // 자체 점검 - 도통단선 점검 결과 확인
				g_MenuPage = 0;//g_MenuPage == 0 ? 1:0;   //0
				g_InputValue = 0;
				ButtonValue.s_NextButton = 0;
				break;
			case 3:
				IEC1bits.INT1IE = 0;	// 인터럽트 1중지
				InsulationTestResult();
				PageValue.s_LineResult[LINE_INS_RESULT] = 1;       // 자체 점검 - 절연 점검 결과 확인
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
				// <editor-fold defaultstate="collapsed" desc="이전화면">

			case 0:
				TestResultCheck(2);                 // 자체 점검 결과 화면 출력
				if(PageValue.s_LineResult[LINE_RES_RESULT] == 1)	PageValue.s_LineResult[LINE_RES_RESULT] = 0;
				else if(PageValue.s_LineResult[LINE_SHO_RESULT] == 1)	PageValue.s_LineResult[LINE_SHO_RESULT] = 0;
				else if(PageValue.s_LineResult[LINE_INS_RESULT] == 1)	PageValue.s_LineResult[LINE_INS_RESULT] = 0;
				IEC1bits.INT1IE = 1;	// 인터럽트 1중지
				// 자체 점검 결과 화면 출력시 버튼 플래그 초기화
				ButtonValue.s_BackButton = 0;
				g_MenuPage = 1; //g_MenuPage == 0 ? 1:0;   //1
				break;
			case 1:
				printf("i MENU/ModeMenu.jpg,0,0\r");    // 모드 선택 화면 출력
				AlramPrint();
				PageValue.s_First[MODE_SEL] = 1;           // 모드 선택 화면 토글
				PageValue.s_Line[LINE_TEST] = 0;           // 자체 점검 토글(초기화)
				ButtonValue.s_BackButton = 0;
				break;
				// </editor-fold>
			}
		}
		// </editor-fold>
	}
	else if((PageValue.s_Fire[FIRE_TEST] == 1) )//&& (PageValue.s_Check[CHK_LAN]==1) // 발사 계통 점검 세부 메뉴 선택 1,2호탄 선택
	{
		if(ButtonValue.s_OkButton == 1)
		{
			switch(g_InputValue)
			{
				// <editor-fold defaultstate="collapsed" desc="발사 계통 점검 세부 메뉴 선택">
			case  0x01 :
				printf("i MENU/FireEachTestMenu_1.jpg,0,0\r");                              // 1호탄 개별 점검
				TestResultCheck(3);             // 점검 결과 체크 표시
				PageValue.s_Fire[FIRE_TEST_EACH_1] = 1;
				PageValue.s_Fire[FIRE_TEST] = 0;
				ButtonValue.s_OkButton = 0;       // 확인 버튼 초기화
				g_InputValue = 0;
				break;
			case  0x02 :
				printf("i MENU/FireTotalTestMenu_1.jpg,0,0\r");                              // 1호탄 통합 점검
				TestResultCheck(3);             // 점검 결과 체크 표시
				PageValue.s_Fire[FIRE_TEST_TOTAL_1] = 1;
				PageValue.s_Fire[FIRE_TEST] = 0;
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				break;
			case  0x03 :
				printf("i MENU/FireEachTestMenu_2.jpg,0,0\r");                              // 2호탄 개별 점검
				TestResultCheck(4);             // 점검 결과 체크 표시
				PageValue.s_Fire[FIRE_TEST_EACH_2] = 1;
				PageValue.s_Fire[FIRE_TEST] = 0;
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				break;
			case  0x04 :
				printf("i MENU/FireTotalTestMenu_2.jpg,0,0\r");                              // 2호탄 통합 점검
				TestResultCheck(4);             // 점검 결과 체크 표시
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
			// <editor-fold defaultstate="collapsed" desc="이전">
			printf("i MENU/ModeMenu.jpg,0,0\r");        // 모드 선택 화면 출력
			AlramPrint();

			PageValue.s_Fire[FIRE_TEST] = 0;
			PageValue.s_First[MODE_SEL] = 1;
			if(PageValue.s_Fire[FIRE_TEST_EACH_1] == 1)PageValue.s_Fire[FIRE_TEST_EACH_1] = 0;
			else if(PageValue.s_Fire[FIRE_TEST_TOTAL_1] == 1)PageValue.s_Fire[FIRE_TEST_TOTAL_1] = 0;
			else if(PageValue.s_Fire[FIRE_TEST_EACH_2] == 1)PageValue.s_Fire[FIRE_TEST_EACH_2] = 0;
			else if(PageValue.s_Fire[FIRE_TEST_TOTAL_2] == 1)PageValue.s_Fire[FIRE_TEST_TOTAL_2] = 0;
			ButtonValue.s_BackButton = 0;                  // 이전 버튼 토글(초기화)
			g_InputValue = 0;
			// </editor-fold>
		}
	}
	else if(PageValue.s_Fire[FIRE_TEST_EACH_1]==1) //&& (PageValue.s_Check[CHK_LAN])) // 1호탄 개별 점검 개별 메시지 전송 및 수신
	{
		// <editor-fold defaultstate="collapsed" desc="1호탄 개별 점검">
		if(ButtonValue.s_OkButton == 1)
		{
			// <editor-fold defaultstate="collapsed" desc="1호탄 개별 점검">
			switch(g_InputValue)
			{
			case 1:
				SendData(MSL);  // 전송할 메시지 변수 저장
				//인터럽트0번 동작
				// 메시지 전송하고 수신하면 인터럽트 0번 종료
				if(FireStatus.s_MSL >= 0) // 데이터가 수신이 되었으면 메시지에 들어있는 값은 0보다 크거나 같다
				{
					PageValue.s_FireRecvResult_1[MSL_RESULT] = 1;  // 수신완료 플래그 설정하고
					TestResultCheck(3);         // 점검 결과 표시
				}
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 2:
				SendData(EXT);  // 전송할 메시지 변수 저장
				//인터럽트0번 동작
				// 메시지 전송하고 수신하면 인터럽트 0번 종료
				if(FireStatus.s_EXT >= 0) // 데이터가 수신이 되었으면 메시지에 들어있는 값은 0보다 크거나 같다
				{
					PageValue.s_FireRecvResult_1[EXT_RESULT] = 1;  // 수신완료 플래그 설정하고
					TestResultCheck(3);         // 점검 결과 표시
				}

				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 3:
				SendData(EB);  // 전송할 메시지 변수 저장
				//인터럽트0번 동작
				// 메시지 전송하고 수신하면 인터럽트 0번 종료
				if(FireStatus.s_EB >= 0) // 데이터가 수신이 되었으면 메시지에 들어있는 값은 0보다 크거나 같다
				{
					PageValue.s_FireRecvResult_1[EB_RESULT] = 1;  // 수신완료 플래그 설정하고
					TestResultCheck(3);         // 점검 결과 표시
				}
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 4:
				SendData(BAT);  // 전송할 메시지 변수 저장
				//인터럽트0번 동작
				// 메시지 전송하고 수신하면 인터럽트 0번 종료
				if(FireStatus.s_BAT >= 0) // 데이터가 수신이 되었으면 메시지에 들어있는 값은 0보다 크거나 같다
				{
					PageValue.s_FireRecvResult_1[BAT_RESULT] = 1;  // 수신완료 플래그 설정하고
					TestResultCheck(3);         // 점검 결과 표시
				}
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 5:
				SendData(ABAT);  // 전송할 메시지 변수 저장
				//인터럽트0번 동작
				// 메시지 전송하고 수신하면 인터럽트 0번 종료
				if(FireStatus.s_ABAT >= 0) // 데이터가 수신이 되었으면 메시지에 들어있는 값은 0보다 크거나 같다
				{
					PageValue.s_FireRecvResult_1[ABAT_RESULT] = 1;  // 수신완료 플래그 설정하고
					TestResultCheck(3);         // 점검 결과 표시
				}
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 6:
				SendData(BDU);  // 전송할 메시지 변수 저장
				//인터럽트0번 동작
				// 메시지 전송하고 수신하면 인터럽트 0번 종료
				if(FireStatus.s_BDU >= 0) // 데이터가 수신이 되었으면 메시지에 들어있는 값은 0보다 크거나 같다
				{
					PageValue.s_FireRecvResult_1[BDU_RESULT] = 1;  // 수신완료 플래그 설정하고
					TestResultCheck(3);         // 점검 결과 표시
				}
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 7:
				SendData(ARMING);  // 전송할 메시지 변수 저장
				//인터럽트0번 동작
				// 메시지 전송하고 수신하면 인터럽트 0번 종료
				if(FireStatus.s_ARMING >= 0) // 데이터가 수신이 되었으면 메시지에 들어있는 값은 0보다 크거나 같다
				{
					PageValue.s_FireRecvResult_1[ARMING_RESULT] = 1;  // 수신완료 플래그 설정하고
					TestResultCheck(3);         // 점검 결과 표시
				}
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 8:
				SendData(INTARM);  // 전송할 메시지 변수 저장
				//인터럽트0번 동작
				// 메시지 전송하고 수신하면 인터럽트 0번 종료
				if(FireStatus.s_INTARM >= 0) // 데이터가 수신이 되었으면 메시지에 들어있는 값은 0보다 크거나 같다
				{
					PageValue.s_FireRecvResult_1[INTARM_RESULT] = 1;  // 수신완료 플래그 설정하고
					TestResultCheck(3);         // 점검 결과 표시
				}
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 9:
				SendData(INT);  // 전송할 메시지 변수 저장
				//인터럽트0번 동작
				// 메시지 전송하고 수신하면 인터럽트 0번 종료
				if(FireStatus.s_INT >= 0) // 데이터가 수신이 되었으면 메시지에 들어있는 값은 0보다 크거나 같다
				{
					PageValue.s_FireRecvResult_1[INT_RESULT] = 1;  // 수신완료 플래그 설정하고
					TestResultCheck(3);         // 점검 결과 표시
				}
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			}
			// </editor-fold>
		}
		else if(ButtonValue.s_NextButton == 1)
		{
			// <editor-fold defaultstate="collapsed" desc="1호탄 개별 점검 세부 결과">
			switch(g_InputValue)
			{
			case 1:

				printf("i Result/MSLEXTPWR.jpg,0,0\r");
				RecvDataSort(MSL,FIRST);    // 파싱된 데이터를 선택하는 세부 점검 결과별 점검 결과를 표시
				AlramPrint();
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;
			case 2:
				printf("i Result/TESTEXTPWR.jpg,0,0\r");
				AlramPrint();
				RecvDataSort(EXT,FIRST);    // 파싱된 데이터를 선택하는 세부 점검 결과별 점검 결과를 표시
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;

			case 3:
				printf("i Result/EBSQB.jpg,0,0\r");
				AlramPrint();
				RecvDataSort(EB,FIRST);    // 파싱된 데이터를 선택하는 세부 점검 결과별 점검 결과를 표시
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;
			case 4:
				printf("i Result/BATSQB.jpg,0,0\r");
				AlramPrint();
				RecvDataSort(BAT,FIRST);    // 파싱된 데이터를 선택하는 세부 점검 결과별 점검 결과를 표시
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;

			case 5:
				printf("i Result/ABATSQB.jpg,0,0\r");
				AlramPrint();
				RecvDataSort(ABAT,FIRST);    // 파싱된 데이터를 선택하는 세부 점검 결과별 점검 결과를 표시
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;
			case 6:
				printf("i Result/BDUSQB.jpg,0,0\r");
				AlramPrint();
				RecvDataSort(BDU,FIRST);    // 파싱된 데이터를 선택하는 세부 점검 결과별 점검 결과를 표시
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;

			case 7:
				printf("i Result/ARMING.jpg,0,0\r");
				AlramPrint();
				RecvDataSort(ARMING,FIRST);    // 파싱된 데이터를 선택하는 세부 점검 결과별 점검 결과를 표시
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;
			case 8:
				printf("i Result/INTARMING.jpg,0,0\r");
				AlramPrint();
				RecvDataSort(INTARM,FIRST);    // 파싱된 데이터를 선택하는 세부 점검 결과별 점검 결과를 표시
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;

			case 9:
				printf("i Result/INTSQB.jpg,0,0\r");
				AlramPrint();
				RecvDataSort(INT,FIRST);    // 파싱된 데이터를 선택하는 세부 점검 결과별 점검 결과를 표시
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;

			}
			// </editor-fold>
		}
		else if((ButtonValue.s_BackButton == 1) && (PORTEbits.RE4 == 0))
		{
			// <editor-fold defaultstate="collapsed" desc="이전">
			switch(g_MenuPage)
			{
			case 0:
				printf("i MENU/FireEachTestMenu_1.jpg,0,0\r");
				TestResultCheck(3);                 // 자체 점검 결과 화면 출력
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
				IEC1bits.INT1IE = 1;	// 인터럽트 1동작
				// 자체 점검 결과 화면 출력시 버튼 플래그 초기화
				ButtonValue.s_BackButton = 0;
				g_MenuPage = 1; //g_MenuPage == 0 ? 1:0;   //1
				break;
			case 1:
				printf("i MENU/FireTestMenu.jpg,0,0\r");
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
		// <editor-fold defaultstate="collapsed" desc="1호탄 통합 점검">
		if(ButtonValue.s_OkButton == 1)
		{
			// <editor-fold defaultstate="collapsed" desc="1호탄 통합 점검">
			// 1호탄 통합 점검 시작 화면 출력
			switch(g_TotalSel)
			{
			case 1:
				// MSL 메시지 전송중 화면 출력
				SendData(MSL);  // 전송할 메시지 변수 저장
				//인터럽트0번 동작
				// 메시지 전송하고 수신하면 인터럽트 0번 종료
				if(FireStatus.s_MSL >= 0) // 데이터가 수신이 되었으면 메시지에 들어있는 값은 0보다 크거나 같다
				{
					PageValue.s_FireRecvResult_1[MSL_RESULT] = 1;  // 수신완료 플래그 설정하고
					// MSL 메시지 수신완료 화면 출력
				}
				break;
			case 2:
				SendData(EXT);  // 전송할 메시지 변수 저장
				//인터럽트0번 동작
				// 메시지 전송하고 수신하면 인터럽트 0번 종료
				if(FireStatus.s_EXT >= 0) // 데이터가 수신이 되었으면 메시지에 들어있는 값은 0보다 크거나 같다
				{
					PageValue.s_FireRecvResult_1[EXT_RESULT] = 1;  // 수신완료 플래그 설정하고
				}
				break;
			case 3:
				SendData(EB);  // 전송할 메시지 변수 저장
				//인터럽트0번 동작
				// 메시지 전송하고 수신하면 인터럽트 0번 종료
				if(FireStatus.s_EB >= 0) // 데이터가 수신이 되었으면 메시지에 들어있는 값은 0보다 크거나 같다
				{
					PageValue.s_FireRecvResult_1[EB_RESULT] = 1;  // 수신완료 플래그 설정하고
				}
				break;
			case 4:
				SendData(BAT);  // 전송할 메시지 변수 저장
				//인터럽트0번 동작
				// 메시지 전송하고 수신하면 인터럽트 0번 종료
				if(FireStatus.s_BAT >= 0) // 데이터가 수신이 되었으면 메시지에 들어있는 값은 0보다 크거나 같다
				{
					PageValue.s_FireRecvResult_1[BAT_RESULT] = 1;  // 수신완료 플래그 설정하고
				}
				break;
			case 5:
				SendData(ABAT);  // 전송할 메시지 변수 저장
				//인터럽트0번 동작
				// 메시지 전송하고 수신하면 인터럽트 0번 종료
				if(FireStatus.s_ABAT >= 0) // 데이터가 수신이 되었으면 메시지에 들어있는 값은 0보다 크거나 같다
				{
					PageValue.s_FireRecvResult_1[ABAT_RESULT] = 1;  // 수신완료 플래그 설정하고
				}
				break;
			case 6:
				SendData(BDU);  // 전송할 메시지 변수 저장
				//인터럽트0번 동작
				// 메시지 전송하고 수신하면 인터럽트 0번 종료
				if(FireStatus.s_BDU >= 0) // 데이터가 수신이 되었으면 메시지에 들어있는 값은 0보다 크거나 같다
				{
					PageValue.s_FireRecvResult_1[BDU_RESULT] = 1;  // 수신완료 플래그 설정하고
				}
				break;
			case 7:
				SendData(ARMING);  // 전송할 메시지 변수 저장
				//인터럽트0번 동작
				// 메시지 전송하고 수신하면 인터럽트 0번 종료
				if(FireStatus.s_ARMING >= 0) // 데이터가 수신이 되었으면 메시지에 들어있는 값은 0보다 크거나 같다
				{
					PageValue.s_FireRecvResult_1[ARMING_RESULT] = 1;  // 수신완료 플래그 설정하고
				}
				break;
			case 8:
				SendData(INTARM);  // 전송할 메시지 변수 저장
				//인터럽트0번 동작
				// 메시지 전송하고 수신하면 인터럽트 0번 종료
				if(FireStatus.s_INTARM >= 0) // 데이터가 수신이 되었으면 메시지에 들어있는 값은 0보다 크거나 같다
				{
					PageValue.s_FireRecvResult_1[INTARM_RESULT] = 1;  // 수신완료 플래그 설정하고
				}
				break;
			case 9:
				SendData(INT);  // 전송할 메시지 변수 저장
				//인터럽트0번 동작
				// 메시지 전송하고 수신하면 인터럽트 0번 종료
				if(FireStatus.s_INT >= 0) // 데이터가 수신이 되었으면 메시지에 들어있는 값은 0보다 크거나 같다
				{
					PageValue.s_FireRecvResult_1[INT_RESULT] = 1;  // 수신완료 플래그 설정하고
				}
				break;
			default:
				//10일 경우에 아무런 동작을 하지 않는다.
				break;
			}
			// </editor-fold>

		}
		else if(ButtonValue.s_NextButton == 1)
		{
			// <editor-fold defaultstate="collapsed" desc="1호탄 통합 점검 세부 결과">
			switch(g_InputValue)
			{
			case 1:

				printf("i Result/MSLEXTPWR.jpg,0,0\r");
				RecvDataSort(MSL,FIRST);    // 파싱된 데이터를 선택하는 세부 점검 결과별 점검 결과를 표시
				AlramPrint();
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;
			case 2:
				printf("i Result/TESTEXTPWR.jpg,0,0\r");
				AlramPrint();
				RecvDataSort(EXT,FIRST);    // 파싱된 데이터를 선택하는 세부 점검 결과별 점검 결과를 표시
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;

			case 3:
				printf("i Result/EBSQB.jpg,0,0\r");
				AlramPrint();
				RecvDataSort(EB,FIRST);    // 파싱된 데이터를 선택하는 세부 점검 결과별 점검 결과를 표시
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;
			case 4:
				printf("i Result/BATSQB.jpg,0,0\r");
				AlramPrint();
				RecvDataSort(BAT,FIRST);    // 파싱된 데이터를 선택하는 세부 점검 결과별 점검 결과를 표시
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;

			case 5:
				printf("i Result/ABATSQB.jpg,0,0\r");
				AlramPrint();
				RecvDataSort(ABAT,FIRST);    // 파싱된 데이터를 선택하는 세부 점검 결과별 점검 결과를 표시
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;
			case 6:
				printf("i Result/BDUSQB.jpg,0,0\r");
				AlramPrint();
				RecvDataSort(BDU,FIRST);    // 파싱된 데이터를 선택하는 세부 점검 결과별 점검 결과를 표시
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;

			case 7:
				printf("i Result/ARMING.jpg,0,0\r");
				AlramPrint();
				RecvDataSort(ARMING,FIRST);    // 파싱된 데이터를 선택하는 세부 점검 결과별 점검 결과를 표시
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;
			case 8:
				printf("i Result/INTARMING.jpg,0,0\r");
				AlramPrint();
				RecvDataSort(INTARM,FIRST);    // 파싱된 데이터를 선택하는 세부 점검 결과별 점검 결과를 표시
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;

			case 9:
				printf("i Result/INTSQB.jpg,0,0\r");
				AlramPrint();
				RecvDataSort(INT,FIRST);    // 파싱된 데이터를 선택하는 세부 점검 결과별 점검 결과를 표시
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;

			}
			// </editor-fold>
		}
		else if((ButtonValue.s_BackButton == 1) && (PORTEbits.RE4 == 0))
		{
			// <editor-fold defaultstate="collapsed" desc="이전">
			switch(g_MenuPage)
			{
			case 0:
				printf("i MENU/FireTotalTestMenu_1.jpg,0,0\r");
				TestResultCheck(3);                 // 자체 점검 결과 화면 출력
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
				IEC1bits.INT1IE = 1;	// 인터럽트 1중지
				// 자체 점검 결과 화면 출력시 버튼 플래그 초기화
				ButtonValue.s_BackButton = 0;
				g_MenuPage = 1; //g_MenuPage == 0 ? 1:0;   //1
				break;
			case 1:
				printf("i MENU/FireTestMenu.jpg,0,0\r");
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


	else if(PageValue.s_Fire[FIRE_TEST_EACH_2]==1) //&& (PageValue.s_Check[CHK_LAN])) // 1호탄 개별 점검 개별 메시지 전송 및 수신
	{
		// <editor-fold defaultstate="collapsed" desc="2호탄 개별 점검">
		if(ButtonValue.s_OkButton == 1)
		{

		}
		else if(ButtonValue.s_NextButton == 1)
		{

		}
		else if((ButtonValue.s_BackButton == 1) && (PORTEbits.RE4 == 0))
		{
			// <editor-fold defaultstate="collapsed" desc="이전">
			switch(g_MenuPage)
			{
			case 0:
				printf("i MENU/FireEachTestMenu_2.jpg,0,0\r");
				TestResultCheck(4);                 // 자체 점검 결과 화면 출력
				if(PageValue.s_FireDetailResult_2[MSL_RESULT]==1) PageValue.s_FireDetailResult_2[MSL_RESULT]=0;
				else if(PageValue.s_FireDetailResult_2[EXT_RESULT]==1) PageValue.s_FireDetailResult_2[EXT_RESULT]=0;
				else if(PageValue.s_FireDetailResult_2[EB_RESULT]==1) PageValue.s_FireDetailResult_2[EB_RESULT]=0;
				else if(PageValue.s_FireDetailResult_2[BAT_RESULT]==1) PageValue.s_FireDetailResult_2[BAT_RESULT]=0;
				else if(PageValue.s_FireDetailResult_2[ABAT_RESULT]==1) PageValue.s_FireDetailResult_2[ABAT_RESULT]=0;
				else if(PageValue.s_FireDetailResult_2[BDU_RESULT]==1) PageValue.s_FireDetailResult_2[BDU_RESULT]=0;
				else if(PageValue.s_FireDetailResult_2[ARMING_RESULT]==1) PageValue.s_FireDetailResult_2[ARMING_RESULT]=0;
				else if(PageValue.s_FireDetailResult_2[INTARM_RESULT]==1) PageValue.s_FireDetailResult_2[INTARM_RESULT]=0;
				else if(PageValue.s_FireDetailResult_2[INT_RESULT]==1) PageValue.s_FireDetailResult_2[INT_RESULT]=0;
				//IEC1bits.INT1IE = 1;	// 인터럽트 1중지
				// 자체 점검 결과 화면 출력시 버튼 플래그 초기화
				ButtonValue.s_BackButton = 0;
				g_MenuPage = 1; //g_MenuPage == 0 ? 1:0;   //1
				break;
			case 1:
				printf("i MENU/FireTestMenu.jpg,0,0\r");
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
	else if(PageValue.s_Fire[FIRE_TEST_TOTAL_2]==1) //&& (PageValue.s_Check[CHK_LAN])) // 2호탄 통합 점검 개별 메시지 전송 및 수신
	{
		// <editor-fold defaultstate="collapsed" desc="2호탄 통합 점검">
		if(ButtonValue.s_OkButton == 1)
		{

		}
		else if(ButtonValue.s_NextButton == 1)
		{

		}
		else if((ButtonValue.s_BackButton == 1) && (PORTEbits.RE4 == 0))
		{
			// <editor-fold defaultstate="collapsed" desc="이전">
			switch(g_MenuPage)
			{
			case 0:
				printf("i MENU/FireTotalTestMenu_2.jpg,0,0\r");
				TestResultCheck(4);                 // 자체 점검 결과 화면 출력
				if(PageValue.s_FireDetailResult_2[MSL_RESULT]==1) PageValue.s_FireDetailResult_2[MSL_RESULT]=0;
				else if(PageValue.s_FireDetailResult_2[EXT_RESULT]==1) PageValue.s_FireDetailResult_2[EXT_RESULT]=0;
				else if(PageValue.s_FireDetailResult_2[EB_RESULT]==1) PageValue.s_FireDetailResult_2[EB_RESULT]=0;
				else if(PageValue.s_FireDetailResult_2[BAT_RESULT]==1) PageValue.s_FireDetailResult_2[BAT_RESULT]=0;
				else if(PageValue.s_FireDetailResult_2[ABAT_RESULT]==1) PageValue.s_FireDetailResult_2[ABAT_RESULT]=0;
				else if(PageValue.s_FireDetailResult_2[BDU_RESULT]==1) PageValue.s_FireDetailResult_2[BDU_RESULT]=0;
				else if(PageValue.s_FireDetailResult_2[ARMING_RESULT]==1) PageValue.s_FireDetailResult_2[ARMING_RESULT]=0;
				else if(PageValue.s_FireDetailResult_2[INTARM_RESULT]==1) PageValue.s_FireDetailResult_2[INTARM_RESULT]=0;
				else if(PageValue.s_FireDetailResult_2[INT_RESULT]==1) PageValue.s_FireDetailResult_2[INT_RESULT]=0;
				IEC1bits.INT1IE = 1;	// 인터럽트 1중지
				// 자체 점검 결과 화면 출력시 버튼 플래그 초기화
				ButtonValue.s_BackButton = 0;
				g_MenuPage = 1; //g_MenuPage == 0 ? 1:0;   //1
				break;
			case 1:
				printf("i MENU/FireTestMenu.jpg,0,0\r");
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

void SendData(unsigned int Name)    // 16비트 배열로 전송됨 name 별 메시지 전송이 다름
{
	if(PageValue.s_Fire[FIRE_TEST_EACH_1]==1) FireTestCheck.s_Opcode = 0x4100;
	else if(PageValue.s_Fire[FIRE_TEST_EACH_2]==1) FireTestCheck.s_Opcode = 0x4200;
	FireTestCheck.s_Repeat = (FireTestCheck.s_Repeat == 3) ? 0 : FireTestCheck.s_Repeat+1 ;
	FireTestCheck.s_SeqNo = (FireTestCheck.s_SeqNo == 65535) ? 0 : FireTestCheck.s_SeqNo+1 ;
	FireTestCheck.s_DataSize = 0x0008;
	FireTestCheck.s_LchrId = 0x0001;
	FireTestCheck.s_McuId = 0x0001;
	FireTestCheck.s_Time = 0;
	g_DataBuffer[0] = FireTestCheck.s_Opcode;
	g_DataBuffer[1] = FireTestCheck.s_Repeat;
	g_DataBuffer[2] = FireTestCheck.s_SeqNo;
	g_DataBuffer[3] = FireTestCheck.s_DataSize;
	g_DataBuffer[4] = (FireTestCheck.s_LchrId << 7);
	g_DataBuffer[4] = FireTestCheck.s_McuId ;
	g_DataBuffer[5] = FireTestCheck.s_Time;
	switch(Name)
	{
	case CON: //  통신 점검
		ConnectCheck.s_Opcode = 0x4005;
		ConnectCheck.s_Repeat = (ConnectCheck.s_Repeat == 3) ? 0 : ConnectCheck.s_Repeat+1 ;
		ConnectCheck.s_SeqNo = (ConnectCheck.s_SeqNo == 65535) ? 0 : ConnectCheck.s_SeqNo+1 ;
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
	case MSL: // 1호탄 발사 계통 점검
		FireTestCheck.s_MSLPWR = 0x02;
		FireTestCheck.s_EXTPWR = 0;
		FireTestCheck.s_EBSQUIB = 0;
		FireTestCheck.s_BATSQUIB = 0;
		FireTestCheck.s_ABATSQUIB = 0;
		FireTestCheck.s_BDUSQUIB = 0;
		FireTestCheck.s_ARMING = 0;
		FireTestCheck.s_INTARM = 0;
		FireTestCheck.s_INTSQUIB = 0;
		g_DataBuffer[6] =  (FireTestCheck.s_MSLPWR << 13);
		g_DataBuffer[7] = 0;
		break;
	case EXT:
		FireTestCheck.s_MSLPWR = 0;
		FireTestCheck.s_EXTPWR = 0x01;
		FireTestCheck.s_EBSQUIB = 0;
		FireTestCheck.s_BATSQUIB = 0;
		FireTestCheck.s_ABATSQUIB = 0;
		FireTestCheck.s_BDUSQUIB = 0;
		FireTestCheck.s_ARMING = 0;
		FireTestCheck.s_INTARM = 0;
		FireTestCheck.s_INTSQUIB = 0;
		g_DataBuffer[6] =  (FireTestCheck.s_EXTPWR << 11);
		g_DataBuffer[7] = 0;
		break;
	case EB:
		FireTestCheck.s_MSLPWR = 0;
		FireTestCheck.s_EXTPWR = 0;
		FireTestCheck.s_EBSQUIB = 0x01;
		FireTestCheck.s_BATSQUIB = 0;
		FireTestCheck.s_ABATSQUIB = 0;
		FireTestCheck.s_BDUSQUIB = 0;
		FireTestCheck.s_ARMING = 0;
		FireTestCheck.s_INTARM = 0;
		FireTestCheck.s_INTSQUIB = 0;
		g_DataBuffer[6] =  (FireTestCheck.s_EBSQUIB << 9);
		g_DataBuffer[7] = 0;
		break;

	case BAT:
		FireTestCheck.s_MSLPWR = 0;
		FireTestCheck.s_EXTPWR = 0;
		FireTestCheck.s_EBSQUIB = 0;
		FireTestCheck.s_BATSQUIB = 0x01;
		FireTestCheck.s_ABATSQUIB = 0;
		FireTestCheck.s_BDUSQUIB = 0;
		FireTestCheck.s_ARMING = 0;
		FireTestCheck.s_INTARM = 0;
		FireTestCheck.s_INTSQUIB = 0;
		g_DataBuffer[6] =  (FireTestCheck.s_BATSQUIB << 7);
		g_DataBuffer[7] = 0;
		break;
	case ABAT:
		FireTestCheck.s_MSLPWR = 0;
		FireTestCheck.s_EXTPWR = 0;
		FireTestCheck.s_EBSQUIB = 0;
		FireTestCheck.s_BATSQUIB = 0;
		FireTestCheck.s_ABATSQUIB = 0x01;
		FireTestCheck.s_BDUSQUIB = 0;
		FireTestCheck.s_ARMING = 0;
		FireTestCheck.s_INTARM = 0;
		FireTestCheck.s_INTSQUIB = 0;
		g_DataBuffer[6] =  (FireTestCheck.s_ABATSQUIB << 5);
		g_DataBuffer[7] = 0;
		break;
	case BDU:
		FireTestCheck.s_MSLPWR = 0;
		FireTestCheck.s_EXTPWR = 0;
		FireTestCheck.s_EBSQUIB = 0;
		FireTestCheck.s_BATSQUIB = 0;
		FireTestCheck.s_ABATSQUIB = 0;
		FireTestCheck.s_BDUSQUIB = 0x01;
		FireTestCheck.s_ARMING = 0;
		FireTestCheck.s_INTARM = 0;
		FireTestCheck.s_INTSQUIB = 0;
		g_DataBuffer[6] =  (FireTestCheck.s_BDUSQUIB << 3);
		g_DataBuffer[7] = 0;
		break;
	case ARMING:
		FireTestCheck.s_MSLPWR = 0;
		FireTestCheck.s_EXTPWR = 0;
		FireTestCheck.s_EBSQUIB = 0;
		FireTestCheck.s_BATSQUIB = 0;
		FireTestCheck.s_ABATSQUIB = 0;
		FireTestCheck.s_BDUSQUIB = 0;
		FireTestCheck.s_ARMING = 0x01;
		FireTestCheck.s_INTARM = 0;
		FireTestCheck.s_INTSQUIB = 0;
		g_DataBuffer[6] =  (FireTestCheck.s_ARMING << 1);
		g_DataBuffer[7] = 0;
		break;
	case INTARM:
		FireTestCheck.s_MSLPWR = 0;
		FireTestCheck.s_EXTPWR = 0;
		FireTestCheck.s_EBSQUIB = 0;
		FireTestCheck.s_BATSQUIB = 0;
		FireTestCheck.s_ABATSQUIB = 0;
		FireTestCheck.s_BDUSQUIB = 0;
		FireTestCheck.s_ARMING = 0;
		FireTestCheck.s_INTARM = 0x01;
		FireTestCheck.s_INTSQUIB = 0;
		g_DataBuffer[6] =  FireTestCheck.s_INTARM << 0;
		g_DataBuffer[7] = 0;
		break;
	case INT:
		FireTestCheck.s_MSLPWR = 0;
		FireTestCheck.s_EXTPWR = 0;
		FireTestCheck.s_EBSQUIB = 0;
		FireTestCheck.s_BATSQUIB = 0;
		FireTestCheck.s_ABATSQUIB = 0;
		FireTestCheck.s_BDUSQUIB = 0;
		FireTestCheck.s_ARMING = 0;
		FireTestCheck.s_INTARM = 0;
		FireTestCheck.s_INTSQUIB = 0x01;
		g_DataBuffer[6] = 0;
		g_DataBuffer[7] = (FireTestCheck.s_INTSQUIB << 13);
		break;
	}
}

void RecvDataParsing(unsigned int Name)                     // 16비트 배열로 수신됨 CON_ACK, FIRE_ACK
{
	switch(Name)
	{
	case CON_ACK: // 통신 점검 응답 
		ConnectCheckAck.s_Opcode = g_TempBuffer[0];
		ConnectCheckAck.s_Repeat = g_TempBuffer[1];
		ConnectCheckAck.s_SeqNo = g_TempBuffer[2];
		ConnectCheckAck.s_DataSize = g_TempBuffer[3];
		ConnectCheckAck.s_LchrId = g_TempBuffer[4];
		ConnectCheckAck.s_McuId = g_TempBuffer[5];
		if( ConnectCheckAck.s_Opcode == 0x4005) PageValue.s_Check[CHK_LAN] = 1; // 랜 연결됨 

		break;
	case FIRE_ACK:
		FireTestCheckResult.s_Opcode = g_TempBuffer[0]; 
		FireTestCheckResult.s_Repeat = g_TempBuffer[1];
		FireTestCheckResult.s_SeqNo = g_TempBuffer[2];
		FireTestCheckResult.s_DataSize = g_TempBuffer[3];
		FireTestCheckResult.s_LchrId = (g_TempBuffer[4] & 0xFF00) >> 8;
		FireTestCheckResult.s_McuId = (g_TempBuffer[4] & 0x00FF);

		FireTestCheckResult.s_MSLPWR = (g_TempBuffer[5] & 0x3000)>> 12; 
		FireTestCheckResult.s_EXTPWR = (g_TempBuffer[5] & 0x0300)>> 8;
		FireTestCheckResult.s_EBSQUIB = (g_TempBuffer[5] & 0x001F);
		FireTestCheckResult.EB.s_EB1SQUIB1 = FireTestCheckResult.s_EBSQUIB & 0x01;
		FireTestCheckResult.EB.s_EB1SQUIB2 = FireTestCheckResult.s_EBSQUIB & 0x02;
		FireTestCheckResult.EB.s_EB2SQUIB1 = FireTestCheckResult.s_EBSQUIB & 0x04;
		FireTestCheckResult.EB.s_EB2SQUIB2 = FireTestCheckResult.s_EBSQUIB & 0x08;
		FireTestCheckResult.EB.s_Status =  FireTestCheckResult.s_EBSQUIB & 0x10;

		FireTestCheckResult.s_BATSQUIB = (g_TempBuffer[6] & 0xFF00)>> 8;
		FireTestCheckResult.BAT.s_BAT1SQUIB1 = FireTestCheckResult.s_BATSQUIB & 0x01;
		FireTestCheckResult.BAT.s_BAT1SQUIB2 = FireTestCheckResult.s_BATSQUIB & 0x02;
		FireTestCheckResult.BAT.s_BAT2SQUIB1 = FireTestCheckResult.s_BATSQUIB & 0x04;
		FireTestCheckResult.BAT.s_BAT2SQUIB2 = FireTestCheckResult.s_BATSQUIB & 0x08;
		FireTestCheckResult.BAT.s_Status =  FireTestCheckResult.s_BATSQUIB & 0x10;

		FireTestCheckResult.s_ABATSQUIB = (g_TempBuffer[6] & 0x00FF);
		FireTestCheckResult.ABAT.s_ABAT1SQUIB1 = FireTestCheckResult.s_ABATSQUIB & 0x01;
		FireTestCheckResult.ABAT.s_ABAT1SQUIB2 = FireTestCheckResult.s_ABATSQUIB & 0x02;
		FireTestCheckResult.ABAT.s_ABAT2SQUIB1 = FireTestCheckResult.s_ABATSQUIB & 0x04;
		FireTestCheckResult.ABAT.s_ABAT2SQUIB2 = FireTestCheckResult.s_ABATSQUIB & 0x08;
		FireTestCheckResult.ABAT.s_Status =  FireTestCheckResult.s_ABATSQUIB & 0x10;

		FireTestCheckResult.s_BDUSQUIB = (g_TempBuffer[7] & 0xF000)>> 8;
		FireTestCheckResult.BDU.s_BDUSQUIB1 = FireTestCheckResult.s_BDUSQUIB & 0x10;
		FireTestCheckResult.BDU.s_BDUSQUIB2 = FireTestCheckResult.s_BDUSQUIB & 0x20;
		FireTestCheckResult.BDU.s_Status =  FireTestCheckResult.s_BDUSQUIB & 0x40;

		FireTestCheckResult.s_ARMING = (g_TempBuffer[7] & 0x0F00)>> 8;
		FireTestCheckResult.s_INTARM = (g_TempBuffer[7] & 0x00F0);

		FireTestCheckResult.s_INTSQUIB = (g_TempBuffer[7] & 0x000F);
		FireTestCheckResult.INT.s_INTSQUIB1 = FireTestCheckResult.s_INTSQUIB & 0x01;
		FireTestCheckResult.INT.s_INTSQUIB2 = FireTestCheckResult.s_INTSQUIB & 0x02;
		FireTestCheckResult.INT.s_Status =  FireTestCheckResult.s_INTSQUIB & 0x04;
		break;
	}
}

void RecvDataSort(unsigned int Name, unsigned int Num)
{
	switch(Name)
	{
	case MSL:
		if(FireTestCheckResult.s_MSLPWR == 0x0001)	// 유도탄 전원 끔이면
		{
			FireStatus.s_MSL[Num] = OFF_1 ;
			printf("i ETC/Check_FAIL.jpg,179,70\r");
			(Num == FIRST) ? (PageValue.s_FireDetailResult_1[MSL_RESULT] = 1) : (PageValue.s_FireDetailResult_1[MSL_RESULT] = 1);	// 유도탄 번호가 1,2이면 수신 확인 값 변경
		}
		else if(FireTestCheckResult.s_MSLPWR == 0x0002)	// 유도탄 전원 켬이면
		{
			FireStatus.s_MSL[Num] = ON_2 ;
			printf("i ETC/Check_OK.jpg,179,70\r");
			(Num == FIRST) ? (PageValue.s_FireDetailResult_1[MSL_RESULT] = 1) : (PageValue.s_FireDetailResult_2[MSL_RESULT] = 1);	// 유도탄 번호가 1,2이면 수신 확인 값 변경
		}
		else // 유도탄 전원 해당 없으면
		{
			FireStatus.s_MSL[Num] = NONE_0 ;
			//printf("f   ,197,54\r");
			(Num == FIRST) ? (PageValue.s_FireDetailResult_1[MSL_RESULT] = 1) : (PageValue.s_FireDetailResult_2[MSL_RESULT] = 1);	// 유도탄 번호가 1,2이면 수신 확인 값 변경
		}

		break;

	case EXT:
		if(FireTestCheckResult.s_EXTPWR == 0x0001)	// 시험장치 전원 끔이면
		{
			FireStatus.s_EXT[Num] = OFF_1 ;
			printf("i ETC/Check_FAIL.jpg,179,70\r");
			(Num == FIRST) ? (PageValue.s_FireDetailResult_1[EXT_RESULT] = 1) : (PageValue.s_FireDetailResult_2[EXT_RESULT] = 1);	// 유도탄 번호가 1,2이면 수신 확인 값 변경
		}
		else if(FireTestCheckResult.s_EXTPWR == 0x0002)	// 시험장치 전원 켬이면
		{
			FireStatus.s_EXT[Num] = ON_2 ;
			printf("i ETC/Check_OK.jpg,179,70\r");
			(Num == FIRST) ? (PageValue.s_FireDetailResult_1[EXT_RESULT] = 1) : (PageValue.s_FireDetailResult_2[EXT_RESULT] = 1);	// 유도탄 번호가 1,2이면 수신 확인 값 변경
		}
		else // 시험장치 전원 해당 없으면
		{
			FireStatus.s_EXT[Num] = NONE_0 ;
			//printf("f   ,214,54\r");
			(Num == FIRST) ? (PageValue.s_FireDetailResult_1[EXT_RESULT] = 1) : (PageValue.s_FireDetailResult_2[EXT_RESULT] = 1);	// 유도탄 번호가 1,2이면 수신 확인 값 변경
		}

		break;

	case EB:
		FireStatus.s_EB[Num][EB_STATUS] = (FireTestCheckResult.EB.s_Status == 0x0010) ? GOOD_1 : FAIL;	// EB 점검여부가 점검완료
		if(FireStatus.s_EB[Num][EB_STATUS] == GOOD_1)
		{ // 점검이 완료 되었을경우 정상 비정상 판단함
			(Num == FIRST) ? (PageValue.s_FireDetailResult_1[EB_RESULT] = 1) : (PageValue.s_FireDetailResult_2[EB_RESULT] = 1);	// 유도탄 번호가 1,2이면 수신 확인 값 변경
			FireStatus.s_EB[Num][EB1_SQUIB1] = (FireTestCheckResult.EB.s_EB1SQUIB1 == 0x0001) ? GOOD_1 : FAIL;	// EB1 SQUIB1 이 정상이면
			if(FireStatus.s_EB[Num][EB1_SQUIB1] == GOOD_1) printf("i ETC/Check_OK.jpg,179,70\r");
			else printf("i ETC/Check_FAIL.jpg,179,70\r");

			FireStatus.s_EB[Num][EB1_SQUIB2] = (FireTestCheckResult.EB.s_EB1SQUIB2 == 0x0002) ? GOOD_1 : FAIL;	// EB1 SQUIB2 이 정상이면
			if(FireStatus.s_EB[Num][EB1_SQUIB2] == GOOD_1) printf("i ETC/Check_OK.jpg,179,94\r");
			else printf("i ETC/Check_FAIL.jpg,179,94\r");

			FireStatus.s_EB[Num][EB2_SQUIB1] = (FireTestCheckResult.EB.s_EB2SQUIB1 == 0x0004) ? GOOD_1 : FAIL;	// EB2 SQUIB1 이 정상이면
			if(FireStatus.s_EB[Num][EB2_SQUIB1] == GOOD_1) printf("i ETC/Check_OK.jpg,179,118\r");
			else printf("i ETC/Check_FAIL.jpg,179,118\r");

			FireStatus.s_EB[Num][EB2_SQUIB2] = (FireTestCheckResult.EB.s_EB2SQUIB2 == 0x0008) ? GOOD_1 : FAIL;	// EB2 SQUIB2 이 정상이면
			if(FireStatus.s_EB[Num][EB2_SQUIB2] == GOOD_1) printf("i ETC/Check_OK.jpg,179,142\r");
			else printf("i ETC/Check_FAIL.jpg,179,142\r");

		}
		else	// 미점검 일경우
		{
			FireStatus.s_EB[Num][EB1_SQUIB1] =  FAIL;	
			FireStatus.s_EB[Num][EB1_SQUIB2] =  FAIL;	
			FireStatus.s_EB[Num][EB2_SQUIB1] =  FAIL;	
			FireStatus.s_EB[Num][EB2_SQUIB2] =  FAIL;	
			(Num == FIRST) ? (PageValue.s_FireDetailResult_1[EB_RESULT] = 0) : (PageValue.s_FireDetailResult_2[EB_RESULT] = 0);	// 미점검 일경우 점검결과 체크 안함
		}
		break;

	case BAT:
		FireStatus.s_BAT[Num][BAT_STATUS] = (FireTestCheckResult.BAT.s_Status == 0x0010) ? GOOD_1 : FAIL;	// BAT 점검여부가 점검완료
		if(FireStatus.s_BAT[Num][BAT_STATUS] == GOOD_1)
		{ // 점검이 완료 되었을경우 정상 비정상 판단함
			(Num == FIRST) ? (PageValue.s_FireDetailResult_1[BAT_RESULT] = 1) : (PageValue.s_FireDetailResult_2[BAT_RESULT] = 1);	// 유도탄 번호가 1,2이면 수신 확인 값 변경
			FireStatus.s_BAT[Num][BAT1_SQUIB1] = (FireTestCheckResult.BAT.s_BAT1SQUIB1 == 0x0001) ? GOOD_1 : FAIL;	// BAT1_SQUIB1 이 정상이면
			if(FireStatus.s_BAT[Num][BAT1_SQUIB1] == GOOD_1) printf("i ETC/Check_OK.jpg,179,70\r");
			else printf("i ETC/Check_FAIL.jpg,179,70\r");

			FireStatus.s_BAT[Num][BAT1_SQUIB2] = (FireTestCheckResult.BAT.s_BAT1SQUIB2 == 0x0002) ? GOOD_1 : FAIL;	// BAT1_SQUIB2 이 정상이면
			if(FireStatus.s_BAT[Num][BAT1_SQUIB2] == GOOD_1) printf("i ETC/Check_OK.jpg,179,94\r");
			else printf("i ETC/Check_FAIL.jpg,179,94\r");

			FireStatus.s_BAT[Num][BAT2_SQUIB1] = (FireTestCheckResult.BAT.s_BAT2SQUIB1 == 0x0004) ? GOOD_1 : FAIL;	// BAT2_SQUIB1 이 정상이면
			if(FireStatus.s_BAT[Num][BAT2_SQUIB1] == GOOD_1) printf("i ETC/Check_OK.jpg,179,118\r");
			else printf("i ETC/Check_FAIL.jpg,179,118\r");

			FireStatus.s_BAT[Num][BAT2_SQUIB2] = (FireTestCheckResult.BAT.s_BAT2SQUIB2 == 0x0008) ? GOOD_1 : FAIL;	// BAT2_SQUIB2 이 정상이면
			if(FireStatus.s_BAT[Num][BAT2_SQUIB2] == GOOD_1) printf("i ETC/Check_OK.jpg,179,142\r");
			else printf("i ETC/Check_FAIL.jpg,179,142\r");

		}
		else	// 미점검 일경우
		{
			FireStatus.s_BAT[Num][BAT1_SQUIB1] =  FAIL;	
			FireStatus.s_BAT[Num][BAT1_SQUIB2] =  FAIL;	
			FireStatus.s_BAT[Num][BAT2_SQUIB1] =  FAIL;	
			FireStatus.s_BAT[Num][BAT2_SQUIB2] =  FAIL;	
			(Num == FIRST) ? (PageValue.s_FireDetailResult_1[BAT_RESULT] = 0) : (PageValue.s_FireDetailResult_2[BAT_RESULT] = 0);	// 미점검 일경우 점검결과 체크 안함
		}
		break;

	case ABAT:
		FireStatus.s_ABAT[Num][ABAT_STATUS] = (FireTestCheckResult.ABAT.s_Status == 0x0010) ? GOOD_1 : FAIL;	// ABAT 점검여부가 점검완료
		if(FireStatus.s_ABAT[Num][ABAT_STATUS] == GOOD_1){ // 점검이 완료 되었을경우 정상 비정상 판단함
			(Num == FIRST) ? (PageValue.s_FireDetailResult_1[ABAT_RESULT] = 1) : (PageValue.s_FireDetailResult_2[ABAT_RESULT] = 1);	// 유도탄 번호가 1,2이면 수신 확인 값 변경
			FireStatus.s_ABAT[Num][ABAT1_SQUIB1] = (FireTestCheckResult.ABAT.s_ABAT1SQUIB1 == 0x0001) ? GOOD_1 : FAIL;	// ABAT1_SQUIB1 이 정상이면
			if(FireStatus.s_ABAT[Num][ABAT1_SQUIB1] == GOOD_1) printf("i ETC/Check_OK.jpg,179,70\r");
			else printf("i ETC/Check_FAIL.jpg,179,70\r");

			FireStatus.s_ABAT[Num][ABAT1_SQUIB2] = (FireTestCheckResult.ABAT.s_ABAT1SQUIB2 == 0x0002) ? GOOD_1 : FAIL;	// ABAT1_SQUIB2 이 정상이면
			if(FireStatus.s_ABAT[Num][ABAT1_SQUIB2] == GOOD_1) printf("i ETC/Check_OK.jpg,179,94\r");
			else printf("i ETC/Check_FAIL.jpg,179,94\r");

			FireStatus.s_ABAT[Num][ABAT2_SQUIB1] = (FireTestCheckResult.ABAT.s_ABAT2SQUIB1 == 0x0004) ? GOOD_1 : FAIL;	// ABAT2_SQUIB1 이 정상이면
			if(FireStatus.s_ABAT[Num][ABAT2_SQUIB1] == GOOD_1) printf("i ETC/Check_OK.jpg,179,118\r");
			else printf("i ETC/Check_FAIL.jpg,179,118\r");

			FireStatus.s_ABAT[Num][ABAT2_SQUIB2] = (FireTestCheckResult.ABAT.s_ABAT2SQUIB2 == 0x0008) ? GOOD_1 : FAIL;	// ABAT2_SQUIB2 이 정상이면
			if(FireStatus.s_ABAT[Num][ABAT2_SQUIB2] == GOOD_1) printf("i ETC/Check_OK.jpg,179,142\r");
			else printf("i ETC/Check_FAIL.jpg,179,142\r");

		}
		else	// 미점검 일경우
		{
			FireStatus.s_ABAT[Num][ABAT1_SQUIB1] =  FAIL;	
			FireStatus.s_ABAT[Num][ABAT1_SQUIB2] =  FAIL;	
			FireStatus.s_ABAT[Num][ABAT2_SQUIB1] =  FAIL;	
			FireStatus.s_ABAT[Num][ABAT2_SQUIB2] =  FAIL;	
			(Num == FIRST) ? (PageValue.s_FireDetailResult_1[ABAT_RESULT] = 0) : (PageValue.s_FireDetailResult_2[ABAT_RESULT] = 0);	// 미점검 일경우 점검결과 체크 안함
		}
		break;

	case BDU:
		FireStatus.s_BDU[Num][BDU_STATUS] = (FireTestCheckResult.BDU.s_Status == 0x0004) ? GOOD_1 : FAIL;	// BDU 점검여부가 점검완료
		if(FireStatus.s_BDU[Num][BDU_STATUS] == GOOD_1)
		{ // 점검이 완료 되었을경우 정상 비정상 판단함
			(Num == FIRST) ? (PageValue.s_FireDetailResult_1[BDU_RESULT] = 1) : (PageValue.s_FireDetailResult_2[BDU_RESULT] = 1);	// 유도탄 번호가 1,2이면 수신 확인 값 변경
			FireStatus.s_BDU[Num][BDU_SQUIB1] = (FireTestCheckResult.BDU.s_BDUSQUIB1 == 0x0001) ? GOOD_1 : FAIL;	// BDU1_SQUIB1 이 정상이면
			if(FireStatus.s_BDU[Num][BDU_SQUIB1] == GOOD_1) printf("i ETC/Check_OK.jpg,179,70\r");
			else printf("i ETC/Check_FAIL.jpg,179,70\r");

			FireStatus.s_BDU[Num][BDU_SQUIB2] = (FireTestCheckResult.BDU.s_BDUSQUIB2 == 0x0002) ? GOOD_1 : FAIL;	// BDU1_SQUIB2 이 정상이면
			if(FireStatus.s_BDU[Num][BDU_SQUIB2] == GOOD_1) printf("i ETC/Check_OK.jpg,179,94\r");
			else printf("i ETC/Check_FAIL.jpg,179,94\r");


		}
		else	// 미점검 일경우
		{
			FireStatus.s_BDU[Num][BDU_SQUIB1] =  FAIL;	
			FireStatus.s_BDU[Num][BDU_SQUIB2] =  FAIL;	
			(Num == FIRST) ? (PageValue.s_FireDetailResult_1[BDU_RESULT] = 0) : (PageValue.s_FireDetailResult_2[BDU_RESULT] = 0);	// 미점검 일경우 점검결과 체크 안함
		}
		break;

	case ARMING:

		if(FireTestCheckResult.s_ARMING == 0x0001)	// 신관장전 비정상이면
		{
			FireStatus.s_ARMING[Num] = FAIL ;
			printf("i ETC/Check_FAIL.jpg,179,70\r");
			(Num == FIRST) ? (PageValue.s_FireDetailResult_1[ARMING_RESULT] = 1) : (PageValue.s_FireDetailResult_2[ARMING_RESULT] = 1);	// 유도탄 번호가 1,2이면 수신 확인 값 변경
		}
		else if(FireTestCheckResult.s_ARMING == 0x0002)	// 신관장전 정상이면
		{
			FireStatus.s_ARMING[Num] = GOOD_1 ;
			printf("i ETC/Check_OK.jpg,179,70\r");
			(Num == FIRST) ? (PageValue.s_FireDetailResult_1[ARMING_RESULT] = 1) : (PageValue.s_FireDetailResult_2[ARMING_RESULT] = 1);	// 유도탄 번호가 1,2이면 수신 확인 값 변경
		}
		else // 신관장전 해당 없으면
		{
			FireStatus.s_ARMING[Num] = NONE_0 ;
			(Num == FIRST) ? (PageValue.s_FireDetailResult_1[ARMING_RESULT] = 1) : (PageValue.s_FireDetailResult_2[ARMING_RESULT] = 1);	// 유도탄 번호가 1,2이면 수신 확인 값 변경
		}

		break;

	case INTARM:

		if(FireTestCheckResult.s_INTARM == 0x0001)	// 점화장전 비정상이면
		{
			FireStatus.s_INTARM[Num] = FAIL ;
			printf("i ETC/Check_FAIL.jpg,177,73\r");
			(Num == FIRST) ? (PageValue.s_FireDetailResult_1[INTARM_RESULT] = 1) : (PageValue.s_FireDetailResult_2[INTARM_RESULT] = 1);	// 유도탄 번호가 1,2이면 수신 확인 값 변경
		}
		else if(FireTestCheckResult.s_INTARM == 0x0002)	// 점화장전 정상이면
		{
			FireStatus.s_INTARM[Num] = GOOD_1 ;
			printf("i ETC/Check_OK.jpg,177,73\r");
			(Num == FIRST) ? (PageValue.s_FireDetailResult_1[INTARM_RESULT] = 1) : (PageValue.s_FireDetailResult_2[INTARM_RESULT] = 1);	// 유도탄 번호가 1,2이면 수신 확인 값 변경
		}
		else // 점화장전 해당 없으면
		{
			FireStatus.s_INTARM[Num] = NONE_0 ;
			(Num == FIRST) ? (PageValue.s_FireDetailResult_1[INTARM_RESULT] = 1) : (PageValue.s_FireDetailResult_2[INTARM_RESULT] = 1);	// 유도탄 번호가 1,2이면 수신 확인 값 변경
		}

		break;

	case INT:
		FireStatus.s_INT[Num][INT_STATUS] = (FireTestCheckResult.INT.s_Status == 0x0004) ? GOOD_1 : FAIL;	// INT 점검여부가 점검완료
		if(FireStatus.s_INT[Num][INT_STATUS] == GOOD_1)
		{ // 점검이 완료 되었을경우 정상 비정상 판단함
			(Num == FIRST) ? (PageValue.s_FireDetailResult_1[INT_RESULT] = 1) : (PageValue.s_FireDetailResult_2[INT_RESULT] = 1);	// 유도탄 번호가 1,2이면 수신 확인 값 변경
			FireStatus.s_INT[Num][INT_SQUIB1] = (FireTestCheckResult.INT.s_INTSQUIB1 == 0x0001) ? GOOD_1 : FAIL;	// INT1_SQUIB1 이 정상이면
			if(FireStatus.s_INT[Num][INT_SQUIB1] == GOOD_1) printf("i ETC/Check_OK.jpg,179,70\r");
			else printf("i ETC/Check_FAIL.jpg,179,70\r");

			FireStatus.s_INT[Num][INT_SQUIB2] = (FireTestCheckResult.INT.s_INTSQUIB2 == 0x0002) ? GOOD_1 : FAIL;	// INT1_SQUIB2 이 정상이면
			if(FireStatus.s_INT[Num][INT_SQUIB2] == GOOD_1) printf("i ETC/Check_OK.jpg,179,94\r");
			else printf("i ETC/Check_FAIL.jpg,179,94\r");
		}
		else	// 미점검 일경우
		{
			FireStatus.s_INT[Num][INT_SQUIB1] =  FAIL;	
			FireStatus.s_INT[Num][INT_SQUIB2] =  FAIL;	
			(Num == FIRST) ? (PageValue.s_FireDetailResult_1[INT_RESULT] = 0) : (PageValue.s_FireDetailResult_2[INT_RESULT] = 0);	// 미점검 일경우 점검결과 체크 안함
		}
		break;
	}
}

void InitXHyper255A(void)
{
//   ADDR32(GPIO_BASE+GPDR2) |= GPDR2_VALUE; //~0x0;//((ADDR32(GPIO_BASE+GPDR2) & 0x0001FFFF) | GPDR2_VALUE);
//   ADDR32(GPIO_BASE+GAFR2_L) |= GAFR2L_VALUE;
//   ADDR32(MEM_CTL_BASE+MSC1) = 0x00008259;//0x000095F8;
}
void     loopback_udp(SOCKET s, uint16 port, uint8* buf, uint16 mode)
{
   uint32 len;
   uint8 destip[4];
   uint16 destport;
   
   switch(getSn_SSR(s))
   {
                                                         // -------------------------------
      case SOCK_UDP:                                     // 
         if((len=getSn_RX_RSR(s)) > 0)                   // check the size of received data
         {
            len = recvfrom(s,buf,len,destip,&destport);  // receive data from a destination
            if(len !=sendto(s,buf,len,destip,destport))  // send the data to the destination
            {
               printf("%d : Sendto Fail.len=%d,",s,len);
               printf("%d.%d.%d.%d(%d)\r\n",destip[0],destip[1],destip[2],destip[3],destport);
            }
         }
         break;
                                                         // -----------------
      case SOCK_CLOSED:                                  // CLOSED
         close(s);                                       // close the SOCKET
         socket(s,Sn_MR_UDP,port,mode);                  // open the SOCKET with UDP mode
         break;
      default:
         break;
   }
}
void W5300_Setting()
{
   uint8 tx_mem_conf[8] = {8,8,8,8,8,8,8,8};          // for setting TMSR regsiter
   uint8 rx_mem_conf[8] = {8,8,8,8,8,8,8,8};          // for setting RMSR regsiter
   
   uint8 * data_buf = (uint8 *) 0xA10E0000;         // buffer for loopack data
   
   uint8 ip[4] = {192,168,111,200};                   // for setting SIP register
   uint8 gw[4] = {192,168,111,1};                     // for setting GAR register
   uint8 sn[4] = {255,255,255,0};                     // for setting SUBR register
   uint8 mac[6] = {0x00,0x08,0xDC,0x00,111,200};      // for setting SHAR register
   
   uint8 serverip[4] = {192,168,111,78};              // "TCP SERVER" IP address for loopback_tcpc()
   
   
   data_buf = (uint8*)0xA10F0000;                       
   
   InitXHyper255A();                                  // initiate MCU
   
   /* initiate W5300 */
   iinchip_init();  

   /* allocate internal TX/RX Memory of W5300 */
   if(!sysinit(tx_mem_conf,rx_mem_conf))           
   {
      printf("MEMORY CONFIG ERR.\r\n");
      while(1);
   }

   //setMR(getMR()|MR_FS);                            // If Little-endian, set MR_FS.
   
   setSHAR(mac);                                      // set source hardware address
   
   #ifdef __DEF_IINCHIP_PPP__
      if(pppinit((uint8*)"test01", 6, (uint8*)"pppoe1000", 9)!=1)
      {
         printf("PPPoE fail.\r\n");
         while(1);
      }
      close(0);
   #else
      /* configure network information */
      setGAR(gw);                                     // set gateway IP address
      setSUBR(sn);                                    // set subnet mask address
      setSIPR(ip);                                    // set source IP address
   #endif
   
   /* verify network information */
   getSHAR(mac);                                      // get source hardware address 
   getGAR(gw);                                        // get gateway IP address      
   getSUBR(sn);                                       // get subnet mask address     
   getSIPR(ip);                                       // get source IP address       
   
   printf("SHAR : %02x:%02x:%02x:%02x:%02x:%02x\r\n",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
   printf("GWR  : %d.%d.%d.%d\r\n",gw[0],gw[1],gw[2],gw[3]);
   printf("SUBR : %d.%d.%d.%d\r\n",sn[0],sn[1],sn[2],sn[3]);
   printf("SIPR : %d.%d.%d.%d\r\n",ip[0],ip[1],ip[2],ip[3]);
    while(1)
   {
      loopback_udp(7,3000,data_buf,0);
   }

   #ifdef __DEF_IINCHIP_PPP__
   {
      uint8 ppp_mac[6];
      getPDHAR(ppp_mac);
      pppterm(ppp_mac, getPSIDR());
   }
   #endif

   while(1);
}
int main(void)
{
	InitGPIO();          // gpio 초기화
	InitValue();           // 변수 초기화
	InitUart1();           // uart 초기화
	InitSpi1();            // spi 초기화
    InitSpi2();             // spi 초기화
	InitAdc();
	InitRelay();            // 릴레이 포트 초기화
	InitTimer1();
	InitTimer2();
	InnerVoltTest();      // 내부 전압 점검
	InitInterrupt();       // 인터럽트 초기화
    W5300_Setting();        // w5300 초기화
	IFS1bits.INT1IF = 0;		// INT1 Interrupt Flag Clear
	IFS1bits.INT2IF = 0;		// INT2 Interrupt Flag Clear
	IFS0bits.T1IF = 0;			// T1 Interrupt Flag Clear
	IFS0bits.T2IF = 0;			// T2 Interrupt Flag Clear


	if( PageValue.s_First[VOLT_ERR] == 0 )
	{
		while(1)
		{
			if(PageValue.s_Check[CHK_VOLT] == 1)   MenuDisplay();// 화면 출력 부분

		}
	}
	return(0);
}
