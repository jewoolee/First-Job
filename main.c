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
#include "lib\init_drv.h"
#include "lib\data_define.h"

//Configuration Bits 설정
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


// 인터럽트 선언 부************************************************
void __attribute__((interrupt, auto_psv)) _U1RXInterrupt(void);
void __attribute__((interrupt, auto_psv)) _T1Interrupt(void);
void __attribute__((interrupt, auto_psv)) _T2Interrupt(void);
//****************************************************************
// <editor-fold defaultstate="collapsed" desc="열거형선언">
enum { MODE_SEL, VOLT_ERR}; // 2 모드 화면, 에러 화면
enum { CHK_OK, CHK_NO, CHK_RESULT, CHK_HINT, CHK_HINT_1, CHK_LAN, CHK_VOLT, CHK_BIT}; // 7
enum { NONE, SELF, LINE, FIRE1, FIRE2 };
//enum { SELF_RES_RESULT, SELF_SHO_RESULT, SELF_INS_RESULT, LINE_RES_RESULT, LINE_SHO_RESULT, LINE_INS_RESULT } ;
enum { SELF_TEST, SELF_RES_TEST, SELF_SHO_TEST, SELF_INS_TEST}; // 5
enum { SELF_RES_ING, SELF_SHO_ING, SELF_INS_ING, SELF_RES_RESULT, SELF_SHO_RESULT, SELF_INS_RESULT}; // 6
enum { LINE_RES_ING, LINE_SHO_ING, LINE_INS_ING, LINE_RES_RESULT, LINE_SHO_RESULT, LINE_INS_RESULT}; // 6
enum { LINE_TEST, LINE_RES_TEST, LINE_SHO_TEST, LINE_INS_TEST}; // 5
enum { FIRE_TEST, FIRE_TEST_EACH_1, FIRE_TEST_TOTAL_1, FIRE_TEST_EACH_2, FIRE_TEST_TOTAL_2 }; // 6
enum { MSL_RESULT, EXT_RESULT, EB_RESULT, BAT_RESULT, ABAT_RESULT, BDU_RESULT, ARMING_RESULT, INTARM_RESULT, INT_RESULT}; //9
enum { RESULT,DC_VREF, DC5V, DC12V_PLUS, DC12V_MINUS};//enum { RESULT,DC_VREF, DC3V, DC5V, DC12V_PLUS, DC12V_MINUS};
enum { A0, A1, A2, CLEAR}; // 어드레스 제어
enum { SEL_ADC, SEL_LED, SEL_DATA, SEL_ADDRESS, SEL_RELAY01, SEL_LEDADC};   // CS 선택
enum { RESIDUAL, SHORT, INSULATION};    // 잔류, 도통/단선, 절연
enum { RESIDUAL_RELAY, INSUL_RELAY, OK_LED, INSUL_LED, INTEGRATION_LED, SP_LED, SHORT_LED, RESIDUAL_VOLT_LED};
// </editor-fold>
// 전역 변수 선언************************
unsigned char g_RxBuffer[100];
unsigned char g_RxData;
unsigned char g_Buffer;
unsigned char g_UartFunCtionButton;
unsigned int g_AdcBuffer[5];
unsigned int g_AdcCount=0;
unsigned int g_RelayLedStatus=0;
unsigned long int g_AdcSample[5];
unsigned int g_RelayChannel = 0;
unsigned int g_InputValue=0 ; // 입력값 저장 변수
unsigned int g_TimerCount=0;
unsigned int g_OkButtonTimerCount =0;
unsigned int g_OkButtonTimerCountPre =0;
unsigned int g_TimerCounter =0;
int g_Sw[7]={0};
int d_Sw[5]={0};
int g_SwSum =0;


