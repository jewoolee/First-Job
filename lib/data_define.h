struct INSUALATIONDATA{
    float InsulTestValue;   // 절연 값
    int InsulTestStatus;    // 절연 상태
};
struct SHORTDATA{
    float ShortTestValue;   // 도통/단선 값
    int ShortTestStatus;    // 도통/단선 상태
};
struct RESIDUALVOLTDATA{
    float ResidualVoltTestValue;    // 잔류전압 값
    int ResidualVoltTestStatus;     // 잔류전압 상태
};
struct ECTSSTATUS{
    struct INSUALATIONDATA InsulTestResult[20];             // 20개 채널 절연점검 데이터 
    int InsulTestTotalResult;                               // 절연점검 전체 결과
    struct SHORTDATA ShortTestResult[20];                   // 20개 채널 도통/단선 점검 데이터
    int ShortTestTotalResult;                               // 도통/단선 전체 결과
    struct RESIDUALVOLTDATA ResidualVoltTestResult[20];     // 20개 채널 잔류전압점검 데이터
    int ResidualVoltTestTotalResult;                        // 잔류전압 점검 전체 결과
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
