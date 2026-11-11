#ifndef INVERTER_APP_H
#define INVERTER_APP_H
		
		
#ifdef __cplusplus
		extern "C" {
#endif


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "ql_log.h"
#include "ql_uart.h"

#define QL_UART_RX_BUFF_SIZE                2048
#define QL_UART_TX_BUFF_SIZE                2048

#define MIN(a,b) ((a) < (b) ? (a) : (b))

#define QL_UART1_TX_PIN						138
#define QL_UART1_TX_FUNC					0x03

#define QL_UART1_RX_PIN						137
#define QL_UART1_RX_FUNC					0x03

#define SERIAL_NUMBER_LENGTH_INV               10

typedef struct
{
	const unsigned char  *ParamCmd;
}Inverter_Cmd_t;

extern int INVERTER_COMMUNICATION;
extern int PARAM_CONFIG_MODE;
extern int WRITE_DATE_TIME ;
extern char RX_CONFIG_VALUE[20];
extern int CONFIG_WRITE_VAR;
extern int IsProcessDone ;

extern char CONFIG_VAR_NAME[20];
extern char CONFIG_VAR_STR[100];
extern double CONFIG_VAR_VALUE;
extern int Final_Fault;
extern BOOL Push_Fault_Server;

extern char serial_number[SERIAL_NUMBER_LENGTH_INV + 1];

typedef struct									                                 
  {
    float RunState;//0
    float GridVoltageAB;//1
    float GridVoltageBC;//2
    float GridVoltageAC;//3
    float GridVoltageA;//4
    float GridVoltageB;//5
    float GridVoltageC;//6
    float GridCurrentA;//7
    float GridCurrentB;//8
    float GridCurrentC;//9
    float GridFrequency;//10
    float DCVoltage1;//11
    float DCVoltage2;//12
    float DCVoltage3;//13
    float DCVoltage4;//14
    float DCCurrent1;//15
    float DCCurrent2;//16
    float DCCurrent3;//17
    float DCCurrent4;//18
    float DCPower1;//15
    float DCPower2;//16
    float DCPower3;//17
    float DCPower4;//18
    double TodayGeneratedEnergy;//19
    double CumulativeGeneratedEnergy;//20
    double MonthlyGeneratedEnergy;//21
    float InputActivePower;//22
    float OutputApparentPower;//23
    float OutputActivePower;//24
    float OutputReactivePower;//25
    float DayPvPower;//26
    float DayActivePower;//27
    float DayReactivePower;//28
    double TotalActivePower;//29
    double TotalReactivePower;//30
    double TotalWorkTime;//31
    double HistoryPvPower;//32
    float IGBTtemperatur;//33
    float RadiatorTemperature;
    float PowerFactor;//34
    float Warning;//35
    int Fault1;//36
    int Fault2;//37
    int Fault3;//38
    int Fault4;//39
  }InverterData_Init;

 extern InverterData_Init InverterData;

 typedef struct 
 {
    float Grid_Power_L1; 
    float Grid_Power_L2; 
    float Grid_Power_L3;
    float Inverter_Output_Power_L1; 
    float Inverter_Output_Power_L2; 
    float Inverter_Output_Power_L3;
    float Limiter_Power_L1; 
    float Limiter_Power_L2; 
    float Limiter_Power_L3;
    float Load_Power_L1; 
    float Load_Power_L2; 
    float Load_Power_L3;
    float Total_Invt_Output_Power;
    float Total_Grd_External_Power;
    float Total_Load_Power;
    float Day_Load_Power;
    double Cumulative_Load_Power;
    float Daily_Grid_Feed_In;
    double Cumulative_Grid_Feed_In;
    float Daily_Energy_Purchase;
    double Cumulative_Energy_Purchase;

 }Inverter_Output_Grid_Param;
 
extern Inverter_Output_Grid_Param Inverter_Output_Grid_Parameter;

 typedef union  
 {
   float DC_Voltage_Upper_Limit;
   float Grid_Voltage_Upper_Limit;
   float Grid_Voltage_Upper_Limit_L2;
   float Grid_Voltage_Upper_Limit_L3;
   float Grid_Voltage_Lower_Limit;
   float Grid_Voltage_Lower_Limit_L2;
   float Grid_Voltage_Lower_Limit_L3;
   float Grid_Current_Upper_Limit;
   float Grid_Freq_Upper_Limit;
   float Grid_Freq_Upper_Limit_L2;
   float Grid_Freq_Upper_Limit_L3;
   float Grid_Freq_Lower_Limit;
   float Grid_Freq_Lower_Limit_L2;
   float Grid_Freq_Lower_Limit_L3;
   float Rated_Grid_Voltage_Set;
   float Grid_Voltage_Upper_Limit_Trip_Time_1OD;
   float Grid_Voltage_Upper_Limit_Trip_Time_2OD;
   float Grid_Voltage_Upper_Limit_Trip_Time_3OD;
   float Grid_Voltage_Lower_Limit_Trip_Time_1OD;
   float Grid_Voltage_Lower_Limit_Trip_Time_2OD;
   float Grid_Voltage_Lower_Limit_Trip_Time_3OD;
   float Grid_Freq_Upper_Limit_Trip_Time_1OD;
   float Grid_Freq_Upper_Limit_Trip_Time_2OD;
   float Grid_Freq_Upper_Limit_Trip_Time_3OD;
   float Grid_Freq_Lower_Limit_Trip_Time_1OD;
   float Grid_Freq_Lower_Limit_Trip_Time_2OD;
   float Grid_Freq_Lower_Limit_Trip_Time_3OD;
   float Remote_Lock;
   float Self_Test_Time;
   float Starting_Voltage_Upper_Limit;
   float Starting_Voltage_Lower_Limit;
   float Over_Frequency_Drate_Point;
   float Over_Frequency_Drate;
   float Internal_Temp_Upper_Limit;
   float Power_Factor_Regulation;
   float Active_Power_Regulation;
   float Reactive_Power_Regulation;
   float Apparent_Power_Regulation;
   float Switch_ON_OFF_Enable;
   float Factory_Reset_Enable;
   float Self_Checking_Time;
   float Island_Protection_Enable;
   float Meter_Enable;
   float RCD_Enable;
   float RISO_Enable;
   float Grid_Standard;
   float PV_Curved_Enable;
   float Eeprome_Initial_Enabled;
   float Limiter_Function_Enabled;
   float Power_Wh_Factor;
   float External_CT_Flag;
 }Inverter_Config_Var;
 
extern Inverter_Config_Var Inverter_Config_Param;

 typedef struct 
 {
    int Protocal_Version;
    int Firmware_Ver_Control_Bord;
    int Firmware_Ver_Communication_Bord;
    int Inverter_Type;
    int Product_Type;        
    int MPPT_Phase_No;     
    int Rated_Grid_Voltage_Get;
    float Rated_Power;     
 }Inverter_Basic_Detl;
 
extern Inverter_Basic_Detl Inverter_Basic_Detail;


 typedef enum {
    RATED_GRID_VOLTAGE_SET = 1,
    REMOTE_LOCK,
    SELF_TEST_TIME,
    DC_VOLTAGE_UPPER_LIMIT ,
    GRID_VOLTAGE_LOWER_LIMIT ,
    GRID_VOLTAGE_UPPER_LIMIT,
    GRID_FREQ_LOWER_LIMIT,
    GRID_FREQ_UPPER_LIMIT,
    GRID_CURRENT_UPPER_LIMIT,
    STARTING_VOLTAGE_UPPER_LIMIT,
    STARTING_VOLTAGE_LOWET_LIMIT,
    OVER_FREQUENCY_DRATE_POINT,
    OVER_FREQUENCY_DRATE,
    INTERNAL_TEMPERATURE_UPPER_LIMIT,
    POWER_FACTOR_REGULATION,
    ACTIVE_POWER_REGULATION,
    REACTIVE_POWER_REGULATION,
    APPARENT_POWER_REGULATION,
    SWITCH_ON_OFF_ENABLE,
    FACTORY_RESET_ENABLE,
    SELF_CHECKING_TIME,
    ISLAND_PROTECTION_ENABLE,
    METER_ENABLED,
    RCD_ENABLED,
    RISO_ENABLED,
    GRID_STANDARD,
    PV_CURVE_ENABLED,
    EEPROME_INITAL_ENABLED,
    LIMITER_FUNCTION_ENABLE,
    POWER_WH_FACTOR,
    GRID_VOLTAGE_LOWER_LIMIT_L2 ,
    GRID_VOLTAGE_UPPER_LIMIT_L2,
    GRID_FREQ_LOWER_LIMIT_L2,
    GRID_FREQ_UPPER_LIMIT_L2,
    GRID_VOLTAGE_LOWER_LIMIT_L3 ,
    GRID_VOLTAGE_UPPER_LIMIT_L3,
    GRID_FREQ_LOWER_LIMIT_L3,
    GRID_FREQ_UPPER_LIMIT_L3,
    GRID_VOLTAGE_UPPER_LIMIT_TRIP_TIME_1OD,
    GRID_VOLTAGE_UPPER_LIMIT_TRIP_TIME_2OD,
    GRID_VOLTAGE_UPPER_LIMIT_TRIP_TIME_3OD,
    GRID_VOLTAGE_LOWER_LIMIT_TRIP_TIME_1OD,
    GRID_VOLTAGE_LOWER_LIMIT_TRIP_TIME_2OD,
    GRID_VOLTAGE_LOWER_LIMIT_TRIP_TIME_3OD,
    GRID_FREQ_UPPER_LIMIT_TRIP_TIME_1OD,
    GRID_FREQ_UPPER_LIMIT_TRIP_TIME_2OD,
    GRID_FREQ_UPPER_LIMIT_TRIP_TIME_3OD,
    GRID_FREQ_LOWER_LIMIT_TRIP_TIME_1OD,
    GRID_FREQ_LOWER_LIMIT_TRIP_TIME_2OD,
    GRID_FREQ_LOWER_LIMIT_TRIP_TIME_3OD,
    EXTERNAL_CT_FLAG,
    WR_DATE_TIME,

}ConfigVariables;

void UART_Init(void);
void RE_DE_AND_LED_init(void);
int inverter_app_init(void);
void inverter_comm_envent_loop();
void Serial_Write(unsigned char c);
void serial_int_ISR(unsigned char* c);
void inverter_parameter_setter(const unsigned char  *Re_cmd, unsigned char *Resp);
unsigned int ModRTU_CRC(unsigned char id,const unsigned char  *buf, unsigned int  len);
void ql_uart_notify_cb(QL_UART_PORT_NUMBER_E port, void *para);
void push_to_ParamCmd_buffer(Inverter_Cmd_t * cmd);
void Write_Parameter();
void Write_Config_Parameter();
unsigned char* getVariableFromEnum(int configVar);
void Assign_Value_to_Param(char *Val, unsigned char *Param_Name);
void Write_Config_Param_R(unsigned char *Inverter_Cmd);
void Write_Config_Param_W(unsigned char *Inverter_Cmd, char *Value);
void extract_serial_number(uint8_t *Resp, int length);
void Assign_DateTime_to_Param(char *Val, unsigned char *Param_Name);
void sanitize_string(char *str);
int decimalToBinary(unsigned long int num);
void Controlle_Led();
#ifdef __cplusplus
		}/*"C" */
#endif
		
#endif   
