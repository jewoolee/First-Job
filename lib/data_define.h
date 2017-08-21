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
    //float s_Dc3v;
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
