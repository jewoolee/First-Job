struct INSUALATIONDATA{
    float InsulTestValue;   // ���� ��
    int InsulTestStatus;    // ���� ����
};
struct SHORTDATA{
    float ShortTestValue;   // ����/�ܼ� ��
    int ShortTestStatus;    // ����/�ܼ� ����
};
struct RESIDUALVOLTDATA{
    float ResidualVoltTestValue;    // �ܷ����� ��
    int ResidualVoltTestStatus;     // �ܷ����� ����
};
struct ECTSSTATUS{
    struct INSUALATIONDATA InsulTestResult[20];             // 20�� ä�� �������� ������ 
    int InsulTestTotalResult;                               // �������� ��ü ���
    struct SHORTDATA ShortTestResult[20];                   // 20�� ä�� ����/�ܼ� ���� ������
    int ShortTestTotalResult;                               // ����/�ܼ� ��ü ���
    struct RESIDUALVOLTDATA ResidualVoltTestResult[20];     // 20�� ä�� �ܷ��������� ������
    int ResidualVoltTestTotalResult;                        // �ܷ����� ���� ��ü ���
};
struct AD7980VALUE{
	unsigned int s_Data[20];
	unsigned long int s_Sum[20];
	float s_Value[20];
};
struct SELFTESTTYPE{
	unsigned char s_Result;
	float s_Vref;
    float s_Dc3v;
	float s_Dc5v;
	float s_Dc12vPlus;
	float s_Dc12vMinus;
    
};
#define LEDPRINTMAXDATA 50
#define CHANNEL 20
#define SHORTESTREFERENCE 0.38
#define INSULESTREFERENCE 4
#define RESIDUALREFERENCE 0.1
#define RESIDUALMARGIN 0.1   //10%

#define KEYPAD_1    PORTAbits.RA0 == 0
#define KEYPAD_2    PORTAbits.RA1 == 0
#define KEYPAD_3    (PORTAbits.RA0 == 0) && (PORTAbits.RA1 == 0)
#define KEYPAD_MENU    PORTAbits.RA2 == 0
#define KEYPAD_4    (PORTAbits.RA0 == 0) && (PORTAbits.RA2 == 0)
#define KEYPAD_5    (PORTAbits.RA1 == 0) && (PORTAbits.RA2 == 0)
#define KEYPAD_6    (PORTAbits.RA0 == 0) && (PORTAbits.RA1 == 0) && (PORTAbits.RA2 == 0)
#define KEYPAD_BACK    PORTAbits.RA3 == 0
#define KEYPAD_7    (PORTAbits.RA0 == 0) && (PORTAbits.RA3 == 0)
#define KEYPAD_8    PORTAbits.RA4 == 0
#define KEYPAD_9    PORTAbits.RA5 == 0
#define KEYPAD_S_BACK    (PORTAbits.RA4 == 0) && (PORTAbits.RA5 == 0)
#define KEYPAD_0    PORTAbits.RA6 == 0
#define KEYPAD_OK   (PORTAbits.RA4 == 0) && (PORTAbits.RA6 == 0)
#define KEYPAD_S_NEXT   (PORTAbits.RA5 == 0) && (PORTAbits.RA6 == 0)

#define CS0 (PORTGbits.RG0 = 1)   //  LED
#define CS1 (PORTGbits.RG1 = 1)   //  LED
#define CS2 (PORTGbits.RG2 = 1)   //  DATA
#define CS3 (PORTGbits.RG3 = 1)   //  ADD
#define CE_CNV (PORTGbits.RG9 = 1)    //  ADC
#define INIT_CS (CS0+CS1+CE_CNV)      //  �⺻�����ؾߵǴ�  LED+LED+ADC ����

#define Relay0 PORTEbits.RE6        // ������ 0�� �ܷ� ���� ����
#define Relay1 PORTEbits.RE7        // ������ 1�� ���� ����