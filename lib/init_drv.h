#ifndef _INIT_DRV_H_
#define _INIT_DRV_H_


#define FCY					20000000    //FCY = FOSC/4 = 20000000 : FOSC = ECIO_PLL8 = (OSC X 8)= 80MHz
#define BAUDRATE			57600
#define BRGVAL				((FCY/BAUDRATE)/16)-1

//Bit manipulation macro
#define CLEAR_BIT(data, loc)			((data) &= ~(0x1<<(loc)))
#define SET_BIT(data, loc)				((data) |= (0x1<<(loc)))
#define CNV_WORD(data1, data2)			((((data1)<<8) & 0xFF00) + (data2))
#define CHECK_BIT(data, loc)			((data) & (0x1<<(loc)))

/******************* Write PORT Define *******************/
/* for control heat channel*/
#define BAD						LATCbits.LATC14
#define GOOD					LATCbits.LATC15


/******************** VALUE Define ***********************/
#define ON						1
#define OFF						0
#define VREF					4.988f	//4.998
#define BIT10					1023
#define BIT16					65535
#define GAIN_p5V				6
#define GAIN_p3V				6
#define GAIN_p12V				6
#define GAIN_m12V				5
#define BLINK_TIME				50

/******************* EEPROM address Define **************/
#define EEPROM_BASE_ADDR		0x7FF800
#define EEPROM_INITIAL_DATA		0xFFFF


/******************* Delay ******************************/ 
#define DELAY_US_CNT		(FCY*0.00000002)//use setting
//#define Delay_200uS_Cnt	  (Fcy * 0.0002)
#define DELAY_MS_CNT		(FCY*0.000004)//use setting
#endif

void InitGPIO(void);
void InitTimer1(void);
void InitTimer2(void);
void InitUart1(void);
void InitAdc(void);
void InitSpi1(void);
void InitInterrupt(void);


extern const unsigned int  IOPortSelectParam[9];
extern const unsigned int  ChipSelectParam[6];

extern const unsigned int RelaySelectParam[5];