int g_MenuPage;
int b=0;
const float g_Error[3] = { 0.05f,  0.4f, 1.5f};//const float g_Error[4] = { 0.05f, 0.2f, 0.4f, 1.5f};
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
	ButtonValue.s_NextButton == 0 ? printf("R 80,240,104,264,255,255,255,1\r"):NULL;    // 세부 점검 결과 화면에서 숫자 버튼 못쓰도록
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
		printf("f int플래그 %d,999,999\r", ButtonValue.s_BackButton);   
		printf("f int포트 %d,999,999\r", PORTEbits.RE4);   
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
	g_Sw[0] =  PORTEbits.RE0;    // 1(확인)
	g_Sw[1] =  PORTEbits.RE1;    // 2(도통)
	g_Sw[2] =  PORTEbits.RE2;    // 3(통합)
	g_Sw[3] =  PORTEbits.RE3;    // ok(빈칸)
	g_Sw[4] =  PORTEbits.RE4;    // 이전(절연)
	g_Sw[5] =  PORTEbits.RE5;    // >(잔류)
}
void ButtonPrintValue(void) // 버튼입력값 화면 출력 SCI 연동
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
void InnerVoltTest(void)      // 내부 전원 점검 (완료 vref. 5v, +15v, -15v 정상값 확인 완료)
{
	unsigned char n;
	unsigned int *pADC_buf16;
	printf("R 0,0,480,272,255,255,255,1\r"); // 화면 초기화
	printf("i TESTING/InnerVoltTesting.png,0,0\r");  // 내부 전압 점검중 화면 출력
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
	//		{     // ((Vref 샘플값 >= (4.096f-에러율)) && (Vref 샘플값 <= (4.096f+에러율))
	//			SET_BIT(SelfTest.s_Result,0);   // 정상이면 결과값의 0비트 1로 변경
	//		}
	//		else CLEAR_BIT(SelfTest.s_Result,0);    // 비정상이면 결과값의 0비트를 0으로 변경
	//
	//		if((SelfTest.s_Dc3v >= (3.0f-g_Error[1]))&&(SelfTest.s_Dc3v <= (3.0f+g_Error[1])))
	//		{
	//			SET_BIT(SelfTest.s_Result,1);// 정상이면 결과값의 1비트 1로 변경
	//		}
	//            else CLEAR_BIT(SelfTest.s_Result,1);// 비정상이면 결과값의 1비트를 0으로 변경
	//
	//		if((SelfTest.s_Dc5v >= (5.0f-g_Error[2]))&&(SelfTest.s_Dc5v <= (5.0f+g_Error[2])))
	//		{
	//			SET_BIT(SelfTest.s_Result,2);// 정상이면 결과값의 2비트 1로 변경
	//		}
	//		else CLEAR_BIT(SelfTest.s_Result,2);// 비정상이면 결과값의 2비트를 0으로 변경
	//
	//		if((SelfTest.s_Dc12vPlus >= (15.0f-g_Error[3]))&&(SelfTest.s_Dc12vPlus <= (15.f+g_Error[3])))
	//		{
	//			SET_BIT(SelfTest.s_Result,3);// 정상이면 결과값의 3비트 1로 변경
	//		}
	//		else CLEAR_BIT(SelfTest.s_Result,3);// 비정상이면 결과값의 3비트를 0으로 변경
	//
	//		if((SelfTest.s_Dc12vMinus >= (-15.0f-g_Error[3]))&&(SelfTest.s_Dc12vMinus <= (-15.0f+g_Error[3])))
	//		{
	//			SET_BIT(SelfTest.s_Result,4);// 정상이면 결과값의 4비트 1로 변경
	//		}
	//		else CLEAR_BIT(SelfTest.s_Result,4);// 비정상이면 결과값의 4비트를 0으로 변경

	if(SelfTest.s_Result == 0x0F) // 결과값 모두 정상이면 동작
	{
		printf("i MENU/ModeMenu.png,0,0\r");
		PageValue.s_First[MODE_SEL] = 1;          // 모드 선택 화면 토글

		printf("i ETC/VoltGood.png,0,0\r");
		PageValue.s_Check[CHK_VOLT] = 1;            // 내부 전압 정상 알람 토글
		if(PORTEbits.RE6 == 0)// 자체 점검 플러그 인식 확인(자체점검플러그 미 연결시 =1, 자체점검플러그 연결시 = 0)
		{
			printf("i ETC/BitCheck.png,325,0\r");       
			PageValue.s_Check[CHK_BIT] = 1;             // 자체 점검 가능 알람 토글
		}
	}
	else           // 내부 전압 비정상시
	{
		printf("i ERROR/InnerVoltFail.png,0,0\r");  // 내부 전압 비정상
		PageValue.s_First[VOLT_ERR] = 1;              // 내부 전압 비정상 토글
		SelfTest.s_Result = 0; // 결과값 초기화
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

	memset(&PageValue, 0, sizeof(struct PAGEVALUE));    // 화면페이지 변수 초기화
	memset(&ButtonValue, 0, sizeof(struct BUTTONVALUE));    // 버튼선택값 변수 초기화
	memset(&Adc7980Value, 0, sizeof(struct AD7980VALUE));    // ADC값 변수 초기화
	memset(&SelfTest, 0, sizeof(struct SELFTESTTYPE));    // 내부 전원점검 변수 초기화

}
void TestResultCheck(unsigned int Label)
{
	unsigned char n=0;        // 힌트 출력 비교
	switch(Label)
	{
	case 1 : // 잔류, 도통/단선, 절연 점검 체크
		printf("i MENU/SelfTestMenu.png,0,0\r"); // 자체 점검 메뉴 출력
		AlramPrint();
		if(PageValue.s_Self[SELF_RES_TEST]==1){ printf("i ETC/Check.png,233,73\r"); printf("i ETC/Check_.png,260,75\r"); n++;}
		if(PageValue.s_Self[SELF_SHO_TEST]==1){ printf("i ETC/Check.png,233,96\r"); printf("i ETC/Check_.png,260,97\r"); n++;}
		if(PageValue.s_Self[SELF_INS_TEST]==1){ printf("i ETC/Check.png,233,118\r"); printf("i ETC/Check_.png,260,120\r"); n++;}
		if(n>0) printf("i ETC/Hint.png,0,205\r");     // 힌트 출력

		break;
	case 2 : // 잔류, 도통/단선, 절연 점검 체크
		printf("i MENU/LineTestMenu.png,0,0\r"); // 점화 계통 점검 메뉴 출력
		AlramPrint();
		if(PageValue.s_Line[LINE_RES_TEST]==1){ printf("i ETC/Check.png,233,73\r"); printf("i ETC/Check_.png,260,75\r"); n++;}
		if(PageValue.s_Line[LINE_SHO_TEST]==1){ printf("i ETC/Check.png,233,96\r"); printf("i ETC/Check_.png,260,97\r"); n++;}
		if(PageValue.s_Line[LINE_INS_TEST]==1){ printf("i ETC/Check.png,233,118\r"); printf("i ETC/Check_.png,260,120\r"); n++;}
		if(n>0) printf("i ETC/Hint.png,0,205\r");     // 힌트 출력
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
		if(n>0) printf("i ETC/Hint_1.png,240,175\r");     // 힌트 출력
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
		if(n>0) printf("i ETC/Hint_1.png,240,175\r");     // 힌트 출력
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
	PORTG = ChipSelectParam[SEL_DATA];//PORTG = 0x0044; // CS4, CS3, CS2 DATA 선택
	PORTG = ChipSelectParam[SEL_ADC];// PORTG = 0x0004; // CS3, CS2     DATA 초기화

	PORTD = RelaySelectParam[Add];//PORTD = 0x0008;// A3 - H -> 릴레이 1번 제어            
	PORTG = ChipSelectParam[SEL_ADDRESS];//PORTG = 0x0006;// CS3, CS2, CS1    ADDRESS 선택
	PORTG = ChipSelectParam[SEL_ADC];// PORTG = 0x0004; // CS3, CS2         ADDRESS 초기화

	PORTD = RelaySelectParam[CLEAR];//PORTD = 0x000f; //A0 - H, A1 - H, A2 - H, A3 - H  -> 릴레이 동작 해제
	PORTG = ChipSelectParam[SEL_ADDRESS];//PORTG = 0x0006; // CS3, CS2, CS1    ADDRESS 선택
	PORTG = ChipSelectParam[SEL_ADC];//PORTG = 0x0004; // CS3, CS2         ADDRESS 초기화
	ReadADCValue(50);
	g_RelayChannel++;
}
void InitRelay(void)
{
	PORTD = 0x0000;                 // DO 0~7번 사용 안할경우 0으로 초기화
	PORTG = ChipSelectParam[SEL_ADC];
} 
void AlramPrint()
{
	if(PageValue.s_Check[CHK_VOLT] == 1) printf("i ETC/VoltGood.png,0,0\r");        // 내부 전원 정상시 알람 출력
	if(PageValue.s_Check[CHK_BIT] == 1) printf("i ETC/BitCheck.png,325,0\r");        // 자체 점검 가능 알람 출력
	if(PageValue.s_Check[CHK_LAN] == 1) printf("i ETC/LanConnect.png,165,0\r");        // LAN 연결상태 확인 알람
}
void RelayLedControl(unsigned char Ch, unsigned char On_Off)
{
	// RESIDUAL_RELAY - 릴레이 0번 사용 , INSUL_RELAY - 릴레이 1번 사용
	if(On_Off == ON)
		SET_BIT(g_RelayLedStatus, Ch);
	else
		CLEAR_BIT(g_RelayLedStatus, Ch);
	PORTD = g_RelayLedStatus;
	PORTG = ChipSelectParam[SEL_LEDADC];   //  ADC(CS2) + LED(CS0)
	PORTG = ChipSelectParam[SEL_ADC];   // ADC(CS2)
}
void ResidualVoltTest(void)     // 잔류 전압 점검(완료  ADC값 검증 필요)FORI0번 건너 뛰고 비교해야됨
{
	int fori;
	InitRelay();    // 릴레이 초기화
	
	IEC1bits.INT1IE = 0;	// 인터럽트 1중지
	IEC1bits.INT2IE = 0;    // 인터럽트 2중지
	RelayLedControl(RESIDUAL_RELAY,ON); // ON = 1 릴레이 0번
	RelayLedControl(INSUL_RELAY,OFF);   // OFF = 0 릴레이 1번
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
void InsulationTest(void)       // 절연 점검(완료 ADC값 검증 필요)FORI0번 건너 뛰고 비교해야됨
{
	int fori;
	InitRelay();    // 릴레이 초기화
	IEC1bits.INT1IE = 0;	
	IEC1bits.INT2IE = 0;	
	RelayLedControl(RESIDUAL_RELAY,OFF); // OFF = 0 릴레이 0번
	RelayLedControl(INSUL_RELAY,ON);   // ON = 1 릴레이 1번

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
void ShortTest(void)            // 도통단선점검(완료 ADC값 검증 필요)FORI0번 건너 뛰고 비교해야됨
{
	int fori;
	InitRelay();    // 릴레이 초기화
	IEC1bits.INT1IE = 0;	
	IEC1bits.INT2IE = 0;	
	RelayLedControl(RESIDUAL_RELAY,OFF); // OFF = 0 릴레이 0번
	RelayLedControl(INSUL_RELAY,OFF);   // OFF = 0 릴레이 1번
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
unsigned int ReadADC7980(void)
{
	unsigned int data;
	data = 0;

	PORTG = ChipSelectParam[SEL_ADC]; // RG9 제어 및 LED 동작 RG0,1
	SPI1BUF = 0x00;
	while(SPI1STATbits.SPITBF);
	while(!SPI1STATbits.SPIRBF);
	data = SPI1BUF;
	PORTG = ChipSelectParam[SEL_LED]; // RG9 제어 및 LED 동작 RG0,1

	return data;
}
void ReadADCValue(unsigned int AverageCount)
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
        // <editor-fold defaultstate="collapsed" desc="자체 점검 선택">
        if(ButtonValue.s_OkButton == 1)
        {
            
                switch(g_InputValue)
                {
                    // <editor-fold defaultstate="collapsed" desc="점검">
                case 1:
                    printf("i TESTING/ResidualTesting.png,0,0\r");
                    AlramPrint();
                    ResidualVoltTest(); 
                    PageValue.s_Self[SELF_RES_TEST] = 1;  // 자체 점검 완료 플래그
                    printf("i MENU/SelfTestMenu.png,0,0\r"); // 점화계통 점검 메뉴 출력
                    AlramPrint();
                    TestResultCheck(1);                  // 점검 결과 표시
                    // 점검 완료시 버튼 플래그 초기화
                    ButtonValue.s_OkButton = 0; // 점검이 완료 되어서 
                    g_MenuPage =1;// g_MenuPage == 0 ? 1:0;   //1
                    g_InputValue = 0;
                    break;
                case 2:
                    printf("i TESTING/ShortTesting.png,0,0\r");
                    AlramPrint();
                    ShortTest(); 
                    PageValue.s_Self[SELF_SHO_TEST] = 1;  // 도통/단선 점검
                    printf("i MENU/SelfTestMenu.png,0,0\r"); // 점화계통 점검 메뉴 출력
                    AlramPrint();
                    TestResultCheck(1);                  // 점검 결과 표시
                     ButtonValue.s_OkButton = 0;       // 확인 버튼 초기화
                    ButtonValue.s_BackButton = 0;
                    ButtonValue.s_NextButton = 0;
                    g_MenuPage =1;// g_MenuPage == 0 ? 1:0;   //1
                    g_InputValue = 0;
                    break;
                case 3:
                    printf("i TESTING/InsulTesting.png,0,0\r");
                    AlramPrint();    
                   InsulationTest();
                    PageValue.s_Self[SELF_INS_TEST] = 1;  // 절연 점검
                     printf("i MENU/SelfTestMenu.png,0,0\r"); // 점화계통 점검 메뉴 출력
                    AlramPrint();
                     TestResultCheck(1);                  // 점검 결과 표시
                    ButtonValue.s_OkButton = 0;       // 확인 버튼 초기화
                    ButtonValue.s_BackButton = 0;
                    ButtonValue.s_NextButton = 0;
                    g_MenuPage =1;// g_MenuPage == 0 ? 1:0;   //1
                    g_InputValue = 0;
                    break;
                case 4:
                     
                    printf("i TESTING/ResidualTesting.png,0,0\r");
                    AlramPrint();
                    ResidualVoltTest(); 
                     PageValue.s_Self[SELF_RES_TEST] = 1;  // 자체 점검
                   
                    printf("i TESTING/ShortTesting.png,0,0\r");
                    AlramPrint();
                    ShortTest(); 
                     PageValue.s_Self[SELF_SHO_TEST] = 1;  // 도통/단선 점검
                   
                    printf("i TESTING/InsulTesting.png,0,0\r");
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
                    printf("i MENU/ModeMenu.png,0,0\r");    // 모드 선택 화면 출력
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
                    printf("i TESTING/ResidualTesting.png,0,0\r");
                    AlramPrint();
                    ResidualVoltTest(); 
                    PageValue.s_Line[LINE_RES_TEST] = 1;  // 자체 점검 완료 플래그
                    printf("i MENU/LineTestMenu.png,0,0\r"); // 자체 점검 메뉴 출력
                    AlramPrint();
                    TestResultCheck(2);                  // 점검 결과 표시
                    // 점검 완료시 버튼 플래그 초기화
                    ButtonValue.s_OkButton = 0; // 점검이 완료 되어서 
                    g_MenuPage =1;// g_MenuPage == 0 ? 1:0;   //1
                    g_InputValue = 0;
                    break;
                case 2:
                    printf("i TESTING/ShortTesting.png,0,0\r");
                    AlramPrint();
                    ShortTest(); 
                    PageValue.s_Line[LINE_SHO_TEST] = 1;  // 도통/단선 점검
                    printf("i MENU/LineTestMenu.png,0,0\r"); // 자체 점검 메뉴 출력
                    AlramPrint();
                    TestResultCheck(2);                  // 점검 결과 표시
                     ButtonValue.s_OkButton = 0;       // 확인 버튼 초기화
                    ButtonValue.s_BackButton = 0;
                    ButtonValue.s_NextButton = 0;
                    g_MenuPage =1;// g_MenuPage == 0 ? 1:0;   //1
                    g_InputValue = 0;
                    break;
                case 3:
                    printf("i TESTING/InsulTesting.png,0,0\r");
                    AlramPrint();    
                   InsulationTest();
                    PageValue.s_Line[LINE_INS_TEST] = 1;  // 절연 점검
                     printf("i MENU/LineTestMenu.png,0,0\r"); // 자체 점검 메뉴 출력
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
                    printf("i TESTING/ResidualTesting.png,0,0\r");
                    AlramPrint();
                    ResidualVoltTest(); 
                    PageValue.s_Line[LINE_SHO_TEST] = 1;  // 도통/단선 점검
                    printf("i TESTING/ShortTesting.png,0,0\r");
                    AlramPrint();
                    ShortTest(); 
                    PageValue.s_Line[LINE_INS_TEST] = 1;  // 절연 점검
                    printf("i TESTING/InsulTesting.png,0,0\r");
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
                    printf("i MENU/ModeMenu.png,0,0\r");    // 모드 선택 화면 출력
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
				printf("i MENU/FireEachTestMenu_1.png,0,0\r");                              // 1호탄 개별 점검
				TestResultCheck(3);             // 점검 결과 체크 표시
				PageValue.s_Fire[FIRE_TEST_EACH_1] = 1;
				PageValue.s_Fire[FIRE_TEST] = 0;
				ButtonValue.s_OkButton = 0;       // 확인 버튼 초기화
				g_InputValue = 0;
				break;
			case  0x02 :
				printf("i MENU/FireTotalTestMenu_1.png,0,0\r");                              // 1호탄 통합 점검
				TestResultCheck(3);             // 점검 결과 체크 표시
				PageValue.s_Fire[FIRE_TEST_TOTAL_1] = 1;
				PageValue.s_Fire[FIRE_TEST] = 0;
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				break;
			case  0x03 :
				printf("i MENU/FireEachTestMenu_2.png,0,0\r");                              // 2호탄 개별 점검
				TestResultCheck(4);             // 점검 결과 체크 표시
				PageValue.s_Fire[FIRE_TEST_EACH_2] = 1;
				PageValue.s_Fire[FIRE_TEST] = 0;
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				break;
			case  0x04 :
				printf("i MENU/FireTotalTestMenu_2.png,0,0\r");                              // 2호탄 통합 점검
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
			printf("i MENU/ModeMenu.png,0,0\r");        // 모드 선택 화면 출력
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
				PageValue.s_FireRecvResult_1[MSL_RESULT] = 1; // 유도탄 전원 수신 확인 
				TestResultCheck(3);         // 점검 결과 표시
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 2:
				PageValue.s_FireRecvResult_1[EXT_RESULT] = 1; // 유도탄 전원 수신 확인 
				TestResultCheck(3);
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 3:
				
				PageValue.s_FireRecvResult_1[EB_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글
                TestResultCheck(3);
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 4:
				PageValue.s_FireRecvResult_1[BAT_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글
                TestResultCheck(3);
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 5:
				PageValue.s_FireRecvResult_1[ABAT_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글
                TestResultCheck(3);
				ButtonValue.s_OkButton = 0; 
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 6:
				PageValue.s_FireRecvResult_1[BDU_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글
                TestResultCheck(3);
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 7:
				PageValue.s_FireRecvResult_1[ARMING_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글
                TestResultCheck(3);
				ButtonValue.s_OkButton = 0; 
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 8:
				PageValue.s_FireRecvResult_1[INTARM_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글
                TestResultCheck(3);
				ButtonValue.s_OkButton = 0; 
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 9:
				PageValue.s_FireRecvResult_1[INT_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글
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
            // <editor-fold defaultstate="collapsed" desc="1호탄 개별 점검 세부 결과">
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
				PageValue.s_FireDetailResult_1[EXT_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;

			case 3:
				printf("i Result/EBSQB.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_1[EB_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;
			case 4:
				printf("i Result/BATSQB.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_1[BAT_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;

			case 5:
				printf("i Result/ABATSQB.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_1[ABAT_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글 
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;
			case 6:
				printf("i Result/BDUSQB.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_1[BDU_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;

			case 7:
				printf("i Result/ARMING.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_1[ARMING_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;
			case 8:
				printf("i Result/INTARMING.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_1[INTARM_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;

			case 9:
				printf("i Result/INTSQB.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_1[INT_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글
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
                	printf("i MENU/FireEachTestMenu_1.png,0,0\r");  
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
        // <editor-fold defaultstate="collapsed" desc="1호탄 통합 점검">
		if(ButtonValue.s_OkButton == 1)
		{
            // <editor-fold defaultstate="collapsed" desc="1호탄 통합 점검">
			switch(g_InputValue)
			{
			case 1:
				PageValue.s_FireRecvResult_1[MSL_RESULT] = 1; // 유도탄 전원 수신 확인 
				TestResultCheck(3);         // 점검 결과 표시
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 2:
				PageValue.s_FireRecvResult_1[EXT_RESULT] = 1; // 유도탄 전원 수신 확인 
				TestResultCheck(3);
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 3:
				
				PageValue.s_FireRecvResult_1[EB_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글
                TestResultCheck(3);
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 4:
				PageValue.s_FireRecvResult_1[BAT_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글
                TestResultCheck(3);
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 5:
				PageValue.s_FireRecvResult_1[ABAT_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글
                TestResultCheck(3);
				ButtonValue.s_OkButton = 0; 
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 6:
				PageValue.s_FireRecvResult_1[BDU_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글
                TestResultCheck(3);
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 7:
				PageValue.s_FireRecvResult_1[ARMING_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글
                TestResultCheck(3);
				ButtonValue.s_OkButton = 0; 
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 8:
				PageValue.s_FireRecvResult_1[INTARM_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글
                TestResultCheck(3);
				ButtonValue.s_OkButton = 0; 
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 9:
				PageValue.s_FireRecvResult_1[INT_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글
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
            // <editor-fold defaultstate="collapsed" desc="1호탄 통합 점검 세부 결과">
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
				PageValue.s_FireDetailResult_1[EXT_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;

			case 3:
				printf("i Result/EBSQB.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_1[EB_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;
			case 4:
				printf("i Result/BATSQB.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_1[BAT_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;

			case 5:
				printf("i Result/ABATSQB.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_1[ABAT_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글 
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;
			case 6:
				printf("i Result/BDUSQB.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_1[BDU_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;

			case 7:
				printf("i Result/ARMING.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_1[ARMING_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;
			case 8:
				printf("i Result/INTARMING.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_1[INTARM_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;

			case 9:
				printf("i Result/INTSQB.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_1[INT_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글
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
                printf("i MENU/FireTotalTestMenu_1.png,0,0\r");         
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
    
    
    else if(PageValue.s_Fire[FIRE_TEST_EACH_2]==1) //&& (PageValue.s_Check[CHK_LAN])) // 1호탄 개별 점검 개별 메시지 전송 및 수신
	{
        // <editor-fold defaultstate="collapsed" desc="2호탄 개별 점검">
	if(ButtonValue.s_OkButton == 1)
		{
            // <editor-fold defaultstate="collapsed" desc="2호탄 개별 점검">
			switch(g_InputValue)
			{
			case 1:
				PageValue.s_FireRecvResult_2[MSL_RESULT] = 1; // 유도탄 전원 수신 확인 
				TestResultCheck(4);         // 점검 결과 표시
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 2:
				PageValue.s_FireRecvResult_2[EXT_RESULT] = 1; // 유도탄 전원 수신 확인 
				TestResultCheck(4);
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 3:
				
				PageValue.s_FireRecvResult_2[EB_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글
               TestResultCheck(4);
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 4:
				PageValue.s_FireRecvResult_2[BAT_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글
               TestResultCheck(4);
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 5:
				PageValue.s_FireRecvResult_2[ABAT_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글
               TestResultCheck(4);
				ButtonValue.s_OkButton = 0; 
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 6:
				PageValue.s_FireRecvResult_2[BDU_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글
               TestResultCheck(4);
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 7:
				PageValue.s_FireRecvResult_2[ARMING_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글
                TestResultCheck(4);
				ButtonValue.s_OkButton = 0; 
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 8:
				PageValue.s_FireRecvResult_2[INTARM_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글
               TestResultCheck(4);
				ButtonValue.s_OkButton = 0; 
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 9:
				PageValue.s_FireRecvResult_2[INT_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글
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
            // <editor-fold defaultstate="collapsed" desc="2호탄 개별 점검 세부 결과">
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
				PageValue.s_FireDetailResult_2[EXT_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;

			case 3:
				printf("i Result/EBSQB.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_2[EB_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;
			case 4:
				printf("i Result/BATSQB.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_2[BAT_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;

			case 5:
				printf("i Result/ABATSQB.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_2[ABAT_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글 
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;
			case 6:
				printf("i Result/BDUSQB.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_2[BDU_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;

			case 7:
				printf("i Result/ARMING.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_2[ARMING_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;
			case 8:
				printf("i Result/INTARMING.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_2[INTARM_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;

			case 9:
				printf("i Result/INTSQB.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_2[INT_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글
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
                printf("i MENU/FireEachTestMenu_2.png,0,0\r");     
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
      else if(PageValue.s_Fire[FIRE_TEST_TOTAL_2]==1) //&& (PageValue.s_Check[CHK_LAN])) // 2호탄 통합 점검 개별 메시지 전송 및 수신
	{
          // <editor-fold defaultstate="collapsed" desc="2호탄 통합 점검">
	if(ButtonValue.s_OkButton == 1)
		{
            // <editor-fold defaultstate="collapsed" desc="2호탄 통합 점검">
			switch(g_InputValue)
			{
			case 1:
				PageValue.s_FireRecvResult_2[MSL_RESULT] = 1; // 유도탄 전원 수신 확인 
				TestResultCheck(4);         // 점검 결과 표시
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 2:
				PageValue.s_FireRecvResult_2[EXT_RESULT] = 1; // 유도탄 전원 수신 확인 
				TestResultCheck(4);
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 3:
				
				PageValue.s_FireRecvResult_2[EB_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글
               TestResultCheck(4);
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 4:
				PageValue.s_FireRecvResult_2[BAT_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글
               TestResultCheck(4);
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 5:
				PageValue.s_FireRecvResult_2[ABAT_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글
               TestResultCheck(4);
				ButtonValue.s_OkButton = 0; 
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 6:
				PageValue.s_FireRecvResult_2[BDU_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글
               TestResultCheck(4);
				ButtonValue.s_OkButton = 0;
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 7:
				PageValue.s_FireRecvResult_2[ARMING_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글
                TestResultCheck(4);
				ButtonValue.s_OkButton = 0; 
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 8:
				PageValue.s_FireRecvResult_2[INTARM_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글
               TestResultCheck(4);
				ButtonValue.s_OkButton = 0; 
				g_InputValue = 0;
				g_MenuPage = 1;
				break;
			case 9:
				PageValue.s_FireRecvResult_2[INT_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글
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
            // <editor-fold defaultstate="collapsed" desc="2호탄 통합 점검 세부 결과">
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
				PageValue.s_FireDetailResult_2[EXT_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;

			case 3:
				printf("i Result/EBSQB.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_2[EB_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;
			case 4:
				printf("i Result/BATSQB.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_2[BAT_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;

			case 5:
				printf("i Result/ABATSQB.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_2[ABAT_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글 
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;
			case 6:
				printf("i Result/BDUSQB.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_2[BDU_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;

			case 7:
				printf("i Result/ARMING.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_2[ARMING_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;
			case 8:
				printf("i Result/INTARMING.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_2[INTARM_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글
				ButtonValue.s_NextButton = 0;
				g_InputValue = 0;
				g_MenuPage = 0;
				break;

			case 9:
				printf("i Result/INTSQB.png,0,0\r");
				AlramPrint();
				PageValue.s_FireDetailResult_2[INT_RESULT] = 1;              // 발사 계통 점검 선택 메뉴 토글
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
                printf("i MENU/FireTotalTestMenu_2.png,0,0\r");   
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



int main(void)
{
	InitGPIO();          // gpio 초기화
	InitValue();           // 변수 초기화
	InitUart1();           // uart 초기화
	InitSpi1();            // spi 초기화
	InitAdc();
	InitRelay();            // 릴레이 포트 초기화
	InitTimer1();
	InitTimer2();
	InnerVoltTest();      // 내부 전압 점검
	InitInterrupt();       // 인터럽트 초기화
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
//:DONE: