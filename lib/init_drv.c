#include <p33FJ128GP310A.h> 
#include "init_drv.h"
/*
 const unsigned char first[2] = { 0x10,  // ��� ����
                                  0x20   // ���� ���� ������
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
	

const unsigned int RelaySelectParam[5] = {0x0008,    // A3-H           -> ������ 1�� ����
                                         0x0009,    // A0 - H, A3 - H -> ������ 2�� ����
                                         0x000A,    // A1 - H, A3 - H -> ������ 3�� ����
                                         0x000F    //A0 - H, A1 - H, A2 - H, A3 - H -> Y7�� ����(����Ȱ� �����Ƿ� ������ ���� ���� 
                                        };  // ������ ������ ��� ������ �ϰ� ������ �߻��Ҷ� ���� �Ѵ�.
                                            // 
const unsigned int IOPortSelectParam[9] = {	0x0001,     // ���� 8�� D   I/O ��Ʈ ���� ���� ���
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
    
	IFS0bits.T1IF = 0;			//Interrupt Flag : reset ���ͷ�Ʈ �÷��� ���� ��Ʈ
	IPC0bits.T1IP = 4;			//Interrupt Priority : LEVEL4(default) ���ͷ�Ʈ �켱���� ���� ��Ʈ
 	IEC0bits.T1IE = 1;			//Interrupt Enable ���ͷ�Ʈ ��� ���� ��Ʈ
	T1CONbits.TON = 0; 			//Timer ON control : OFF  �����Ʈ ���� Ÿ�̸� 1=���� 0=����
	T1CONbits.TSIDL = 0;		//Timer Stop in IDLe mode : OFF idle ��� ���� ��Ʈ 1=����̽��� idle ��忡 ���� Ÿ�̸ӵ��� ����, 0=idle ��忡�� Ÿ�̸� ���� ���
	T1CONbits.TGATE = 0;		//GATEed time accumulation : disable ����Ʈ�� ����Ǵ� Ÿ�̸� �ð� ���� ��ȿ
	T1CONbits.TCKPS = 3;		//Clock PreScale Val : 11(256:1)		
	T1CONbits.TSYNC = 0;		//Timer SYNC : disable Ÿ�̸� �ܺ� Ŭ�� �Է� ����ȭ ���� ��Ʈ 
                                //tcs=1 �϶� 1=�ܺ�Ŭ���Էµ���ȭ 0=�ܺ�Ŭ�� �Է� ����ȭ ���� 
                                //tcs=0�϶� �̺�Ʈ�� �����Ѵ�. Ÿ�̸�1�� �ܺ� Ŭ������ ����Ѵ�.
	T1CONbits.TCS = 0;			//Timer Clock Source : Internal  Ÿ�̸� Ŭ�� �ҽ� ���� ��Ʈ 1= �� txck������ �ܺ� Ŭ�� 0=����Ŭ��(fosc/4))
	PR1 = (FCY/256)/50;          //100ms Period �ֱ⼳��  
    
    //����Ŭ��(FCY)/���������Ϸ� = Xs���� -> 1/Xs -> ns�� ��ȯ 10��9�� ���� -> ns�� 1Ŭ���� �ɸ��� �ð�
    // ?ns X PRx = ?ns / 1000-> ?ms�� �� ���ϴ� �ֱ⸦ ���ϰ� 1Ŭ���� �ɸ��� �ð��� ���� PR1���� ���� 
    // �߻��ֱ� = 1/(fosc/4)/prescale/count
    //              ���� Ŭ��/���������Ϸ�(�����)/count�� ��� �ǳ� �ֱ⸦ ���̰� ������ ���
	       // ���ͷ�Ʈ �����Ʈ ���� Ÿ�̸� ����
}

void InitTimer2(void)   // Ȯ�� 3�� ��������� ����� ��ġ Ȯ�� Ÿ�̸�
{
    IFS0bits.T2IF = 0;			//Interrupt Flag : reset ���ͷ�Ʈ �÷��� ���� ��Ʈ
	IPC1bits.T2IP = 4;			//Interrupt Priority : LEVEL4(default) ���ͷ�Ʈ �켱���� ���� ��Ʈ
 	IEC0bits.T2IE = 1;			//Interrupt Enable ���ͷ�Ʈ ��� ���� ��Ʈ
	T2CONbits.TON = 0; 			//Timer ON control : OFF  �����Ʈ ���� Ÿ�̸� 1=���� 0=����
	T2CONbits.TSIDL = 0;		//Timer Stop in IDLe mode : OFF idle ��� ���� ��Ʈ 1=����̽��� idle ��忡 ���� Ÿ�̸ӵ��� ����, 0=idle ��忡�� Ÿ�̸� ���� ���
	T2CONbits.TGATE = 0;		//GATEed time accumulation : disable ����Ʈ�� ����Ǵ� Ÿ�̸� �ð� ���� ��ȿ
	T2CONbits.TCKPS = 3;		//Clock PreScale Val : 11(256:1)		
	T2CONbits.TCS = 0;			//Timer Clock Source : Internal  Ÿ�̸� Ŭ�� �ҽ� ���� ��Ʈ 1= �� txck������ �ܺ� Ŭ�� 0=����Ŭ��(fosc/4))
	PR2 = (FCY/256)/10;          //500ms Period �ֱ⼳��  
    
    //����Ŭ��(FCY)/���������Ϸ� = Xs���� -> 1/Xs -> ns�� ��ȯ 10��9�� ���� -> ns�� 1Ŭ���� �ɸ��� �ð�
    // ?ns X PRx = ?ns / 1000-> ?ms�� �� ���ϴ� �ֱ⸦ ���ϰ� 1Ŭ���� �ɸ��� �ð��� ���� PR1���� ���� 
    // �߻��ֱ� = 1/(fosc/4)/prescale/count
    //              ���� Ŭ��/���������Ϸ�(�����)/count�� ��� �ǳ� �ֱ⸦ ���̰� ������ ���
	       // ���ͷ�Ʈ �����Ʈ ���� Ÿ�̸� ����
    
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
								
	AD1CON2bits.BUFM = 0;		//Buffer Fill Status bit : 0bit ���۴� �ϳ��� 16���� ���۷� ȯ�� ����(1�� �ϸ� �ΰ��� 8�������)
	AD1CON2bits.SMPI = 5;		//Sample/Convert Sequences Per Interrupt Selection bits ���ͷ�Ʈ �� adc ��ȯ(adc 5�� ��ȯ�ϰ� ���ͷ�Ʈ �߻�)
	AD1CON2bits.CSCNA = 1;		//Scan Input Selections for CH0+ S/H Input for MUX A Input Multiplexer Setting bit: Scan inputs
	AD1CON2bits.VCFG = 3;		//Voltage Reference Configuration bits: Vreg+ = VREF+, Vreg- = VREF-

	AD1CON3bits.ADCS = 0x3F;		//A/D Conversion Clock Select bits : Max 
								//Tad=Tcy*(ADCS+1)=(1/40M)*64 = 1.6us (625KHz)
								//0/10��Ʈ ADC ��ȯ �ð� Tc = 12*Tad = 19.2us (52KHz)
								
	AD1CON3bits.SAMC=4; 			//Auto Sample Time = 0*Tad
	AD1CON3bits.ADRC=0;			//A/D Conversion Clock Source bit

	AD1CHS0=0;					// �Է� ä�� ���� ��������

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
	IEC0bits.INT0IE = 0;		// INT0 Interrupt ���� (0 - ����, 1 - ����)
	IPC0bits.INT0IP = 7;		// INT0 Interrupt Priority(0~7)
	
	INTCON2bits.INT1EP = 0;		// 1 = Interrupt on negative edge, 0 = Interrupt on positive edge ���ͷ�Ʈ ��ȣ�� 0�϶� ����
    // 0->1�� �ٲ�°� ����Ƽ�꿧��(���) 1->0���� �ٲ�°� �װ�Ƽ�� ����(�ϰ�)
	IFS1bits.INT1IF = 0;		// INT1 Interrupt Flag Clear
	IEC1bits.INT1IE = 1;		// INT1 Interrupt ���� (0 - ����, 1 - ����)
	IPC5bits.INT1IP = 7;		// INT1 Interrupt Priority(0~7)
	
	INTCON2bits.INT2EP = 0;		// 1 = Interrupt on negative edge, 0 = Interrupt on positive edge
	IFS1bits.INT2IF = 0;		// INT2 Interrupt Flag Clear
	IEC1bits.INT2IE = 1;		// INT2 Interrupt Stop (0 - ����, 1 - ����)
	IPC7bits.INT2IP = 7;		// INT2 Interrupt Priority(0~7)
	
	INTCON2bits.INT3EP = 0;		// 1 = Interrupt on negative edge, 0 = Interrupt on positive edge
	IFS3bits.INT3IF = 0;		// INT3 Interrupt Flag Clear
	IEC3bits.INT3IE = 0;		// INT3 Interrupt Stop 
	IPC13bits.INT3IP = 7;		// INT3 Interrupt Priority(0~7)
}

