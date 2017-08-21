#include <p33FJ128GP310A.h> 
#include "init_drv.h"
/*
 const unsigned char first[2] = { 0x10,  // 모드 선택
                                  0x20   // 내부 전압 비정상
                                };
 const unsigned char chk[7] = { 0x10,   // CHK_OK
                                0x20,   // CHK_RESULT
                                0x30,   // CHK_HINT
                                0x31,   // CHK_HINT_1
                                0x40,   // CHK_LAN
                                0x50,   // CHK_VOLT
                                0x60    //CHK_BIT
                                };
 const unsigned char self[5] = { 0x10,  // SELF_TEST
                                 0x11,  // SELF_TEST_RES
                                 0x12,  // SELF_TEST_SHO
                                 0x13,  // SELF_TEST_INS
                                 0x14   // SELF_TEST_TOTAL
                                };
 const unsigned char line[5] ={ 0x20,   // LINE_TEST
                                0x21,   // LINE_TEST_RES
                                0x22,   // LINE_TEST_SHO
                                0x23,   // LINE_TEST_INS
                                0x24    // LINE_TEST_TOTAL
                                };
 const unsigned char fire[6] = { 0x30,  // FIRE_TEST
                                 0x31,  // FIRE_TEST_SEL
                                 0x32,  // FIRE_TEST_EACH_1
                                 0x33,  // FIRE_TEST_TOTAL_1
                                 0x34,  // FIRE_TEST_EACH_2
                                 0x35   //FIRE_TEST_TOTAL_2
                                };
 const unsigned char fireresult[9] = { 0x01,    // MSL_RESULT
                                       0x02,    // EXT_RESULT
                                       0x03,    // EB_RESULT
                                       0x04,    // BAT_RESULT
                                       0x05,    // ABAT_RESULT
                                       0x06,    // BDU_RESULT
                                       0x07,    // ARMING_RESULT
                                       0x08,    // INTARM_RESULT
                                       0x09     // INT_RESULT
                                      };
const unsigned char result_ing[6] = { 0x10,    // RES_ING
                               0x11,    // RES_RESULT
                               0x20,    // SHO_ING
                               0x21,    // SHO_RESULT
                               0x30,    // INS_ING
                               0x31     // INS_RESULT
                              };
*/
	

const unsigned int RelaySelectParam[5] = {0x0008,    // A3-H           -> 릴레이 1번 제어
                                         0x0009,    // A0 - H, A3 - H -> 릴레이 2번 제어
                                         0x000A,    // A1 - H, A3 - H -> 릴레이 3번 제어
                                         0x000F    //A0 - H, A1 - H, A2 - H, A3 - H -> Y7번 동작(연결된게 없으므로 릴레이 동작 해제 
                                        };  // 릴레이 동작은 상승 엣지와 하강 엣지가 발생할때 동작 한다.
                                            // 
const unsigned int IOPortSelectParam[9] = {	0x0001,     // 공통 8개 D   I/O 포트 개별 선택 상수
                                            0x0002,
                                            0x0004,
                                            0x0008,
                                            0x0010,
                                            0x0020,
                                            0x0040,
                                            0x0080};
 
const unsigned int ChipSelectParam[6] = {0x0001,        //Select LED (CS0)
                                         0x0004,		//Select ADC (CS2)
                                         0x0044,		//Select DATA ADC(CS4 + CS2)
                                         0x0006,		//Select ADDRESS ACD (CS1 + CS2)
                                         0x0003,         //Select RELAY0,1 DO0+DO1
                                         0x0005         // Select ADC LED (CS2 + CS0))
};


void InitGPIO(void)
{
	// INPUT = 1 , OUTPUT = 0
	TRISA = 0xFFFF; 	// INPUT : ALL
	TRISB = 0xFFFF; 	// INPUT : ALL
	TRISC = 0x0000; 	// INPUT : ALL
	TRISD = 0x0000; 	// INPUT : ALL
	TRISE = 0xFFFF; 	// INTPUT : ALL
	TRISF = 0x0084; 	// INPUT : RF2,RF7					OUTPUT : RF0,RF1,RF3~RF6,RF8
	TRISG = 0x0000; 	// INPUT : 							OUTPUT : ALL
	
	// PORT output initialize
	PORTA = 0x0000;
	PORTB = 0x0000;
	PORTC = 0x0000;
	PORTD = 0x0000;
	PORTE = 0x0000;
	PORTF = 0x0001;
	PORTG = 0x000C;
}

void InitTimer1(void)
{
    
	IFS0bits.T1IF = 0;			//Interrupt Flag : reset 인터럽트 플레그 상태 비트
	IPC0bits.T1IP = 4;			//Interrupt Priority : LEVEL4(default) 인터럽트 우선순위 제어 비트
 	IEC0bits.T1IE = 1;			//Interrupt Enable 인터럽트 허용 제어 비트
	T1CONbits.TON = 0; 			//Timer ON control : OFF  제어비트 관한 타이머 1=시작 0=중지
	T1CONbits.TSIDL = 0;		//Timer Stop in IDLe mode : OFF idle 모드 정지 비트 1=디바이스가 idle 모드에 들어갈때 타이머동작 멈춤, 0=idle 모드에서 타이머 동작 계속
	T1CONbits.TGATE = 0;		//GATEed time accumulation : disable 게이트로 제어되는 타이머 시간 축적 유효
	T1CONbits.TCKPS = 3;		//Clock PreScale Val : 11(256:1)		
	T1CONbits.TSYNC = 0;		//Timer SYNC : disable 타이머 외부 클럭 입력 동기화 선택 비트 
                                //tcs=1 일때 1=외부클럭입력동기화 0=외부클럭 입력 동기화 안함 
                                //tcs=0일때 이비트는 무시한다. 타이머1은 외부 클럭으로 사용한다.
	T1CONbits.TCS = 0;			//Timer Clock Source : Internal  타이머 클럭 소스 선택 비트 1= 핀 txck에서의 외부 클럭 0=내부클럭(fosc/4))
	PR1 = (FCY/256)/50;          //100ms Period 주기설정  
    
    //내부클럭(FCY)/프리스케일러 = Xs값을 -> 1/Xs -> ns로 변환 10의9승 곱함 -> ns는 1클럭이 걸리는 시간
    // ?ns X PRx = ?ns / 1000-> ?ms값 즉 원하는 주기를 정하고 1클럭이 걸리는 시간을 구해 PR1값을 산출 
    // 발생주기 = 1/(fosc/4)/prescale/count
    //              내부 클럭/프리스케일러(없어도됨)/count도 없어도 되나 주기를 줄이고 싶을때 사용
	       // 인터럽트 제어비트 관한 타이머 시작
}

void InitTimer2(void)   // 확인 3초 눌렀을경우 디버깅 수치 확인 타이머
{
    IFS0bits.T2IF = 0;			//Interrupt Flag : reset 인터럽트 플레그 상태 비트
	IPC1bits.T2IP = 4;			//Interrupt Priority : LEVEL4(default) 인터럽트 우선순위 제어 비트
 	IEC0bits.T2IE = 1;			//Interrupt Enable 인터럽트 허용 제어 비트
	T2CONbits.TON = 0; 			//Timer ON control : OFF  제어비트 관한 타이머 1=시작 0=중지
	T2CONbits.TSIDL = 0;		//Timer Stop in IDLe mode : OFF idle 모드 정지 비트 1=디바이스가 idle 모드에 들어갈때 타이머동작 멈춤, 0=idle 모드에서 타이머 동작 계속
	T2CONbits.TGATE = 0;		//GATEed time accumulation : disable 게이트로 제어되는 타이머 시간 축적 유효
	T2CONbits.TCKPS = 3;		//Clock PreScale Val : 11(256:1)		
	T2CONbits.TCS = 0;			//Timer Clock Source : Internal  타이머 클럭 소스 선택 비트 1= 핀 txck에서의 외부 클럭 0=내부클럭(fosc/4))
	PR2 = (FCY/256)/10;          //500ms Period 주기설정  
    
    //내부클럭(FCY)/프리스케일러 = Xs값을 -> 1/Xs -> ns로 변환 10의9승 곱함 -> ns는 1클럭이 걸리는 시간
    // ?ns X PRx = ?ns / 1000-> ?ms값 즉 원하는 주기를 정하고 1클럭이 걸리는 시간을 구해 PR1값을 산출 
    // 발생주기 = 1/(fosc/4)/prescale/count
    //              내부 클럭/프리스케일러(없어도됨)/count도 없어도 되나 주기를 줄이고 싶을때 사용
	       // 인터럽트 제어비트 관한 타이머 시작
    
}

void InitUart1(void)
{
	U1MODE = 0x0000;			// MODE : ALL reset
	U1STA = 0x0000;				// STAtus : ALL reset

	U1MODEbits.STSEL = 0;   	// STop bit : 0(1bit) 1(2bit)
	U1MODEbits.PDSEL = 0;		// Parity&Data : 0(No parity, 8bit)
	U1MODEbits.ABAUD = 0;		// AutoBAUD : disable
	U1BRG = BRGVAL;				// Set BaudRate
	IPC2bits.U1RXIP = 7;
	U1STAbits.UTXISEL1 = 1;		// TX Interrupt : 0(enable) 1(disable)
	U1STAbits.URXISEL = 0;		// RX Interrupt : 0(enable) 1(disable)
	U1MODEbits.UARTEN = 1;		// UART enable : 0(disable) 1(enable)
	U1STAbits.UTXEN = 1;		// Transmit enable : 0(disable) 1(enable)
	IFS0bits.U1TXIF = 0;		// TX Interrupt Flag Clear
	IFS0bits.U1RXIF = 0;		// RX Interrupt Flag Clear
	IEC0bits.U1TXIE = 0;		// 0(TX Interrupt Stop) 1(TX Interrupt Start)
	IEC0bits.U1RXIE = 1;		// 0(RX Interrupt Stop) 1(RX Interrupt Start)
}

void InitAdc(void)
{
	
	AD1CON1bits.SAMP = 0;		//A/D Sample Enable bit
	AD1CON1bits.ASAM = 1;		//A/D Sample Auto-Start bit
	AD1CON1bits.SSRC = 7;		// Conversion Trigger Source Select bits :Internal counter ends sampling and starts conversion (auto convert)
	AD1CON1bits.FORM = 0;		//Data Output Format bits: Integer 
								//(DOUT = 0000 dddd dddd dddd)
								
	AD1CON2bits.BUFM = 0;		//Buffer Fill Status bit : 0bit 버퍼는 하나의 16워드 버퍼로 환경 설정(1로 하면 두개의 8워드버퍼)
	AD1CON2bits.SMPI = 5;		//Sample/Convert Sequences Per Interrupt Selection bits 인터럽트 당 adc 변환(adc 5번 변환하고 인터럽트 발생)
	AD1CON2bits.CSCNA = 1;		//Scan Input Selections for CH0+ S/H Input for MUX A Input Multiplexer Setting bit: Scan inputs
	AD1CON2bits.VCFG = 3;		//Voltage Reference Configuration bits: Vreg+ = VREF+, Vreg- = VREF-

	AD1CON3bits.ADCS = 0x3F;		//A/D Conversion Clock Select bits : Max 
								//Tad=Tcy*(ADCS+1)=(1/40M)*64 = 1.6us (625KHz)
								//0/10비트 ADC 변환 시간 Tc = 12*Tad = 19.2us (52KHz)
								
	AD1CON3bits.SAMC=4; 			//Auto Sample Time = 0*Tad
	AD1CON3bits.ADRC=0;			//A/D Conversion Clock Source bit

	AD1CHS0=0;					// 입력 채널 선택 레지스터

	//AD1CSSL 
	AD1CSSL = 0x00FC;			// 0 = Digital : AN6~AN7 , 1 = Analog : AN0~AN5, AN8~AN15

	//AD1PCFGL		
	AD1PCFGL = 0xFF03;			// 1 = Digital : AN6~AN7 , 0 = Analog : AN0~AN5, AN8~AN15
	IFS0bits.AD1IF = 0;			//ADC Interrupt Flag Clear
	IEC0bits.AD1IE = 0;			//ADC Interrupt : 0(disable) 1(enable)

	AD1CON1bits.ADON = 1;		//A/D Operating Mode bit : start = 1

}

void InitSpi1(void)
{
    
	SPI1STATbits.SPIEN	 = 1;	//SPI : 0(disable) 1(enable)
	SPI1CON1bits.DISSDO = 1;		//SPI Pin enable : SDOx pin is controlled by the module
	SPI1CON1bits.SSEN = 0;		//SPI Slave mode: pin not used by module. Pin controlled by port function
	SPI1CON1bits.SMP = 0;
	SPI1CON1bits.MODE16	= 1;	//Data size 16bit
	SPI1CON1bits.CKE	= 0;		//
	SPI1CON1bits.CKP	= 0;		//
	SPI1CON1bits.MSTEN	= 1;	//Master set
	SPI1CON1bits.PPRE 	= 1;	//16:1
	SPI1CON1bits.SPRE 	= 3;	//4:1
	SPI1CON2bits.FRMEN	= 0;	//Frame mode : 0(disable) 1(enable)
	SPI1CON2bits.SPIFSD = 0;		//Frame SYNC active low : 0(disable) 1(enable)
	SPI1STATbits.SPISIDL = 0;	//Ideal mode SPI: 1(disable) 0(enable)
	SPI1STATbits.SPIROV = 1;	//Overflow
	
	IFS0bits.SPI1IF = 0;		//SPI1 Interrupt Flag Clear
	IEC0bits.SPI1IE = 0;		//SPI1 Interrupt : 0(disable) 1(enable)
}

void InitInterrupt(void)
{
	INTCON1bits.NSTDIS = 0;
	INTCON2bits.ALTIVT = 1;
	IFS1bits.CNIF = 1;
	
	INTCON2bits.INT0EP = 0;		// 1 = Interrupt on negative edge, 0 = Interrupt on positive edge
	IFS0bits.INT0IF = 0;		// INT0 Interrupt Flag Clear
	IEC0bits.INT0IE = 0;		// INT0 Interrupt 중지 (0 - 중지, 1 - 동작)
	IPC0bits.INT0IP = 7;		// INT0 Interrupt Priority(0~7)
	
	INTCON2bits.INT1EP = 0;		// 1 = Interrupt on negative edge, 0 = Interrupt on positive edge 인터럽트 신호가 0일때 동작
    // 0->1로 바뀌는게 파지티브엣지(상승) 1->0으로 바뀌는게 네거티브 엣지(하강)
	IFS1bits.INT1IF = 0;		// INT1 Interrupt Flag Clear
	IEC1bits.INT1IE = 1;		// INT1 Interrupt 동작 (0 - 중지, 1 - 동작)
	IPC5bits.INT1IP = 7;		// INT1 Interrupt Priority(0~7)
	
	INTCON2bits.INT2EP = 0;		// 1 = Interrupt on negative edge, 0 = Interrupt on positive edge
	IFS1bits.INT2IF = 0;		// INT2 Interrupt Flag Clear
	IEC1bits.INT2IE = 1;		// INT2 Interrupt Stop (0 - 중지, 1 - 동작)
	IPC7bits.INT2IP = 7;		// INT2 Interrupt Priority(0~7)
	
	INTCON2bits.INT3EP = 0;		// 1 = Interrupt on negative edge, 0 = Interrupt on positive edge
	IFS3bits.INT3IF = 0;		// INT3 Interrupt Flag Clear
	IEC3bits.INT3IE = 0;		// INT3 Interrupt Stop 
	IPC13bits.INT3IP = 7;		// INT3 Interrupt Priority(0~7)
}

