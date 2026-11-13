
#include "ql_gpio.h"
#include "ql_uart.h"
#include "main_app.h"
#include "inverter_app.h"
#include "mqtt_app.h"
#include "ql_adc.h"
#include "ql_log.h"
#include "ql_rtos.h"
#include "led_config.h"

/* =========================================================================
 * EG800 UART configuration — change here if wiring changes.
 * ========================================================================= */
#define INVERTER_UART_PORT          QL_MAIN_UART_PORT
#define INVERTER_UART_BAUD          QL_UART_BAUD_115200

/* =========================================================================
 * Modbus parameter tables (unchanged from EC200 — register addresses only).
 * ========================================================================= */
const unsigned char  Inverter_Serial[]                      =   {0x03, 0x00, 0x06, 0x00, 0x09}; //change
const unsigned char  Comm_Protocal[]                        =   {0x03, 0x00, 0x02, 0x00, 0x01};

const unsigned char  Run_State[]                            =   {0x03, 0x00, 0x21, 0x00, 0x01};//change
const unsigned char  Date_Time[]                            =   {0x03, 0x00, 0x16, 0x00, 0x03};

const unsigned char  Grid_A_Voltage[]                       =   {0x03, 0x00, 0x49, 0x00, 0x01};
const unsigned char  Grid_B_Voltage[]                       =   {0x03, 0x00, 0x4A, 0x00, 0x01};
const unsigned char  Grid_C_Voltage[]                       =   {0x03, 0x00, 0x4B, 0x00, 0x01};

const unsigned char  Grid_AB_Voltage[]                      =   {0x03, 0x00, 0x46, 0x00, 0x01};
const unsigned char  Grid_BC_Voltage[]                      =   {0x03, 0x00, 0x47, 0x00, 0x01};
const unsigned char  Grid_AC_Voltage[]                      =   {0x03, 0x00, 0x48, 0x00, 0x01};

const unsigned char  Grid_A_Current[]                       =   {0x03, 0x00, 0x4C, 0x00, 0x01};
const unsigned char  Grid_B_Current[]                       =   {0x03, 0x00, 0x4D, 0x00, 0x01};
const unsigned char  Grid_C_Current[]                       =   {0x03, 0x00, 0x4E, 0x00, 0x01};

const unsigned char  Grid_Frequency[]                       =   {0x03, 0x00, 0x4F, 0x00, 0x01};

const unsigned char  DC_Voltage_1[]                         =   {0x03, 0x00, 0x27, 0x00, 0x02};//change
const unsigned char  DC_Voltage_2[]                         =   {0x03, 0x00, 0x2F, 0x00, 0x02};//change
const unsigned char  DC_Voltage_3[]                         =   {0x03, 0x00, 0x71, 0x00, 0x01};
const unsigned char  DC_Voltage_4[]                         =   {0x03, 0x00, 0x73, 0x00, 0x01};

const unsigned char  DC_Current_1[]                         =   {0x03, 0x00, 0x29, 0x00, 0x02};//change
const unsigned char  DC_Current_2[]                         =   {0x03, 0x00, 0x31, 0x00, 0x02};//change
const unsigned char  DC_Current_3[]                         =   {0x03, 0x00, 0x72, 0x00, 0x01};
const unsigned char  DC_Current_4[]                         =   {0x03, 0x00, 0x74, 0x00, 0x01};

const unsigned char  DC_Power_1[]                           =   {0x03, 0x00, 0x2B, 0x00, 0x02};//change
const unsigned char  DC_Power_2[]                           =   {0x03, 0x00, 0x33, 0x00, 0x02};//change
const unsigned char  DC_Power_3[]                           =   {0x03, 0x00, 0xBC, 0x00, 0x01};
const unsigned char  DC_Power_4[]                           =   {0x03, 0x00, 0xBD, 0x00, 0x01};

const unsigned char  Input_Active_Power[]                   =   {0x03, 0x00, 0x52, 0x00, 0x02};
const unsigned char  Output_Apparent_Power[]                =   {0x03, 0x00, 0x54, 0x00, 0x02};
const unsigned char  Output_Active_Power[]                  =   {0x03, 0x00, 0x56, 0x00, 0x02};
const unsigned char  Output_Reactive_Power[]                =   {0x03, 0x00, 0x58, 0x00, 0x02};

const unsigned char  Day_Active_Power[]                     =   {0x03, 0x00, 0x3C, 0x00, 0x01};
const unsigned char  Day_Reactive_Power[]                   =   {0x03, 0x00, 0x3D, 0x00, 0x01};
const unsigned char  Total_Active_Power[]                   =   {0x03, 0x00, 0x3F, 0x00, 0x02};
const unsigned char  Total_Rective_Power[]                  =   {0x03, 0x00, 0x41, 0x00, 0x02};

const unsigned char  Total_Work_Time[]                      =   {0x03, 0x00, 0x43, 0x00, 0x02};

const unsigned char  Today_Gen[]                            =   {0x03, 0x00, 0x3E, 0x00, 0x01};
const unsigned char  Total_Gen_H[]                          =   {0x03, 0x00, 0x5C, 0x00, 0x01};
const unsigned char  Total_Gen_L[]                          =   {0x03, 0x00, 0x5F, 0x00, 0x01};

const unsigned char  RadiatorT_emperature[]                 =   {0x03, 0x00, 0x5A, 0x00, 0x01};
const unsigned char  IGBT_Temperatur[]                      =   {0x03, 0x00, 0x5B, 0x00, 0x01};
const unsigned char  Power_Factor[]                         =   {0x03, 0x00, 0x5D, 0x00, 0x01};

const unsigned char  History_PV_Power[]                     =   {0x03, 0x00, 0x60, 0x00, 0x02};

const unsigned char  Warning_Msg_Ward_1[]                   =   {0x03, 0x00, 0x65, 0x00, 0x01};
const unsigned char  Warning_Msg_Ward_2[]                   =   {0x03, 0x00, 0x66, 0x00, 0x01};
const unsigned char  Fault_Ward_1[]                         =   {0x03, 0x00, 0x67, 0x00, 0x01};
const unsigned char  Fault_Ward_2[]                         =   {0x03, 0x00, 0x68, 0x00, 0x01};
const unsigned char  Fault_Ward_3[]                         =   {0x03, 0x00, 0x69, 0x00, 0x01};
const unsigned char  Fault_Ward_4[]                         =   {0x03, 0x00, 0x6A, 0x00, 0x01};

const unsigned char  Inverter_Type[]                        =   {0x03, 0x00, 0x00, 0x00, 0x01};
const unsigned char  Product_Type[]                         =   {0x03, 0x00, 0x08, 0x00, 0x01};
const unsigned char  Firm_Ver_Control_Bord[]                =   {0x03, 0x00, 0x0D, 0x00, 0x01};
const unsigned char  Firm_Ver_Comm_Bord[]                   =   {0x03, 0x00, 0x0E, 0x00, 0x01};
const unsigned char  Rated_Power[]                          =   {0x03, 0x00, 0x10, 0x00, 0x02};
const unsigned char  MPPT_Phase_No[]                        =   {0x03, 0x00, 0x12, 0x00, 0x01};
const unsigned char  Rated_Grid_Voltage_Get[]               =   {0x03, 0x00, 0x13, 0x00, 0x01};

const unsigned char  Inverter_Output_Power_L1[]             =   {0x03, 0x00, 0xAD, 0x00, 0x01};
const unsigned char  Inverter_Output_Power_L2[]             =   {0x03, 0x00, 0xAE, 0x00, 0x01};

const unsigned char  Grid_Power_L1[]                        =   {0x03, 0x00, 0xA7, 0x00, 0x01};
const unsigned char  Grid_Power_L2[]                        =   {0x03, 0x00, 0xA8, 0x00, 0x01};

const unsigned char  Limiter_Power_L1[]                     =   {0x03, 0x00, 0xAA, 0x00, 0x01};
const unsigned char  Limiter_Power_L2[]                     =   {0x03, 0x00, 0xAB, 0x00, 0x01};

const unsigned char  Load_Power_L1[]                        =   {0x03, 0x00, 0xB0, 0x00, 0x01};
const unsigned char  Load_Power_L2[]                        =   {0x03, 0x00, 0xB1, 0x00, 0x01};

const unsigned char  Total_Invter_Out_Power[]               =   {0x03, 0x00, 0xAF, 0x00, 0x01};
const unsigned char  Toal_Grid_External_Power[]             =   {0x03, 0x00, 0xAC, 0x00, 0x01};
const unsigned char  Total_Load_Power[]                     =   {0x03, 0x00, 0xB2, 0x00, 0x01};

const unsigned char  Day_Load_Power[]                       =   {0x03, 0x00, 0xC8, 0x00, 0x01};
const unsigned char  Cumulative_Load_Power[]                =   {0x03, 0x00, 0xC9, 0x00, 0x02};

const unsigned char  Daily_Grid_Feed_In[]                   =   {0x03, 0x00, 0xCD, 0x00, 0x01};
const unsigned char  Cumulative_Grid_Feed_In[]              =   {0x03, 0x00, 0xCE, 0x00, 0x02};
const unsigned char  Daily_Energy_Purchase[]                =   {0x03, 0x00, 0xD0, 0x00, 0x01};
const unsigned char  Cumulative_Energy_Purchase[]           =   {0x03, 0x00, 0xD1, 0x00, 0x02};
const unsigned char  Meter_Enable_Read[]                    =   {0x03, 0x00, 0x30, 0x00, 0x01};

/* =========================================================================
 * Write-capable config parameters (8 bytes: 5 header + 1 byte-count + 2 data)
 * ========================================================================= */
unsigned char Write_Date_Time[]                             =   {0x03, 0x00, 0x16, 0x00, 0x03, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

unsigned char DC_Voltage_Upper_Limit[]                      =   {0x03, 0x00, 0x1A, 0x00, 0x01, 0x02, 0x00, 0x00};
unsigned char Grid_Voltage_Upper_Limit[]                    =   {0x03, 0x00, 0x1B, 0x00, 0x01, 0x02, 0x00, 0x00};
unsigned char Grid_Voltage_Upper_Limit_L2[]                 =   {0x03, 0x01, 0x2C, 0x00, 0x01, 0x02, 0x00, 0x00};
unsigned char Grid_Voltage_Upper_Limit_L3[]                 =   {0x03, 0x01, 0x2D, 0x00, 0x01, 0x02, 0x00, 0x00};
unsigned char Grid_Voltage_Lower_Limit[]                    =   {0x03, 0x00, 0x1C, 0x00, 0x01, 0x02, 0x00, 0x00};
unsigned char Grid_Voltage_Lower_Limit_L2[]                 =   {0x03, 0x01, 0x2E, 0x00, 0x01, 0x02, 0x00, 0x00};
unsigned char Grid_Voltage_Lower_Limit_L3[]                 =   {0x03, 0x01, 0x2F, 0x00, 0x01, 0x02, 0x00, 0x00};
unsigned char Grid_Current_Upper_Limit[]                    =   {0x03, 0x00, 0x1F, 0x00, 0x01, 0x02, 0x00, 0x00};
unsigned char Grid_Freq_Upper_Limit[]                       =   {0x03, 0x00, 0x1D, 0x00, 0x01, 0x02, 0x00, 0x00};
unsigned char Grid_Freq_Upper_Limit_L2[]                    =   {0x03, 0x01, 0x30, 0x00, 0x01, 0x02, 0x00, 0x00};
unsigned char Grid_Freq_Upper_Limit_L3[]                    =   {0x03, 0x01, 0x31, 0x00, 0x01, 0x02, 0x00, 0x00};
unsigned char Grid_Freq_Lower_Limit[]                       =   {0x03, 0x00, 0x1E, 0x00, 0x01, 0x02, 0x00, 0x00};
unsigned char Grid_Freq_Lower_Limit_L2[]                    =   {0x03, 0x01, 0x32, 0x00, 0x01, 0x02, 0x00, 0x00};
unsigned char Grid_Freq_Lower_Limit_L3[]                    =   {0x03, 0x01, 0x33, 0x00, 0x01, 0x02, 0x00, 0x00};
unsigned char Rated_Grid_Voltage_Set[]                      =   {0x03, 0x00, 0x13, 0x00, 0x01, 0x02, 0x00, 0x00};

unsigned char Grid_Voltage_Upper_Limit_Trip_Time_1OD[]      =   {0x03, 0x01, 0x34, 0x00, 0x01, 0x02, 0x00, 0x00};
unsigned char Grid_Voltage_Upper_Limit_Trip_Time_2OD[]      =   {0x03, 0x01, 0x35, 0x00, 0x01, 0x02, 0x00, 0x00};
unsigned char Grid_Voltage_Upper_Limit_Trip_Time_3OD[]      =   {0x03, 0x01, 0x36, 0x00, 0x01, 0x02, 0x00, 0x00};
unsigned char Grid_Voltage_Lower_Limit_Trip_Time_1OD[]      =   {0x03, 0x01, 0x37, 0x00, 0x01, 0x02, 0x00, 0x00};
unsigned char Grid_Voltage_Lower_Limit_Trip_Time_2OD[]      =   {0x03, 0x01, 0x38, 0x00, 0x01, 0x02, 0x00, 0x00};
unsigned char Grid_Voltage_Lower_Limit_Trip_Time_3OD[]      =   {0x03, 0x01, 0x39, 0x00, 0x01, 0x02, 0x00, 0x00};
unsigned char Grid_Freq_Upper_Limit_Trip_Time_1OD[]         =   {0x03, 0x01, 0x3A, 0x00, 0x01, 0x02, 0x00, 0x00};
unsigned char Grid_Freq_Upper_Limit_Trip_Time_2OD[]         =   {0x03, 0x01, 0x3B, 0x00, 0x01, 0x02, 0x00, 0x00};
unsigned char Grid_Freq_Upper_Limit_Trip_Time_3OD[]         =   {0x03, 0x01, 0x3C, 0x00, 0x01, 0x02, 0x00, 0x00};
unsigned char Grid_Freq_Lower_Limit_Trip_Time_1OD[]         =   {0x03, 0x01, 0x3D, 0x00, 0x01, 0x02, 0x00, 0x00};
unsigned char Grid_Freq_Lower_Limit_Trip_Time_2OD[]         =   {0x03, 0x01, 0x3E, 0x00, 0x01, 0x02, 0x00, 0x00};
unsigned char Grid_Freq_Lower_Limit_Trip_Time_3OD[]         =   {0x03, 0x01, 0x3F, 0x00, 0x01, 0x02, 0x00, 0x00};

unsigned char Remote_Lock[]                                 =   {0x03, 0x00, 0x14, 0x00, 0x01, 0x02, 0x00, 0x00};
unsigned char Self_Test_Time[]                              =   {0x03, 0x00, 0x15, 0x00, 0x01, 0x02, 0x00, 0x00};
unsigned char Starting_Voltage_Upper_Limit[]                =   {0x03, 0x00, 0x20, 0x00, 0x01, 0x02, 0x00, 0x00};
unsigned char Starting_Voltage_Lower_Limit[]                =   {0x03, 0x00, 0x21, 0x00, 0x01, 0x02, 0x00, 0x00};
unsigned char Over_Frequency_Drate_Point[]                  =   {0x03, 0x00, 0x22, 0x00, 0x01, 0x02, 0x00, 0x00};
unsigned char Over_Frequency_Drate[]                        =   {0x03, 0x00, 0x23, 0x00, 0x01, 0x02, 0x00, 0x00};
unsigned char Internal_Temp_Upper_Limit[]                   =   {0x03, 0x00, 0x24, 0x00, 0x01, 0x02, 0x00, 0x00};
unsigned char Power_Factor_Regulation[]                     =   {0x03, 0x00, 0x65, 0x00, 0x01, 0x02, 0x00, 0x00};
unsigned char Active_Power_Regulation[]                     =   {0x03, 0x00, 0x28, 0x00, 0x01, 0x02, 0x00, 0x00};
unsigned char Reactive_Power_Regulation[]                   =   {0x03, 0x00, 0x60, 0x00, 0x01, 0x02, 0x00, 0x00};
unsigned char Apparent_Power_Regulation[]                   =   {0x03, 0x00, 0x2A, 0x00, 0x01, 0x02, 0x00, 0x00};
unsigned char Switch_ON_OFF_Enable[]                        =   {0x03, 0x00, 0x2B, 0x00, 0x01, 0x02, 0x00, 0x00};
unsigned char Factory_Reset_Enable[]                        =   {0x03, 0x00, 0x2C, 0x00, 0x01, 0x02, 0x00, 0x00};
unsigned char Self_Checking_Time[]                          =   {0x03, 0x00, 0x2D, 0x00, 0x01, 0x02, 0x00, 0x00};
unsigned char Island_Protection_Enable[]                    =   {0x03, 0x00, 0x2E, 0x00, 0x01, 0x02, 0x00, 0x00};
unsigned char Meter_Enable[]                                =   {0x03, 0x00, 0x30, 0x00, 0x01, 0x02, 0x00, 0x00};
unsigned char RCD_Enable[]                                  =   {0x03, 0x00, 0x55, 0x00, 0x01, 0x02, 0x00, 0x00};
unsigned char RISO_Enable[]                                 =   {0x03, 0x00, 0x32, 0x00, 0x01, 0x02, 0x00, 0x00};
unsigned char Grid_Standard[]                               =   {0x03, 0x00, 0x33, 0x00, 0x01, 0x02, 0x00, 0x00};
unsigned char PV_Curved_Enable[]                            =   {0x03, 0x00, 0x34, 0x00, 0x01, 0x02, 0x00, 0x00};
unsigned char Eeprome_Initial_Enabled[]                     =   {0x03, 0x00, 0x36, 0x00, 0x01, 0x02, 0x00, 0x00};
unsigned char Limiter_Function_Enabled[]                    =   {0x03, 0x00, 0x38, 0x00, 0x01, 0x02, 0x00, 0x00};
unsigned char Power_Wh_Factor[]                             =   {0x03, 0x00, 0x39, 0x00, 0x01, 0x02, 0x00, 0x00};
unsigned char External_CT_Flag[]                            =   {0x03, 0x00, 0x19, 0x00, 0x01, 0x02, 0x00, 0x00};

/* =========================================================================
 * GPIO / pin macros
 * ========================================================================= */
// #define RI_DI_Pin               GPIO_PIN_NO_68
#define RS485_RE_DE_PIN    GPIO_PIN_NO_35
#define SET_RI_DI                RS485_TX_Enable()
#define RESET_RI_DI              RS485_RX_Enable()

// #define COMMUNICATION_LED_Pin   GPIO_PIN_NO_70
#define COMM_LED_ON              do {} while (0)
#define COMM_LED_OFF             do {} while (0)

#define DEVICE_ID                       0x01
#define INVERTER_RUN_STATE_NORMAL   2   /* check your inverter manual for the "running" value */
#define INVERTER_COMM_BUFFER_SIZE       20
#define RESPONSE_WAIT_TIME              1
#define MAX_INVERTER_CMD_SIZE           75
#define PARAM_UPDATE_RATE               200

/* =========================================================================
 * Data structure instances
 * ========================================================================= */
InverterData_Init        InverterData = {
    0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,
    0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,
    0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,
    0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,
    0,0,0,0
};
Inverter_Config_Var      Inverter_Config_Param = {0.0f};
Inverter_Basic_Detl      Inverter_Basic_Detail = {0,0,0,0,0,0,0,0.0};
Inverter_Output_Grid_Param Inverter_Output_Grid_Parameter = {
    0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,
    0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0
};

/* =========================================================================
 * Command ring buffer
 * ========================================================================= */
unsigned char Inverter_Cmd_head = 0;
unsigned char Inverter_Cmd_tail = 0;

unsigned char Inverter_Comm_Buffer[INVERTER_COMM_BUFFER_SIZE];
unsigned char Inverter_Comm_Buffer_Ndx = 0;

unsigned int DateTime_Str[10];
unsigned char data[] = "hello uart demo\r\n";

Inverter_Cmd_t Inverter_Cmd[MAX_INVERTER_CMD_SIZE];
int            Rx_Flag = 0;

unsigned long WarmUp_Last = 0, REDE_Stabilize_Last = 0;
unsigned long Response_Wait_Last = 0;
unsigned long cmd_last = 0, disp_last = 0;
unsigned int  CrcByte = 0;

/* =========================================================================
 * Shared state consumed by mqtt.c
 * ========================================================================= */
int CONFIG_WRITE_VAR         = 0;
int PARAM_CONFIG_MODE        = 0;
int WRITE_DATE_TIME          = 0;
int INVERTER_COMMUNICATION   = 0;
int RxFlag                   = 0;
int IsProcessDone = 0;


BOOL Push_Fault_Server    = FALSE;
BOOL Is_Process_Compelete = FALSE;
BOOL Print_Tx_Data        = FALSE;

unsigned int Total_Generation_L = 0;
unsigned int Total_Generation_H = 0;

char serial_number[SERIAL_NUMBER_LENGTH_INV + 1];
char Hex_String[10];
unsigned char RX_BUFFER[100];
char RX_CONFIG_VALUE[20];
char CONFIG_VAR_NAME[20];
char CONFIG_VAR_STR[100];
double CONFIG_VAR_VALUE = 0.0;

int Final_Fault     = 0;
int Prevoius_Fault  = 0;
int UPDATE_COUNT    = 0;
int CONFIG_PARAM_COUNT     = 0;
int CONFIG_PARAM_COUNT_MAX = 5;
int WAIT_RESPONSE_COUNT    = 1;
unsigned long int GTI_Date = 0;
unsigned long int GTI_Time = 0;

enum {
    SEND_CMD = 0,
    WAIT_FOR_RESPONSE = 1,
    PROCESS_RESPONSE  = 2
} CMD_STATES_t = SEND_CMD;

////declarationof_functuions

/* =========================================================================
 * decimalToBinary — find lowest set bit index (used by fault decoding).
 * ========================================================================= */
void RS485_GPIO_Init(void)
{
    ql_gpio_init(RS485_RE_DE_PIN,
                 PIN_DIRECTION_OUT,
                 PIN_PULL_DISABLE,
                 PIN_LEVEL_LOW);
}

void RS485_TX_Enable(void)
{
    ql_gpio_set_level(RS485_RE_DE_PIN, PIN_LEVEL_HIGH);
}

void RS485_RX_Enable(void)
{
    ql_gpio_set_level(RS485_RE_DE_PIN, PIN_LEVEL_LOW);
}
int decimalToBinary(unsigned long int num)
{
    int binary[32];
    int i = 0;

    if (num == 0) return 0;

    while (num > 0)
    {
        binary[i] = num % 2;
        num = num / 2;
        i++;
    }

    for (int j = 0; j < i; j++)
        if (binary[j] == 1) return j + 1;

    return 0;
}

/* =========================================================================
 * Modbus RX byte accumulator — unchanged from EC200.
 * Scans for 0x01 (slave address) start byte, collects up to buffer-1.
 * ========================================================================= */
void serial_int_ISR(unsigned char *c)
{
    if (*c == 0x01)
    {
        Rx_Flag = 1;
    }

    if (Rx_Flag == 1)
    {
        Inverter_Comm_Buffer[Inverter_Comm_Buffer_Ndx++] = *c;
        if (Inverter_Comm_Buffer_Ndx >= INVERTER_COMM_BUFFER_SIZE - 1)
        {
            Rx_Flag = 0;
            Inverter_Comm_Buffer_Ndx = 0;
        }
    }
}

/* =========================================================================
 * EG800 UART RX callback — signature (port, para), matches your demo.
 * ========================================================================= */
void ql_uart_notify_cb(QL_UART_PORT_NUMBER_E port, void *para)
{
    (void)para;

    char r_data[QL_UART_RX_BUFF_SIZE] = {0};
    int  read_len = 0;

    read_len = ql_uart_read(port, (unsigned char *)r_data, sizeof(r_data));

    if (read_len > 0)
    {
        printf("UART RX port=%d len=%d first=0x%02X",
                        port, read_len, (unsigned char)r_data[0]);

        for (int a = 0; a < read_len; a++)
        {
            serial_int_ISR((unsigned char *)&r_data[a]);
        }
    }
}

/* =========================================================================
 * inverter_app_thread — state machine.
 * ========================================================================= */
static void inverter_app_thread(void *arg)
{
    UART_Init();
    RS485_GPIO_Init();
    ql_rtos_task_sleep_ms(1000);

    while (1)
    {
        /* ---- Config-read (server asked for a parameter value) ---- */
        if (PARAM_CONFIG_MODE == 1 && Is_Process_Compelete == TRUE)
        {
            Inverter_Cmd_tail = 0;
            Inverter_Cmd_head = 0;
            Print_Tx_Data = TRUE;
            memset(Inverter_Cmd, 0, sizeof(Inverter_Cmd));

            unsigned char *SelectedVar_Read = getVariableFromEnum(CONFIG_WRITE_VAR);
            Write_Config_Param_R(SelectedVar_Read);
            PARAM_CONFIG_MODE = 0;
        }
        /* ---- Config-write (server sent a new parameter value) ---- */
        else if (PARAM_CONFIG_MODE == 2 && Is_Process_Compelete == TRUE)
        {
            Inverter_Cmd_tail = 0;
            Inverter_Cmd_head = 0;
            Print_Tx_Data = TRUE;
            memset(Inverter_Cmd, 0, sizeof(Inverter_Cmd));

            unsigned char *selectedVar = getVariableFromEnum(CONFIG_WRITE_VAR);
            if (selectedVar != NULL)
            {
                Write_Config_Param_W(selectedVar, RX_CONFIG_VALUE);
                ql_rtos_task_sleep_ms(10);
            }
        }
        /* ---- Normal polling: queue next batch periodically ---- */
        else
        {
            if (UPDATE_COUNT++ > PARAM_UPDATE_RATE && Inverter_Cmd_tail == Inverter_Cmd_head)
            {
                // if (CONFIG_PARAM_COUNT++ > CONFIG_PARAM_COUNT_MAX)
                // {
                //     CONFIG_PARAM_COUNT_MAX = 100;
                //     CONFIG_PARAM_COUNT = 0;
                //     // Write_Config_Parameter();
                // }
                // else
                // {
                    Write_Parameter();
                // }
                UPDATE_COUNT = 0;
            }
        }

        /* ---- Process command ring ---- */
        if (Inverter_Cmd_tail != Inverter_Cmd_head)
        {
            switch (CMD_STATES_t)
            {
                case SEND_CMD:
                {
                    Is_Process_Compelete = FALSE;
                    Inverter_Comm_Buffer_Ndx = 0;
                    SET_RI_DI;

                    if (PARAM_CONFIG_MODE == 2 && WRITE_DATE_TIME == 0)
                        CrcByte = ModRTU_CRC(DEVICE_ID, Inverter_Cmd[Inverter_Cmd_tail].ParamCmd, 8);
                    else if (PARAM_CONFIG_MODE == 2 && WRITE_DATE_TIME == 1)
                        CrcByte = ModRTU_CRC(DEVICE_ID, Inverter_Cmd[Inverter_Cmd_tail].ParamCmd, 12);
                    else
                        CrcByte = ModRTU_CRC(DEVICE_ID, Inverter_Cmd[Inverter_Cmd_tail].ParamCmd, 5);

                    Serial_Write(DEVICE_ID);
                    Serial_Write(Inverter_Cmd[Inverter_Cmd_tail].ParamCmd[0]);
                    Serial_Write(Inverter_Cmd[Inverter_Cmd_tail].ParamCmd[1]);
                    Serial_Write(Inverter_Cmd[Inverter_Cmd_tail].ParamCmd[2]);
                    Serial_Write(Inverter_Cmd[Inverter_Cmd_tail].ParamCmd[3]);
                    Serial_Write(Inverter_Cmd[Inverter_Cmd_tail].ParamCmd[4]);

                    if (PARAM_CONFIG_MODE == 2 && WRITE_DATE_TIME == 0)
                    {
                        Serial_Write(Inverter_Cmd[Inverter_Cmd_tail].ParamCmd[5]);
                        Serial_Write(Inverter_Cmd[Inverter_Cmd_tail].ParamCmd[6]);
                        Serial_Write(Inverter_Cmd[Inverter_Cmd_tail].ParamCmd[7]);
                    }
                    if (PARAM_CONFIG_MODE == 2 && WRITE_DATE_TIME == 1)
                    {
                        Serial_Write(Inverter_Cmd[Inverter_Cmd_tail].ParamCmd[5]);
                        Serial_Write(Inverter_Cmd[Inverter_Cmd_tail].ParamCmd[6]);
                        Serial_Write(Inverter_Cmd[Inverter_Cmd_tail].ParamCmd[7]);
                        Serial_Write(Inverter_Cmd[Inverter_Cmd_tail].ParamCmd[8]);
                        Serial_Write(Inverter_Cmd[Inverter_Cmd_tail].ParamCmd[9]);
                        Serial_Write(Inverter_Cmd[Inverter_Cmd_tail].ParamCmd[10]);
                        Serial_Write(Inverter_Cmd[Inverter_Cmd_tail].ParamCmd[11]);
                    }

                    Serial_Write((unsigned char)CrcByte);
                    Serial_Write(CrcByte >> 8);
                    ql_rtos_task_sleep_ms(10);
                    RESET_RI_DI;

                    Print_Tx_Data = FALSE;
                    CMD_STATES_t = WAIT_FOR_RESPONSE;
                    PARAM_CONFIG_MODE = 0;
                    WRITE_DATE_TIME = 0;
                    break;
                }

                case WAIT_FOR_RESPONSE:
                {
                    if (WAIT_RESPONSE_COUNT++ > RESPONSE_WAIT_TIME)
                    {
                        if (Inverter_Comm_Buffer_Ndx == 0)
                        {
                            Inverter_Cmd_tail += 1;
                            if (Inverter_Cmd_tail > MAX_INVERTER_CMD_SIZE - 1)
                                Inverter_Cmd_tail = 0;

                            CMD_STATES_t = SEND_CMD;
                            Is_Process_Compelete = TRUE;
                        }
                        else
                        {
                            CMD_STATES_t = PROCESS_RESPONSE;
                        }
                        WAIT_RESPONSE_COUNT = 0;
                    }
                    break;
                }

                case PROCESS_RESPONSE:
                {
                    inverter_parameter_setter(Inverter_Cmd[Inverter_Cmd_tail].ParamCmd,
                                              Inverter_Comm_Buffer);

                    Inverter_Cmd_tail += 1;
                    if (Inverter_Cmd_tail > MAX_INVERTER_CMD_SIZE - 1)
                        Inverter_Cmd_tail = 0;

                    Is_Process_Compelete = TRUE;
                    CMD_STATES_t = SEND_CMD;
                    break;
                }
            }
        }

        Controlle_Led();
        ql_rtos_task_sleep_ms(20);
    }

    ql_rtos_task_delete(NULL);
}

/* =========================================================================
 * Modbus CRC-16 (poly 0xA001, init 0xFFFF). id = slave address byte.
 * ========================================================================= */
unsigned int ModRTU_CRC(unsigned char id, const unsigned char *buf, unsigned int len)
{
    unsigned int crc = 0xFFFF;
    unsigned char pos = 0, i;

    crc ^= (unsigned int)id;
    for (i = 8; i != 0; i--)
    {
        if ((crc & 0x0001) != 0) { crc >>= 1; crc ^= 0xA001; }
        else                     { crc >>= 1; }
    }

    for (pos = 0; pos < len; pos++)
    {
        crc ^= (unsigned int)buf[pos];
        for (i = 8; i != 0; i--)
        {
            if ((crc & 0x0001) != 0) { crc >>= 1; crc ^= 0xA001; }
            else                     { crc >>= 1; }
        }
    }
    return crc;
}

/* =========================================================================
 * Serial_Write — single byte at a time (Modbus is tiny, no perf concern).
 * ========================================================================= */
void Serial_Write(unsigned char c)
{
    ql_uart_write(INVERTER_UART_PORT, &c, 1);
}

/* =========================================================================
 * push_to_ParamCmd_buffer — enqueue one command.
 * ========================================================================= */
void push_to_ParamCmd_buffer(Inverter_Cmd_t *cmd)
{
    unsigned char next = Inverter_Cmd_head + 1;
    if (next > MAX_INVERTER_CMD_SIZE - 1) next = 0;

    if (next == Inverter_Cmd_tail) return;    /* queue full */

    Inverter_Cmd[Inverter_Cmd_head].ParamCmd = cmd->ParamCmd;
    Inverter_Cmd_head = next;
}

/* =========================================================================
 * Write_Parameter — enqueue the periodic read batch (~60 commands).
 * ========================================================================= */
void Write_Parameter(void)
{
    Inverter_Cmd_t temp_cmd;

    // temp_cmd.ParamCmd = Run_State;                       push_to_ParamCmd_buffer(&temp_cmd);
    // temp_cmd.ParamCmd = Grid_A_Voltage;                  push_to_ParamCmd_buffer(&temp_cmd);
    // temp_cmd.ParamCmd = Grid_B_Voltage;                  push_to_ParamCmd_buffer(&temp_cmd);
    // temp_cmd.ParamCmd = Grid_C_Voltage;                  push_to_ParamCmd_buffer(&temp_cmd);
    // temp_cmd.ParamCmd = Grid_AB_Voltage;                 push_to_ParamCmd_buffer(&temp_cmd);
    // temp_cmd.ParamCmd = Grid_BC_Voltage;                 push_to_ParamCmd_buffer(&temp_cmd);
    // temp_cmd.ParamCmd = Grid_AC_Voltage;                 push_to_ParamCmd_buffer(&temp_cmd);
    // temp_cmd.ParamCmd = Grid_A_Current;                  push_to_ParamCmd_buffer(&temp_cmd);
    // temp_cmd.ParamCmd = Grid_B_Current;                  push_to_ParamCmd_buffer(&temp_cmd);
    // temp_cmd.ParamCmd = Grid_C_Current;                  push_to_ParamCmd_buffer(&temp_cmd);
    // temp_cmd.ParamCmd = Grid_Frequency;                  push_to_ParamCmd_buffer(&temp_cmd);

    temp_cmd.ParamCmd = DC_Voltage_1;                    push_to_ParamCmd_buffer(&temp_cmd);
    temp_cmd.ParamCmd = DC_Voltage_2;                    push_to_ParamCmd_buffer(&temp_cmd);
    // temp_cmd.ParamCmd = DC_Voltage_3;                    push_to_ParamCmd_buffer(&temp_cmd);
    // temp_cmd.ParamCmd = DC_Voltage_4;                    push_to_ParamCmd_buffer(&temp_cmd);
    temp_cmd.ParamCmd = DC_Current_1;                    push_to_ParamCmd_buffer(&temp_cmd);
    temp_cmd.ParamCmd = DC_Current_2;                    push_to_ParamCmd_buffer(&temp_cmd);
    // temp_cmd.ParamCmd = DC_Current_3;                    push_to_ParamCmd_buffer(&temp_cmd);
    // temp_cmd.ParamCmd = DC_Current_4;                    push_to_ParamCmd_buffer(&temp_cmd);

    // temp_cmd.ParamCmd = DC_Power_1;                      push_to_ParamCmd_buffer(&temp_cmd);
    // temp_cmd.ParamCmd = DC_Power_2;                      push_to_ParamCmd_buffer(&temp_cmd);
    // temp_cmd.ParamCmd = DC_Power_3;                      push_to_ParamCmd_buffer(&temp_cmd);
    // temp_cmd.ParamCmd = DC_Power_4;                      push_to_ParamCmd_buffer(&temp_cmd);

    // temp_cmd.ParamCmd = Today_Gen;                       push_to_ParamCmd_buffer(&temp_cmd);
    // temp_cmd.ParamCmd = Total_Gen_L;                     push_to_ParamCmd_buffer(&temp_cmd);
    // temp_cmd.ParamCmd = Total_Gen_H;                     push_to_ParamCmd_buffer(&temp_cmd);

    // temp_cmd.ParamCmd = Day_Active_Power;                push_to_ParamCmd_buffer(&temp_cmd);
    // temp_cmd.ParamCmd = Day_Reactive_Power;              push_to_ParamCmd_buffer(&temp_cmd);
    // temp_cmd.ParamCmd = Total_Active_Power;              push_to_ParamCmd_buffer(&temp_cmd);
    // temp_cmd.ParamCmd = Total_Rective_Power;             push_to_ParamCmd_buffer(&temp_cmd);

    // temp_cmd.ParamCmd = Input_Active_Power;              push_to_ParamCmd_buffer(&temp_cmd);
    // temp_cmd.ParamCmd = Output_Apparent_Power;           push_to_ParamCmd_buffer(&temp_cmd);
    // temp_cmd.ParamCmd = Output_Active_Power;             push_to_ParamCmd_buffer(&temp_cmd);
    // temp_cmd.ParamCmd = Output_Reactive_Power;           push_to_ParamCmd_buffer(&temp_cmd);

    // temp_cmd.ParamCmd = Total_Work_Time;                 push_to_ParamCmd_buffer(&temp_cmd);
    // temp_cmd.ParamCmd = RadiatorT_emperature;            push_to_ParamCmd_buffer(&temp_cmd);
    // temp_cmd.ParamCmd = IGBT_Temperatur;                 push_to_ParamCmd_buffer(&temp_cmd);
    // temp_cmd.ParamCmd = Power_Factor;                    push_to_ParamCmd_buffer(&temp_cmd);

    // temp_cmd.ParamCmd = History_PV_Power;                push_to_ParamCmd_buffer(&temp_cmd);

    // temp_cmd.ParamCmd = Warning_Msg_Ward_1;              push_to_ParamCmd_buffer(&temp_cmd);
    // temp_cmd.ParamCmd = Warning_Msg_Ward_2;              push_to_ParamCmd_buffer(&temp_cmd);

    // temp_cmd.ParamCmd = Fault_Ward_1;                    push_to_ParamCmd_buffer(&temp_cmd);
    // temp_cmd.ParamCmd = Fault_Ward_2;                    push_to_ParamCmd_buffer(&temp_cmd);
    // temp_cmd.ParamCmd = Fault_Ward_3;                    push_to_ParamCmd_buffer(&temp_cmd);
    // temp_cmd.ParamCmd = Fault_Ward_4;                    push_to_ParamCmd_buffer(&temp_cmd);

    // temp_cmd.ParamCmd = Date_Time;                       push_to_ParamCmd_buffer(&temp_cmd);

    // /* Basic inverter detail */
    // temp_cmd.ParamCmd = Inverter_Type;                   push_to_ParamCmd_buffer(&temp_cmd);
    // temp_cmd.ParamCmd = Product_Type;                    push_to_ParamCmd_buffer(&temp_cmd);
    // temp_cmd.ParamCmd = Firm_Ver_Control_Bord;           push_to_ParamCmd_buffer(&temp_cmd);
    // temp_cmd.ParamCmd = Firm_Ver_Comm_Bord;              push_to_ParamCmd_buffer(&temp_cmd);
    // temp_cmd.ParamCmd = Rated_Power;                     push_to_ParamCmd_buffer(&temp_cmd);
    // temp_cmd.ParamCmd = MPPT_Phase_No;                   push_to_ParamCmd_buffer(&temp_cmd);
    // temp_cmd.ParamCmd = Rated_Grid_Voltage_Get;          push_to_ParamCmd_buffer(&temp_cmd);

    // /* Grid & output/load parameters */
    // temp_cmd.ParamCmd = Inverter_Output_Power_L1;        push_to_ParamCmd_buffer(&temp_cmd);
    // temp_cmd.ParamCmd = Inverter_Output_Power_L2;        push_to_ParamCmd_buffer(&temp_cmd);
    // temp_cmd.ParamCmd = Day_Load_Power;                  push_to_ParamCmd_buffer(&temp_cmd);
    // temp_cmd.ParamCmd = Cumulative_Load_Power;           push_to_ParamCmd_buffer(&temp_cmd);
    // temp_cmd.ParamCmd = Daily_Grid_Feed_In;              push_to_ParamCmd_buffer(&temp_cmd);
    // temp_cmd.ParamCmd = Cumulative_Grid_Feed_In;         push_to_ParamCmd_buffer(&temp_cmd);
    // temp_cmd.ParamCmd = Daily_Energy_Purchase;           push_to_ParamCmd_buffer(&temp_cmd);
    // temp_cmd.ParamCmd = Cumulative_Energy_Purchase;      push_to_ParamCmd_buffer(&temp_cmd);

    // temp_cmd.ParamCmd = Comm_Protocal;                   push_to_ParamCmd_buffer(&temp_cmd);
    // temp_cmd.ParamCmd = Inverter_Serial;                 push_to_ParamCmd_buffer(&temp_cmd);

    // /* External CT / meter power */
    // temp_cmd.ParamCmd = Grid_Power_L1;                   push_to_ParamCmd_buffer(&temp_cmd);
    // temp_cmd.ParamCmd = Grid_Power_L2;                   push_to_ParamCmd_buffer(&temp_cmd);
    // temp_cmd.ParamCmd = Limiter_Power_L1;                push_to_ParamCmd_buffer(&temp_cmd);
    // temp_cmd.ParamCmd = Limiter_Power_L2;                push_to_ParamCmd_buffer(&temp_cmd);
    // temp_cmd.ParamCmd = Load_Power_L1;                   push_to_ParamCmd_buffer(&temp_cmd);
    // temp_cmd.ParamCmd = Load_Power_L2;                   push_to_ParamCmd_buffer(&temp_cmd);
    // temp_cmd.ParamCmd = Total_Invter_Out_Power;          push_to_ParamCmd_buffer(&temp_cmd);
    // temp_cmd.ParamCmd = Toal_Grid_External_Power;        push_to_ParamCmd_buffer(&temp_cmd);
    // temp_cmd.ParamCmd = Total_Load_Power;                push_to_ParamCmd_buffer(&temp_cmd);

    // temp_cmd.ParamCmd = Meter_Enable_Read;               push_to_ParamCmd_buffer(&temp_cmd);
}

/* =========================================================================
 * Write_Config_Parameter — enqueue the slower config-read batch.
 * ========================================================================= */
void Write_Config_Parameter(void)
{
    Inverter_Cmd_t temp_cmd;

    temp_cmd.ParamCmd = DC_Voltage_Upper_Limit;                         push_to_ParamCmd_buffer(&temp_cmd);
    temp_cmd.ParamCmd = Grid_Voltage_Upper_Limit;                       push_to_ParamCmd_buffer(&temp_cmd);
    temp_cmd.ParamCmd = Grid_Voltage_Upper_Limit_L2;                    push_to_ParamCmd_buffer(&temp_cmd);
    temp_cmd.ParamCmd = Grid_Voltage_Upper_Limit_L3;                    push_to_ParamCmd_buffer(&temp_cmd);
    temp_cmd.ParamCmd = Grid_Voltage_Lower_Limit;                       push_to_ParamCmd_buffer(&temp_cmd);
    temp_cmd.ParamCmd = Grid_Voltage_Lower_Limit_L2;                    push_to_ParamCmd_buffer(&temp_cmd);
    temp_cmd.ParamCmd = Grid_Voltage_Lower_Limit_L3;                    push_to_ParamCmd_buffer(&temp_cmd);
    temp_cmd.ParamCmd = Grid_Current_Upper_Limit;                       push_to_ParamCmd_buffer(&temp_cmd);
    temp_cmd.ParamCmd = Grid_Freq_Upper_Limit;                          push_to_ParamCmd_buffer(&temp_cmd);
    temp_cmd.ParamCmd = Grid_Freq_Upper_Limit_L2;                       push_to_ParamCmd_buffer(&temp_cmd);
    temp_cmd.ParamCmd = Grid_Freq_Upper_Limit_L3;                       push_to_ParamCmd_buffer(&temp_cmd);
    temp_cmd.ParamCmd = Grid_Freq_Lower_Limit;                          push_to_ParamCmd_buffer(&temp_cmd);
    temp_cmd.ParamCmd = Grid_Freq_Lower_Limit_L2;                       push_to_ParamCmd_buffer(&temp_cmd);
    temp_cmd.ParamCmd = Grid_Freq_Lower_Limit_L3;                       push_to_ParamCmd_buffer(&temp_cmd);
    temp_cmd.ParamCmd = Rated_Grid_Voltage_Set;                         push_to_ParamCmd_buffer(&temp_cmd);

    temp_cmd.ParamCmd = Grid_Voltage_Upper_Limit_Trip_Time_1OD;         push_to_ParamCmd_buffer(&temp_cmd);
    temp_cmd.ParamCmd = Grid_Voltage_Upper_Limit_Trip_Time_2OD;         push_to_ParamCmd_buffer(&temp_cmd);
    temp_cmd.ParamCmd = Grid_Voltage_Upper_Limit_Trip_Time_3OD;         push_to_ParamCmd_buffer(&temp_cmd);
    temp_cmd.ParamCmd = Grid_Voltage_Lower_Limit_Trip_Time_1OD;         push_to_ParamCmd_buffer(&temp_cmd);
    temp_cmd.ParamCmd = Grid_Voltage_Lower_Limit_Trip_Time_2OD;         push_to_ParamCmd_buffer(&temp_cmd);
    temp_cmd.ParamCmd = Grid_Voltage_Lower_Limit_Trip_Time_3OD;         push_to_ParamCmd_buffer(&temp_cmd);
    temp_cmd.ParamCmd = Grid_Freq_Upper_Limit_Trip_Time_1OD;            push_to_ParamCmd_buffer(&temp_cmd);
    temp_cmd.ParamCmd = Grid_Freq_Upper_Limit_Trip_Time_2OD;            push_to_ParamCmd_buffer(&temp_cmd);
    temp_cmd.ParamCmd = Grid_Freq_Upper_Limit_Trip_Time_3OD;            push_to_ParamCmd_buffer(&temp_cmd);
    temp_cmd.ParamCmd = Grid_Freq_Lower_Limit_Trip_Time_1OD;            push_to_ParamCmd_buffer(&temp_cmd);
    temp_cmd.ParamCmd = Grid_Freq_Lower_Limit_Trip_Time_2OD;            push_to_ParamCmd_buffer(&temp_cmd);
    temp_cmd.ParamCmd = Grid_Freq_Lower_Limit_Trip_Time_3OD;            push_to_ParamCmd_buffer(&temp_cmd);

    temp_cmd.ParamCmd = Remote_Lock;                                    push_to_ParamCmd_buffer(&temp_cmd);
    temp_cmd.ParamCmd = Self_Test_Time;                                 push_to_ParamCmd_buffer(&temp_cmd);
    temp_cmd.ParamCmd = Starting_Voltage_Upper_Limit;                   push_to_ParamCmd_buffer(&temp_cmd);
    temp_cmd.ParamCmd = Starting_Voltage_Lower_Limit;                   push_to_ParamCmd_buffer(&temp_cmd);
    temp_cmd.ParamCmd = Over_Frequency_Drate_Point;                     push_to_ParamCmd_buffer(&temp_cmd);
    temp_cmd.ParamCmd = Over_Frequency_Drate;                           push_to_ParamCmd_buffer(&temp_cmd);
    temp_cmd.ParamCmd = Internal_Temp_Upper_Limit;                      push_to_ParamCmd_buffer(&temp_cmd);
    temp_cmd.ParamCmd = Power_Factor_Regulation;                        push_to_ParamCmd_buffer(&temp_cmd);
    temp_cmd.ParamCmd = Active_Power_Regulation;                        push_to_ParamCmd_buffer(&temp_cmd);
    temp_cmd.ParamCmd = Reactive_Power_Regulation;                      push_to_ParamCmd_buffer(&temp_cmd);
    temp_cmd.ParamCmd = Apparent_Power_Regulation;                      push_to_ParamCmd_buffer(&temp_cmd);
    temp_cmd.ParamCmd = Switch_ON_OFF_Enable;                           push_to_ParamCmd_buffer(&temp_cmd);
    temp_cmd.ParamCmd = Factory_Reset_Enable;                           push_to_ParamCmd_buffer(&temp_cmd);
    temp_cmd.ParamCmd = Self_Checking_Time;                             push_to_ParamCmd_buffer(&temp_cmd);
    temp_cmd.ParamCmd = Island_Protection_Enable;                       push_to_ParamCmd_buffer(&temp_cmd);
    temp_cmd.ParamCmd = Meter_Enable;                                   push_to_ParamCmd_buffer(&temp_cmd);
    temp_cmd.ParamCmd = RCD_Enable;                                     push_to_ParamCmd_buffer(&temp_cmd);
    temp_cmd.ParamCmd = RISO_Enable;                                    push_to_ParamCmd_buffer(&temp_cmd);
    temp_cmd.ParamCmd = Grid_Standard;                                  push_to_ParamCmd_buffer(&temp_cmd);
    temp_cmd.ParamCmd = PV_Curved_Enable;                               push_to_ParamCmd_buffer(&temp_cmd);
    temp_cmd.ParamCmd = Eeprome_Initial_Enabled;                        push_to_ParamCmd_buffer(&temp_cmd);
    temp_cmd.ParamCmd = Limiter_Function_Enabled;                       push_to_ParamCmd_buffer(&temp_cmd);
    temp_cmd.ParamCmd = Power_Wh_Factor;                                push_to_ParamCmd_buffer(&temp_cmd);
    temp_cmd.ParamCmd = External_CT_Flag;                               push_to_ParamCmd_buffer(&temp_cmd);
}

/* =========================================================================
 * inverter_parameter_setter — decode the Modbus response into the data
 * structures consumed by mqtt.c. Unchanged logic from EC200; only the
 * port constant for UART reads/writes changed.
 * ========================================================================= */
void inverter_parameter_setter(const unsigned char *Re_cmd, unsigned char *Resp)
{
    unsigned int      Param_Address = ((Re_cmd[1] << 8) + Re_cmd[2]);
    unsigned long int Data = 0;
    unsigned int      Word_1 = 0;
    unsigned int      Word_2 = 0;


    /* Modbus RTU response: header (3 bytes), payload, and CRC (2 bytes). */
    for (int i = 0; i < (int)Resp[2] + 5; i++)
    {
        printf("Resp[%d] = 0x%02X\n", i, Resp[i]);
    }

    if (Resp[2] == 2)
    {
        Data = (unsigned long)Resp[4] << 8 | (unsigned long)Resp[3];
    }

    if (Resp[2] == 4)
    {
        Word_1 = (Resp[3] << 8) | Resp[4];
        Word_2 = (Resp[5] << 8) | Resp[6];
        Data = ((unsigned long)Word_1 << 16) | Word_2;
    }

    if (Resp[2] == 6)
    {
        DateTime_Str[0] = (unsigned long)Resp[3];
        DateTime_Str[1] = (unsigned long)Resp[4];
        DateTime_Str[2] = (unsigned long)Resp[5];
        DateTime_Str[3] = (unsigned long)Resp[6];
        DateTime_Str[4] = (unsigned long)Resp[7];
        DateTime_Str[5] = (unsigned long)Resp[8];

        snprintf(INV_DATE, sizeof(INV_DATE), "20%02u-%02u-%02u",
                 (unsigned int)(DateTime_Str[0] % 100),
                 (unsigned int)(DateTime_Str[1] % 100),
                 (unsigned int)(DateTime_Str[2] % 100));
        snprintf(INV_TIME, sizeof(INV_TIME), "%02u:%02u:%02u",
                 (unsigned int)(DateTime_Str[3] % 100),
                 (unsigned int)(DateTime_Str[4] % 100),
                 (unsigned int)(DateTime_Str[5] % 100));
        printf("APP DATE:%s  APP TIME:%s", INV_DATE, INV_TIME);
    }

    printf("===Data::%ld===", Data);

    if (Resp[2] == 10 || Resp[2] == 0x0A)
    {
        extract_serial_number(Resp, SERIAL_NUMBER_LENGTH_INV);
    }

    /* ---- Run state ---- */
    if (Param_Address == ((Run_State[1] << 8) + Run_State[2]))
    {
        InverterData.RunState = Data;
        if (InverterData.RunState == 0.0 || InverterData.RunState == 1.0 ||
            InverterData.RunState == 2.0 || InverterData.RunState == 3.0 ||
            InverterData.RunState == 4.0)
        {
            INVERTER_COMMUNICATION = 1;
            Inverter_Disconnect_Count = 0;
            COMM_LED_ON;
        }
        printf("Run State:%0.2f", InverterData.RunState);
    }

    if (Param_Address == ((Date_Time[1] << 8) + Date_Time[2]))
    {
        printf("Date String:%lu", GTI_Date);
    }

    /* ---- Grid voltages ---- */
    if (Param_Address == ((Grid_A_Voltage[1] << 8) + Grid_A_Voltage[2]))
    {
        InverterData.GridVoltageA = Data / 10.0;
        printf("Grid Voltage A:%0.2f", InverterData.GridVoltageA);
    }
    if (Param_Address == ((Grid_B_Voltage[1] << 8) + Grid_B_Voltage[2]))
    {
        InverterData.GridVoltageB = Data / 10.0;
    }
    if (Param_Address == ((Grid_C_Voltage[1] << 8) + Grid_C_Voltage[2]))
    {
        InverterData.GridVoltageC = Data / 10.0;
    }
    if (Param_Address == ((Grid_AB_Voltage[1] << 8) + Grid_AB_Voltage[2]))
    {
        InverterData.GridVoltageAB = Data / 10.0;
    }
    if (Param_Address == ((Grid_BC_Voltage[1] << 8) + Grid_BC_Voltage[2]))
    {
        InverterData.GridVoltageBC = Data / 10.0;
    }
    if (Param_Address == ((Grid_AC_Voltage[1] << 8) + Grid_AC_Voltage[2]))
    {
        InverterData.GridVoltageAC = Data / 10.0;
    }

    /* ---- Grid currents ---- */
    if (Param_Address == ((Grid_A_Current[1] << 8) + Grid_A_Current[2]))
    {
        InverterData.GridCurrentA = Data / 10.0;
        printf("Grid Current A:%0.2f", InverterData.GridCurrentA);
    }
    if (Param_Address == ((Grid_B_Current[1] << 8) + Grid_B_Current[2]))
    {
        InverterData.GridCurrentB = Data / 10.0;
    }
    if (Param_Address == ((Grid_C_Current[1] << 8) + Grid_C_Current[2]))
    {
        InverterData.GridCurrentC = Data / 10.0;
    }
    if (Param_Address == ((Grid_Frequency[1] << 8) + Grid_Frequency[2]))
    {
        InverterData.GridFrequency = Data / 100.0;
        printf("Grid Frequency:%0.2f", InverterData.GridFrequency);
    }

    /* ---- DC voltages ---- */
    if (Param_Address == ((DC_Voltage_1[1] << 8) + DC_Voltage_1[2]))
    {
        InverterData.DCVoltage1 = Data / 10.0;
        printf("DC Voltage1:%0.2f", InverterData.DCVoltage1);
    }
    if (Param_Address == ((DC_Voltage_2[1] << 8) + DC_Voltage_2[2]))
    {
        InverterData.DCVoltage2 = Data / 10.0;
        printf("DC Voltage2:%0.2f", InverterData.DCVoltage2);
    }
    if (Param_Address == ((DC_Voltage_3[1] << 8) + DC_Voltage_3[2]))
    {
        InverterData.DCVoltage3 = Data / 10.0;
        printf("DC Voltage3:%0.2f", InverterData.DCVoltage3);
    }
    if (Param_Address == ((DC_Voltage_4[1] << 8) + DC_Voltage_4[2]))
    {
        InverterData.DCVoltage4 = Data / 10.0;
        printf("DC Voltage4:%0.2f", InverterData.DCVoltage4);
    }

    /* ---- DC currents ---- */
    if (Param_Address == ((DC_Current_1[1] << 8) + DC_Current_1[2]))
    {
        InverterData.DCCurrent1 = Data / 10.0;
        printf("DC Current1:%0.2f", InverterData.DCCurrent1);
        
    }
    if (Param_Address == ((DC_Current_2[1] << 8) + DC_Current_2[2]))
    {
        InverterData.DCCurrent2 = Data / 10.0;
        printf("DC Current2:%0.2f", InverterData.DCCurrent2);
    }
    if (Param_Address == ((DC_Current_3[1] << 8) + DC_Current_3[2]))
    {
        InverterData.DCCurrent3 = Data / 10.0;
        printf("DC Current3:%0.2f", InverterData.DCCurrent3);
    }
    if (Param_Address == ((DC_Current_4[1] << 8) + DC_Current_4[2]))
    {
        InverterData.DCCurrent4 = Data / 10.0;
        printf("DC Current4:%0.2f", InverterData.DCCurrent4);
    }

    /* ---- DC power (fallback = V*I when register is stale) ---- */
    if (Param_Address == ((DC_Power_1[1] << 8) + DC_Power_1[2]))
    {
        InverterData.DCPower1 = Data;
        if (InverterData.DCPower1 < 10)
            InverterData.DCPower1 = InverterData.DCVoltage1 * InverterData.DCCurrent1;
        InverterData.DCPower1 = InverterData.DCPower1 / 1000.0;
        printf("DC_Power_1:%0.2f", InverterData.DCPower1);
    }
    if (Param_Address == ((DC_Power_2[1] << 8) + DC_Power_2[2]))
    {
        InverterData.DCPower2 = Data;
        if (InverterData.DCPower2 < 10)
            InverterData.DCPower2 = InverterData.DCVoltage2 * InverterData.DCCurrent2;
        InverterData.DCPower2 = InverterData.DCPower2 / 1000.0;
        printf("DC_Power_2:%0.2f", InverterData.DCPower2);
    }
    if (Param_Address == ((DC_Power_3[1] << 8) + DC_Power_3[2]))
    {
        InverterData.DCPower3 = Data;
        if (InverterData.DCPower3 < 10)
            InverterData.DCPower3 = InverterData.DCVoltage3 * InverterData.DCCurrent3;
        InverterData.DCPower3 = InverterData.DCPower3 / 1000.0;
        printf("DC_Power_3:%0.2f", InverterData.DCPower3);
    }
    if (Param_Address == ((DC_Power_4[1] << 8) + DC_Power_4[2]))
    {
        InverterData.DCPower4 = Data;
        if (InverterData.DCPower4 < 10)
            InverterData.DCPower4 = InverterData.DCVoltage4 * InverterData.DCCurrent4;
        InverterData.DCPower4 = InverterData.DCPower4 / 1000.0;
        printf("DC_Power_4:%0.2f", InverterData.DCPower4);
    }

    /* ---- Generation energy ---- */
    if (Param_Address == ((Today_Gen[1] << 8) + Today_Gen[2]))
    {
        InverterData.TodayGeneratedEnergy = Data / 10.0;
    }
    if (Param_Address == ((Total_Gen_L[1] << 8) + Total_Gen_L[2]))
    {
        Total_Generation_L = Data;
    }
    if (Param_Address == ((Total_Gen_H[1] << 8) + Total_Gen_H[2]))
    {
        Total_Generation_H = Data;
        InverterData.CumulativeGeneratedEnergy =
            (Total_Generation_H << 16) | Total_Generation_L;
        InverterData.CumulativeGeneratedEnergy /= 10.0;
    }

    /* ---- Power / reactive / active ---- */
    if (Param_Address == ((Day_Active_Power[1] << 8) + Day_Active_Power[2]))
    {
        InverterData.DayActivePower = Data / 10.0;
    }
    if (Param_Address == ((Day_Reactive_Power[1] << 8) + Day_Reactive_Power[2]))
    {
        InverterData.DayReactivePower = Data / 10.0;
    }
    if (Param_Address == ((Total_Active_Power[1] << 8) + Total_Active_Power[2]))
    {
        InverterData.TotalActivePower = Data / 10.0;
    }
    if (Param_Address == ((Total_Rective_Power[1] << 8) + Total_Rective_Power[2]))
    {
        InverterData.TotalReactivePower = Data / 10.0;
    }

    if (Param_Address == ((Input_Active_Power[1] << 8) + Input_Active_Power[2]))
    {
        InverterData.InputActivePower = Data / 10000.0;
    }
    if (Param_Address == ((Output_Apparent_Power[1] << 8) + Output_Apparent_Power[2]))
    {
        InverterData.OutputApparentPower = Data / 10000.0;
    }
    if (Param_Address == ((Output_Active_Power[1] << 8) + Output_Active_Power[2]))
    {
        InverterData.OutputActivePower = Data / 10000.0;
    }
    if (Param_Address == ((Output_Reactive_Power[1] << 8) + Output_Reactive_Power[2]))
    {
        InverterData.OutputReactivePower = Data / 10000.0;
    }

    if (Param_Address == ((Total_Work_Time[1] << 8) + Total_Work_Time[2]))
    {
        /* Total work time not currently published; placeholder if needed. */
    }

    /* ---- Temperatures & power factor ---- */
    if (Param_Address == ((RadiatorT_emperature[1] << 8) + RadiatorT_emperature[2]))
    {
        InverterData.RadiatorTemperature = Data / 10.0;
    }
    if (Param_Address == ((IGBT_Temperatur[1] << 8) + IGBT_Temperatur[2]))
    {
        InverterData.IGBTtemperatur = Data / 10.0;
    }
    if (Param_Address == ((Power_Factor[1] << 8) + Power_Factor[2]))
    {
        InverterData.PowerFactor = Data / 10.0;
    }
    if (Param_Address == ((History_PV_Power[1] << 8) + History_PV_Power[2]))
    {
        InverterData.HistoryPvPower = Data / 10.0;
    }

    /* ---- Warnings ---- */
    if (Param_Address == ((Warning_Msg_Ward_1[1] << 8) + Warning_Msg_Ward_1[2]))
    {
        InverterData.Warning = Data;
    }
    if (Param_Address == ((Warning_Msg_Ward_2[1] << 8) + Warning_Msg_Ward_2[2]))
    {
        InverterData.Warning = Data;
    }

    /* ---- Faults ---- */
    if (Param_Address == ((Fault_Ward_1[1] << 8) + Fault_Ward_1[2]))
    {
        InverterData.Fault1 = decimalToBinary(Data);
    }
    if (Param_Address == ((Fault_Ward_2[1] << 8) + Fault_Ward_2[2]))
    {
        InverterData.Fault2 = decimalToBinary(Data);
    }
    if (Param_Address == ((Fault_Ward_3[1] << 8) + Fault_Ward_3[2]))
    {
        InverterData.Fault3 = decimalToBinary(Data);
    }
    if (Param_Address == ((Fault_Ward_4[1] << 8) + Fault_Ward_4[2]))
    {
        InverterData.Fault4 = decimalToBinary(Data);
    }

    if      (InverterData.Fault1) Final_Fault = InverterData.Fault1;
    else if (InverterData.Fault2) Final_Fault = InverterData.Fault2 + 16;
    else if (InverterData.Fault3) Final_Fault = InverterData.Fault3 + 32;
    else if (InverterData.Fault4) Final_Fault = InverterData.Fault4 + 48;
    else                          Final_Fault = 0;

    if (Final_Fault != Prevoius_Fault)
    {
        Prevoius_Fault = Final_Fault;
        Push_Fault_Server = TRUE;
    }

    /* ---- Basic inverter detail ---- */
    if (Param_Address == ((Inverter_Type[1] << 8) + Inverter_Type[2]))
    {
        Inverter_Basic_Detail.Inverter_Type = Data;
    }
    if (Param_Address == ((Product_Type[1] << 8) + Product_Type[2]))
    {
        Inverter_Basic_Detail.Product_Type = Data;
    }
    if (Param_Address == ((Comm_Protocal[1] << 8) + Comm_Protocal[2]))
    {
        Inverter_Basic_Detail.Protocal_Version = Data;
        memset(Hex_String, '\0', 10);
        sprintf(Hex_String, "%x", Inverter_Basic_Detail.Protocal_Version);
        Inverter_Basic_Detail.Protocal_Version = atoi(Hex_String);
    }
    if (Param_Address == ((Firm_Ver_Control_Bord[1] << 8) + Firm_Ver_Control_Bord[2]))
    {
        Inverter_Basic_Detail.Firmware_Ver_Control_Bord = Data;
        memset(Hex_String, '\0', 10);
        sprintf(Hex_String, "%x", Inverter_Basic_Detail.Firmware_Ver_Control_Bord);
        Inverter_Basic_Detail.Firmware_Ver_Control_Bord = atoi(Hex_String);
    }
    if (Param_Address == ((Firm_Ver_Comm_Bord[1] << 8) + Firm_Ver_Comm_Bord[2]))
    {
        Inverter_Basic_Detail.Firmware_Ver_Communication_Bord = Data;
        memset(Hex_String, '\0', 10);
        sprintf(Hex_String, "%x", Inverter_Basic_Detail.Firmware_Ver_Communication_Bord);
        Inverter_Basic_Detail.Firmware_Ver_Communication_Bord = atoi(Hex_String);
    }
    if (Param_Address == ((Rated_Power[1] << 8) + Rated_Power[2]))
    {
        Inverter_Basic_Detail.Rated_Power = Data / 10000.0;
    }
    if (Param_Address == ((MPPT_Phase_No[1] << 8) + MPPT_Phase_No[2]))
    {
        Inverter_Basic_Detail.MPPT_Phase_No = Data;
        memset(Hex_String, '\0', 10);
        sprintf(Hex_String, "%x", Inverter_Basic_Detail.MPPT_Phase_No);
        Inverter_Basic_Detail.MPPT_Phase_No = atoi(Hex_String);
    }
    if (Param_Address == ((Rated_Grid_Voltage_Get[1] << 8) + Rated_Grid_Voltage_Get[2]))
    {
        Inverter_Basic_Detail.Rated_Grid_Voltage_Get = Data;
    }

    /* ---- Grid/output/load parameters ---- */
    if (Param_Address == ((Inverter_Output_Power_L1[1] << 8) + Inverter_Output_Power_L1[2]))
    {
        Inverter_Output_Grid_Parameter.Inverter_Output_Power_L1 = Data / 10.0;
    }
    if (Param_Address == ((Inverter_Output_Power_L2[1] << 8) + Inverter_Output_Power_L2[2]))
    {
        Inverter_Output_Grid_Parameter.Inverter_Output_Power_L2 = Data / 10.0;
    }
    if (Param_Address == ((Day_Load_Power[1] << 8) + Day_Load_Power[2]))
    {
        Inverter_Output_Grid_Parameter.Day_Load_Power = Data / 100.0;
    }
    if (Param_Address == ((Cumulative_Load_Power[1] << 8) + Cumulative_Load_Power[2]))
    {
        Inverter_Output_Grid_Parameter.Cumulative_Load_Power = Data / 10.0;
    }
    if (Param_Address == ((Daily_Grid_Feed_In[1] << 8) + Daily_Grid_Feed_In[2]))
    {
        Inverter_Output_Grid_Parameter.Daily_Grid_Feed_In = Data / 10.0;
    }
    if (Param_Address == ((Cumulative_Grid_Feed_In[1] << 8) + Cumulative_Grid_Feed_In[2]))
    {
        Inverter_Output_Grid_Parameter.Cumulative_Grid_Feed_In = Data / 10.0;
    }
    if (Param_Address == ((Daily_Energy_Purchase[1] << 8) + Daily_Energy_Purchase[2]))
    {
        Inverter_Output_Grid_Parameter.Daily_Energy_Purchase = Data / 10.0;
    }
    if (Param_Address == ((Cumulative_Energy_Purchase[1] << 8) + Cumulative_Energy_Purchase[2]))
    {
        Inverter_Output_Grid_Parameter.Cumulative_Energy_Purchase = Data / 10.0;
    }

    if (Param_Address == ((Grid_Power_L1[1] << 8) + Grid_Power_L1[2]))
    {
        Inverter_Output_Grid_Parameter.Grid_Power_L1 = Data / 1000.0;
        printf("Grid_Power_L1:%0.2f", Inverter_Output_Grid_Parameter.Grid_Power_L1);
    }
    if (Param_Address == ((Grid_Power_L2[1] << 8) + Grid_Power_L2[2]))
    {
        Inverter_Output_Grid_Parameter.Grid_Power_L2 = Data / 1000.0;
        printf("Grid_Power_L2:%0.2f", Inverter_Output_Grid_Parameter.Grid_Power_L2);
    }
    if (Param_Address == ((Limiter_Power_L1[1] << 8) + Limiter_Power_L1[2]))
    {
        Inverter_Output_Grid_Parameter.Limiter_Power_L1 = Data / 1000.0;
        printf("Limiter_Power_L1:%0.2f", Inverter_Output_Grid_Parameter.Limiter_Power_L1);
    }
    if (Param_Address == ((Limiter_Power_L2[1] << 8) + Limiter_Power_L2[2]))
    {
        Inverter_Output_Grid_Parameter.Limiter_Power_L2 = Data / 1000.0;
        printf("Limiter_Power_L2:%0.2f", Inverter_Output_Grid_Parameter.Limiter_Power_L2);
    }
    if (Param_Address == ((Load_Power_L1[1] << 8) + Load_Power_L1[2]))
    {
        Inverter_Output_Grid_Parameter.Load_Power_L1 = Data / 1000.0;
        printf("Load_Power_L1:%0.2f", Inverter_Output_Grid_Parameter.Load_Power_L1);
    }
    if (Param_Address == ((Load_Power_L2[1] << 8) + Load_Power_L2[2]))
    {
        Inverter_Output_Grid_Parameter.Load_Power_L2 = Data / 1000.0;
        printf("Load_Power_L2:%0.2f", Inverter_Output_Grid_Parameter.Load_Power_L2);
    }
    if (Param_Address == ((Total_Invter_Out_Power[1] << 8) + Total_Invter_Out_Power[2]))
    {
        Inverter_Output_Grid_Parameter.Total_Invt_Output_Power = Data / 1000.0;
        printf("Total_Invter_Out_Power:%0.2f", Inverter_Output_Grid_Parameter.Total_Invt_Output_Power);
    }
    if (Param_Address == ((Toal_Grid_External_Power[1] << 8) + Toal_Grid_External_Power[2]))
    {
        Inverter_Output_Grid_Parameter.Total_Grd_External_Power = Data / 1000.0;
        printf("Toal_Grid_External_Power:%0.2f", Inverter_Output_Grid_Parameter.Total_Grd_External_Power);
    }
    if (Param_Address == ((Total_Load_Power[1] << 8) + Total_Load_Power[2]))
    {
        Inverter_Output_Grid_Parameter.Total_Load_Power = Data / 1000.0;
        printf("Total_Load_Power:%0.2f", Inverter_Output_Grid_Parameter.Total_Load_Power);
    }

    /* =====================================================================
     * Read-side config parameter handling
     * Only runs when PARAM_CONFIG_MODE==1 (mqtt.c asked for a specific
     * parameter value). The writable entries populate CONFIG_VAR_VALUE.
     * ===================================================================== */
    if (Param_Address == ((DC_Voltage_Upper_Limit[1] << 8) + DC_Voltage_Upper_Limit[2]))
    {
        Inverter_Config_Param.DC_Voltage_Upper_Limit = Data;
        CONFIG_VAR_VALUE = Inverter_Config_Param.DC_Voltage_Upper_Limit;
        printf("DC_Voltage_Upper_Limit:%0.2f", CONFIG_VAR_VALUE);
    }
    if (Param_Address == ((Grid_Voltage_Upper_Limit[1] << 8) + Grid_Voltage_Upper_Limit[2]))
    {
        Inverter_Config_Param.Grid_Voltage_Upper_Limit = Data;
        CONFIG_VAR_VALUE = Inverter_Config_Param.Grid_Voltage_Upper_Limit;
        printf("Grid_Voltage_Upper_Limit:%0.2f", CONFIG_VAR_VALUE);
    }
    if (Param_Address == ((Grid_Voltage_Upper_Limit_L2[1] << 8) + Grid_Voltage_Upper_Limit_L2[2]))
    {
        Inverter_Config_Param.Grid_Voltage_Upper_Limit_L2 = Data;
        CONFIG_VAR_VALUE = Inverter_Config_Param.Grid_Voltage_Upper_Limit_L2;
        printf("Grid_Voltage_Upper_Limit_L2:%0.2f", CONFIG_VAR_VALUE);
    }
    if (Param_Address == ((Grid_Voltage_Upper_Limit_L3[1] << 8) + Grid_Voltage_Upper_Limit_L3[2]))
    {
        Inverter_Config_Param.Grid_Voltage_Upper_Limit_L3 = Data;
        CONFIG_VAR_VALUE = Inverter_Config_Param.Grid_Voltage_Upper_Limit_L3;
        printf("Grid_Voltage_Upper_Limit_L3:%0.2f", CONFIG_VAR_VALUE);
    }
    if (Param_Address == ((Grid_Voltage_Lower_Limit[1] << 8) + Grid_Voltage_Lower_Limit[2]))
    {
        Inverter_Config_Param.Grid_Voltage_Lower_Limit = Data;
        CONFIG_VAR_VALUE = Inverter_Config_Param.Grid_Voltage_Lower_Limit;
        printf("Grid_Voltage_Lower_Limit:%0.2f", CONFIG_VAR_VALUE);
    }
    if (Param_Address == ((Grid_Voltage_Lower_Limit_L2[1] << 8) + Grid_Voltage_Lower_Limit_L2[2]))
    {
        Inverter_Config_Param.Grid_Voltage_Lower_Limit_L2 = Data;
        CONFIG_VAR_VALUE = Inverter_Config_Param.Grid_Voltage_Lower_Limit_L2;
        printf("Grid_Voltage_Lower_Limit_L2:%0.2f", CONFIG_VAR_VALUE);
    }
    if (Param_Address == ((Grid_Voltage_Lower_Limit_L3[1] << 8) + Grid_Voltage_Lower_Limit_L3[2]))
    {
        Inverter_Config_Param.Grid_Voltage_Lower_Limit_L3 = Data;
        CONFIG_VAR_VALUE = Inverter_Config_Param.Grid_Voltage_Lower_Limit_L3;
        printf("Grid_Voltage_Lower_Limit_L3:%0.2f", CONFIG_VAR_VALUE);
    }
    if (Param_Address == ((Grid_Current_Upper_Limit[1] << 8) + Grid_Current_Upper_Limit[2]))
    {
        Inverter_Config_Param.Grid_Current_Upper_Limit = Data;
        CONFIG_VAR_VALUE = Inverter_Config_Param.Grid_Current_Upper_Limit;
        printf("Grid_Current_Upper_Limit:%0.2f", CONFIG_VAR_VALUE);
    }
    if (Param_Address == ((Grid_Freq_Upper_Limit[1] << 8) + Grid_Freq_Upper_Limit[2]))
    {
        Inverter_Config_Param.Grid_Freq_Upper_Limit = Data;
        CONFIG_VAR_VALUE = Inverter_Config_Param.Grid_Freq_Upper_Limit;
        printf("Grid_Freq_Upper_Limit:%0.2f", CONFIG_VAR_VALUE);
    }
    if (Param_Address == ((Grid_Freq_Upper_Limit_L2[1] << 8) + Grid_Freq_Upper_Limit_L2[2]))
    {
        Inverter_Config_Param.Grid_Freq_Upper_Limit_L2 = Data;
        CONFIG_VAR_VALUE = Inverter_Config_Param.Grid_Freq_Upper_Limit_L2;
        printf("Grid_Freq_Upper_Limit_L2:%0.2f", CONFIG_VAR_VALUE);
    }
    if (Param_Address == ((Grid_Freq_Upper_Limit_L3[1] << 8) + Grid_Freq_Upper_Limit_L3[2]))
    {
        Inverter_Config_Param.Grid_Freq_Upper_Limit_L3 = Data;
        CONFIG_VAR_VALUE = Inverter_Config_Param.Grid_Freq_Upper_Limit_L3;
        printf("Grid_Freq_Upper_Limit_L3:%0.2f", CONFIG_VAR_VALUE);
    }
    if (Param_Address == ((Grid_Freq_Lower_Limit[1] << 8) + Grid_Freq_Lower_Limit[2]))
    {
        Inverter_Config_Param.Grid_Freq_Lower_Limit = Data;
        CONFIG_VAR_VALUE = Inverter_Config_Param.Grid_Freq_Lower_Limit;
        printf("Grid_Freq_Lower_Limit:%0.2f", CONFIG_VAR_VALUE);
    }
    if (Param_Address == ((Grid_Freq_Lower_Limit_L2[1] << 8) + Grid_Freq_Lower_Limit_L2[2]))
    {
        Inverter_Config_Param.Grid_Freq_Lower_Limit_L2 = Data;
        CONFIG_VAR_VALUE = Inverter_Config_Param.Grid_Freq_Lower_Limit_L2;
        printf("Grid_Freq_Lower_Limit_L2:%0.2f", CONFIG_VAR_VALUE);
    }
    if (Param_Address == ((Grid_Freq_Lower_Limit_L3[1] << 8) + Grid_Freq_Lower_Limit_L3[2]))
    {
        Inverter_Config_Param.Grid_Freq_Lower_Limit_L3 = Data;
        CONFIG_VAR_VALUE = Inverter_Config_Param.Grid_Freq_Lower_Limit_L3;
        printf("Grid_Freq_Lower_Limit_L3:%0.2f", CONFIG_VAR_VALUE);
    }
    if (Param_Address == ((Rated_Grid_Voltage_Set[1] << 8) + Rated_Grid_Voltage_Set[2]))
    {
        Inverter_Config_Param.Rated_Grid_Voltage_Set = Data;
        CONFIG_VAR_VALUE = Inverter_Config_Param.Rated_Grid_Voltage_Set;
        printf("Rated_Grid_Voltage_Set:%0.2f", CONFIG_VAR_VALUE);
    }
    if (Param_Address == ((Grid_Voltage_Upper_Limit_Trip_Time_1OD[1] << 8) + Grid_Voltage_Upper_Limit_Trip_Time_1OD[2]))
    {
        Inverter_Config_Param.Grid_Voltage_Upper_Limit_Trip_Time_1OD = Data;
        CONFIG_VAR_VALUE = Inverter_Config_Param.Grid_Voltage_Upper_Limit_Trip_Time_1OD;
        printf("Grid_Voltage_Upper_Limit_Trip_Time_1OD:%0.2f", CONFIG_VAR_VALUE);
    }
    if (Param_Address == ((Grid_Voltage_Upper_Limit_Trip_Time_2OD[1] << 8) + Grid_Voltage_Upper_Limit_Trip_Time_2OD[2]))
    {
        Inverter_Config_Param.Grid_Voltage_Upper_Limit_Trip_Time_2OD = Data;
        CONFIG_VAR_VALUE = Inverter_Config_Param.Grid_Voltage_Upper_Limit_Trip_Time_2OD;
        printf("Grid_Voltage_Upper_Limit_Trip_Time_2OD:%0.2f", CONFIG_VAR_VALUE);
    }
    if (Param_Address == ((Grid_Voltage_Upper_Limit_Trip_Time_3OD[1] << 8) + Grid_Voltage_Upper_Limit_Trip_Time_3OD[2]))
    {
        Inverter_Config_Param.Grid_Voltage_Upper_Limit_Trip_Time_3OD = Data;
        CONFIG_VAR_VALUE = Inverter_Config_Param.Grid_Voltage_Upper_Limit_Trip_Time_3OD;
        printf("Grid_Voltage_Upper_Limit_Trip_Time_3OD:%0.2f", CONFIG_VAR_VALUE);
    }
    if (Param_Address == ((Grid_Voltage_Lower_Limit_Trip_Time_1OD[1] << 8) + Grid_Voltage_Lower_Limit_Trip_Time_1OD[2]))
    {
        Inverter_Config_Param.Grid_Voltage_Lower_Limit_Trip_Time_1OD = Data;
        CONFIG_VAR_VALUE = Inverter_Config_Param.Grid_Voltage_Lower_Limit_Trip_Time_1OD;
        printf("Grid_Voltage_Lower_Limit_Trip_Time_1OD:%0.2f", CONFIG_VAR_VALUE);
    }
    if (Param_Address == ((Grid_Voltage_Lower_Limit_Trip_Time_2OD[1] << 8) + Grid_Voltage_Lower_Limit_Trip_Time_2OD[2]))
    {
        Inverter_Config_Param.Grid_Voltage_Lower_Limit_Trip_Time_2OD = Data;
        CONFIG_VAR_VALUE = Inverter_Config_Param.Grid_Voltage_Lower_Limit_Trip_Time_2OD;
        printf("Grid_Voltage_Lower_Limit_Trip_Time_2OD:%0.2f", CONFIG_VAR_VALUE);
    }
    if (Param_Address == ((Grid_Voltage_Lower_Limit_Trip_Time_3OD[1] << 8) + Grid_Voltage_Lower_Limit_Trip_Time_3OD[2]))
    {
        Inverter_Config_Param.Grid_Voltage_Lower_Limit_Trip_Time_3OD = Data;
        CONFIG_VAR_VALUE = Inverter_Config_Param.Grid_Voltage_Lower_Limit_Trip_Time_3OD;
        printf("Grid_Voltage_Lower_Limit_Trip_Time_3OD:%0.2f", CONFIG_VAR_VALUE);
    }
    if (Param_Address == ((Grid_Freq_Upper_Limit_Trip_Time_1OD[1] << 8) + Grid_Freq_Upper_Limit_Trip_Time_1OD[2]))
    {
        Inverter_Config_Param.Grid_Freq_Upper_Limit_Trip_Time_1OD = Data;
        CONFIG_VAR_VALUE = Inverter_Config_Param.Grid_Freq_Upper_Limit_Trip_Time_1OD;
        printf("Grid_Freq_Upper_Limit_Trip_Time_1OD:%0.2f", CONFIG_VAR_VALUE);
    }
    if (Param_Address == ((Grid_Freq_Upper_Limit_Trip_Time_2OD[1] << 8) + Grid_Freq_Upper_Limit_Trip_Time_2OD[2]))
    {
        Inverter_Config_Param.Grid_Freq_Upper_Limit_Trip_Time_2OD = Data;
        CONFIG_VAR_VALUE = Inverter_Config_Param.Grid_Freq_Upper_Limit_Trip_Time_2OD;
        printf("Grid_Freq_Upper_Limit_Trip_Time_2OD:%0.2f", CONFIG_VAR_VALUE);
    }
    if (Param_Address == ((Grid_Freq_Upper_Limit_Trip_Time_3OD[1] << 8) + Grid_Freq_Upper_Limit_Trip_Time_3OD[2]))
    {
        Inverter_Config_Param.Grid_Freq_Upper_Limit_Trip_Time_3OD = Data;
        CONFIG_VAR_VALUE = Inverter_Config_Param.Grid_Freq_Upper_Limit_Trip_Time_3OD;
        printf("Grid_Freq_Upper_Limit_Trip_Time_3OD:%0.2f", CONFIG_VAR_VALUE);
    }
    if (Param_Address == ((Grid_Freq_Lower_Limit_Trip_Time_1OD[1] << 8) + Grid_Freq_Lower_Limit_Trip_Time_1OD[2]))
    {
        Inverter_Config_Param.Grid_Freq_Lower_Limit_Trip_Time_1OD = Data;
        CONFIG_VAR_VALUE = Inverter_Config_Param.Grid_Freq_Lower_Limit_Trip_Time_1OD;
        printf("Grid_Freq_Lower_Limit_Trip_Time_1OD:%0.2f", CONFIG_VAR_VALUE);
    }
    if (Param_Address == ((Grid_Freq_Lower_Limit_Trip_Time_2OD[1] << 8) + Grid_Freq_Lower_Limit_Trip_Time_2OD[2]))
    {
        Inverter_Config_Param.Grid_Freq_Lower_Limit_Trip_Time_2OD = Data;
        CONFIG_VAR_VALUE = Inverter_Config_Param.Grid_Freq_Lower_Limit_Trip_Time_2OD;
        printf("Grid_Freq_Lower_Limit_Trip_Time_2OD:%0.2f", CONFIG_VAR_VALUE);
    }
    if (Param_Address == ((Grid_Freq_Lower_Limit_Trip_Time_3OD[1] << 8) + Grid_Freq_Lower_Limit_Trip_Time_3OD[2]))
    {
        Inverter_Config_Param.Grid_Freq_Lower_Limit_Trip_Time_3OD = Data;
        CONFIG_VAR_VALUE = Inverter_Config_Param.Grid_Freq_Lower_Limit_Trip_Time_3OD;
        printf("Grid_Freq_Lower_Limit_Trip_Time_3OD:%0.2f", CONFIG_VAR_VALUE);
    }
    if (Param_Address == ((Remote_Lock[1] << 8) + Remote_Lock[2]))
    {
        Inverter_Config_Param.Remote_Lock = Data;
        CONFIG_VAR_VALUE = Inverter_Config_Param.Remote_Lock;
        printf("Remote_Lock:%0.2f", CONFIG_VAR_VALUE);
    }
    if (Param_Address == ((Self_Test_Time[1] << 8) + Self_Test_Time[2]))
    {
        Inverter_Config_Param.Self_Test_Time = Data;
        CONFIG_VAR_VALUE = Inverter_Config_Param.Self_Test_Time;
        printf("Self_Test_Time:%0.2f", CONFIG_VAR_VALUE);
    }
    if (Param_Address == ((Starting_Voltage_Upper_Limit[1] << 8) + Starting_Voltage_Upper_Limit[2]))
    {
        Inverter_Config_Param.Starting_Voltage_Upper_Limit = Data;
        CONFIG_VAR_VALUE = Inverter_Config_Param.Starting_Voltage_Upper_Limit;
        printf("Starting_Voltage_Upper_Limit:%0.2f", CONFIG_VAR_VALUE);
    }
    if (Param_Address == ((Starting_Voltage_Lower_Limit[1] << 8) + Starting_Voltage_Lower_Limit[2]))
    {
        Inverter_Config_Param.Starting_Voltage_Lower_Limit = Data;
        CONFIG_VAR_VALUE = Inverter_Config_Param.Starting_Voltage_Lower_Limit;
        printf("Starting_Voltage_Lower_Limit:%0.2f", CONFIG_VAR_VALUE);
    }
    if (Param_Address == ((Over_Frequency_Drate_Point[1] << 8) + Over_Frequency_Drate_Point[2]))
    {
        Inverter_Config_Param.Over_Frequency_Drate_Point = Data;
        CONFIG_VAR_VALUE = Inverter_Config_Param.Over_Frequency_Drate_Point;
        printf("Over_Frequency_Drate_Point:%0.2f", CONFIG_VAR_VALUE);
    }
    if (Param_Address == ((Over_Frequency_Drate[1] << 8) + Over_Frequency_Drate[2]))
    {
        Inverter_Config_Param.Over_Frequency_Drate = Data;
        CONFIG_VAR_VALUE = Inverter_Config_Param.Over_Frequency_Drate;
        printf("Over_Frequency_Drate:%0.2f", CONFIG_VAR_VALUE);
    }
    if (Param_Address == ((Internal_Temp_Upper_Limit[1] << 8) + Internal_Temp_Upper_Limit[2]))
    {
        Inverter_Config_Param.Internal_Temp_Upper_Limit = Data;
        CONFIG_VAR_VALUE = Inverter_Config_Param.Internal_Temp_Upper_Limit;
        printf("Internal_Temp_Upper_Limit:%0.2f", CONFIG_VAR_VALUE);
    }
    if (Param_Address == ((Power_Factor_Regulation[1] << 8) + Power_Factor_Regulation[2]))
    {
        Inverter_Config_Param.Power_Factor_Regulation = Data;
        CONFIG_VAR_VALUE = Inverter_Config_Param.Power_Factor_Regulation;
        printf("Power_Factor_Regulation:%0.2f", CONFIG_VAR_VALUE);
    }
    if (Param_Address == ((Active_Power_Regulation[1] << 8) + Active_Power_Regulation[2]))
    {
        Inverter_Config_Param.Active_Power_Regulation = Data;
        CONFIG_VAR_VALUE = Inverter_Config_Param.Active_Power_Regulation;
        printf("Active_Power_Regulation:%0.2f", CONFIG_VAR_VALUE);
    }
    if (Param_Address == ((Reactive_Power_Regulation[1] << 8) + Reactive_Power_Regulation[2]))
    {
        Inverter_Config_Param.Reactive_Power_Regulation = Data;
        CONFIG_VAR_VALUE = Inverter_Config_Param.Reactive_Power_Regulation;
        printf("Reactive_Power_Regulation:%0.2f", CONFIG_VAR_VALUE);
    }
    if (Param_Address == ((Apparent_Power_Regulation[1] << 8) + Apparent_Power_Regulation[2]))
    {
        Inverter_Config_Param.Apparent_Power_Regulation = Data;
        CONFIG_VAR_VALUE = Inverter_Config_Param.Apparent_Power_Regulation;
        printf("Apparent_Power_Regulation:%0.2f", CONFIG_VAR_VALUE);
    }
    if (Param_Address == ((Switch_ON_OFF_Enable[1] << 8) + Switch_ON_OFF_Enable[2]))
    {
        Inverter_Config_Param.Switch_ON_OFF_Enable = Data;
        CONFIG_VAR_VALUE = Inverter_Config_Param.Switch_ON_OFF_Enable;
        printf("Switch_ON_OFF_Enable:%0.2f", CONFIG_VAR_VALUE);
    }
    if (Param_Address == ((Factory_Reset_Enable[1] << 8) + Factory_Reset_Enable[2]))
    {
        Inverter_Config_Param.Factory_Reset_Enable = Data;
        CONFIG_VAR_VALUE = Inverter_Config_Param.Factory_Reset_Enable;
        printf("Factory_Reset_Enable:%0.2f", CONFIG_VAR_VALUE);
    }
    if (Param_Address == ((Self_Checking_Time[1] << 8) + Self_Checking_Time[2]))
    {
        Inverter_Config_Param.Self_Checking_Time = Data;
        CONFIG_VAR_VALUE = Inverter_Config_Param.Self_Checking_Time;
        printf("Self_Checking_Time:%0.2f", CONFIG_VAR_VALUE);
    }
    if (Param_Address == ((Island_Protection_Enable[1] << 8) + Island_Protection_Enable[2]))
    {
        Inverter_Config_Param.Island_Protection_Enable = Data;
        CONFIG_VAR_VALUE = Inverter_Config_Param.Island_Protection_Enable;
        printf("Island_Protection_Enable:%0.2f", CONFIG_VAR_VALUE);
    }
    if (Param_Address == ((Meter_Enable[1] << 8) + Meter_Enable[2]))
    {
        Inverter_Config_Param.Meter_Enable = Data;
        CONFIG_VAR_VALUE = Inverter_Config_Param.Meter_Enable;
        printf("Meter_Enable:%0.2f", CONFIG_VAR_VALUE);
    }
    if (Param_Address == ((Meter_Enable_Read[1] << 8) + Meter_Enable_Read[2]))
    {
        Inverter_Config_Param.Meter_Enable = Data;
        printf("Meter_Enable_Read:%0.2f", Inverter_Config_Param.Meter_Enable);
    }
    if (Param_Address == ((RCD_Enable[1] << 8) + RCD_Enable[2]))
    {
        Inverter_Config_Param.RCD_Enable = Data;
        CONFIG_VAR_VALUE = Inverter_Config_Param.RCD_Enable;
        printf("RCD_Enable:%0.2f", CONFIG_VAR_VALUE);
    }
    if (Param_Address == ((RISO_Enable[1] << 8) + RISO_Enable[2]))
    {
        Inverter_Config_Param.RISO_Enable = Data;
        CONFIG_VAR_VALUE = Inverter_Config_Param.RISO_Enable;
        printf("RISO_Enable:%0.2f", CONFIG_VAR_VALUE);
    }
    if (Param_Address == ((Grid_Standard[1] << 8) + Grid_Standard[2]))
    {
        Inverter_Config_Param.Grid_Standard = Data;
        CONFIG_VAR_VALUE = Inverter_Config_Param.Grid_Standard;
        printf("Grid_Standard:%0.2f", CONFIG_VAR_VALUE);
    }
    if (Param_Address == ((PV_Curved_Enable[1] << 8) + PV_Curved_Enable[2]))
    {
        Inverter_Config_Param.PV_Curved_Enable = Data;
        CONFIG_VAR_VALUE = Inverter_Config_Param.PV_Curved_Enable;
        printf("PV_Curved_Enable:%0.2f", CONFIG_VAR_VALUE);
    }
    if (Param_Address == ((Eeprome_Initial_Enabled[1] << 8) + Eeprome_Initial_Enabled[2]))
    {
        Inverter_Config_Param.Eeprome_Initial_Enabled = Data;
        CONFIG_VAR_VALUE = Inverter_Config_Param.Eeprome_Initial_Enabled;
        printf("Eeprome_Initial_Enabled:%0.2f", CONFIG_VAR_VALUE);
    }
    if (Param_Address == ((Limiter_Function_Enabled[1] << 8) + Limiter_Function_Enabled[2]))
    {
        Inverter_Config_Param.Limiter_Function_Enabled = Data;
        CONFIG_VAR_VALUE = Inverter_Config_Param.Limiter_Function_Enabled;
        printf("Limiter_Function_Enabled:%0.2f", CONFIG_VAR_VALUE);
    }
    if (Param_Address == ((Power_Wh_Factor[1] << 8) + Power_Wh_Factor[2]))
    {
        Inverter_Config_Param.Power_Wh_Factor = Data;
        CONFIG_VAR_VALUE = Inverter_Config_Param.Power_Wh_Factor;
        printf("Power_Wh_Factor:%0.2f", CONFIG_VAR_VALUE);
    }
    if (Param_Address == ((External_CT_Flag[1] << 8) + External_CT_Flag[2]))
    {
        Inverter_Config_Param.External_CT_Flag = Data;
        CONFIG_VAR_VALUE = Inverter_Config_Param.External_CT_Flag;
        printf("External_CT_Flag:%0.2f", CONFIG_VAR_VALUE);
    }
}

/* =========================================================================
 * getVariableFromEnum — map server-requested key to the Modbus command
 * template that carries its register address.
 * ========================================================================= */
unsigned char *getVariableFromEnum(int configVar)
{
    switch (configVar)
    {
        case RATED_GRID_VOLTAGE_SET:                return Rated_Grid_Voltage_Set;
        case REMOTE_LOCK:                           return Remote_Lock;
        case SELF_TEST_TIME:                        return Self_Test_Time;
        case DC_VOLTAGE_UPPER_LIMIT:                return DC_Voltage_Upper_Limit;
        case GRID_VOLTAGE_LOWER_LIMIT:              return Grid_Voltage_Lower_Limit;
        case GRID_VOLTAGE_UPPER_LIMIT:              return Grid_Voltage_Upper_Limit;
        case GRID_FREQ_LOWER_LIMIT:                 return Grid_Freq_Lower_Limit;
        case GRID_FREQ_UPPER_LIMIT:                 return Grid_Freq_Upper_Limit;
        case GRID_CURRENT_UPPER_LIMIT:              return Grid_Current_Upper_Limit;
        case STARTING_VOLTAGE_UPPER_LIMIT:          return Starting_Voltage_Upper_Limit;
        case STARTING_VOLTAGE_LOWET_LIMIT:          return Starting_Voltage_Lower_Limit;
        case OVER_FREQUENCY_DRATE_POINT:            return Over_Frequency_Drate_Point;
        case OVER_FREQUENCY_DRATE:                  return Over_Frequency_Drate;
        case INTERNAL_TEMPERATURE_UPPER_LIMIT:      return Internal_Temp_Upper_Limit;
        case POWER_FACTOR_REGULATION:               return Power_Factor_Regulation;
        case ACTIVE_POWER_REGULATION:               return Active_Power_Regulation;
        case REACTIVE_POWER_REGULATION:             return Reactive_Power_Regulation;
        case APPARENT_POWER_REGULATION:             return Apparent_Power_Regulation;
        case SWITCH_ON_OFF_ENABLE:                  return Switch_ON_OFF_Enable;
        case FACTORY_RESET_ENABLE:                  return Factory_Reset_Enable;
        case SELF_CHECKING_TIME:                    return Self_Checking_Time;
        case ISLAND_PROTECTION_ENABLE:              return Island_Protection_Enable;
        case METER_ENABLED:                         return Meter_Enable;
        case RCD_ENABLED:                           return RCD_Enable;
        case RISO_ENABLED:                          return RISO_Enable;
        case GRID_STANDARD:                         return Grid_Standard;
        case PV_CURVE_ENABLED:                      return PV_Curved_Enable;
        case EEPROME_INITAL_ENABLED:                return Eeprome_Initial_Enabled;
        case LIMITER_FUNCTION_ENABLE:               return Limiter_Function_Enabled;
        case POWER_WH_FACTOR:                       return Power_Wh_Factor;
        case GRID_VOLTAGE_LOWER_LIMIT_L2:           return Grid_Voltage_Lower_Limit_L2;
        case GRID_VOLTAGE_UPPER_LIMIT_L2:           return Grid_Voltage_Upper_Limit_L2;
        case GRID_FREQ_LOWER_LIMIT_L2:              return Grid_Freq_Lower_Limit_L2;
        case GRID_FREQ_UPPER_LIMIT_L2:              return Grid_Freq_Upper_Limit_L2;
        case GRID_VOLTAGE_LOWER_LIMIT_L3:           return Grid_Voltage_Lower_Limit_L3;
        case GRID_VOLTAGE_UPPER_LIMIT_L3:           return Grid_Voltage_Upper_Limit_L3;
        case GRID_FREQ_LOWER_LIMIT_L3:              return Grid_Freq_Lower_Limit_L3;
        case GRID_FREQ_UPPER_LIMIT_L3:              return Grid_Freq_Upper_Limit_L3;
        case GRID_VOLTAGE_UPPER_LIMIT_TRIP_TIME_1OD: return Grid_Voltage_Upper_Limit_Trip_Time_1OD;
        case GRID_VOLTAGE_UPPER_LIMIT_TRIP_TIME_2OD: return Grid_Voltage_Upper_Limit_Trip_Time_2OD;
        case GRID_VOLTAGE_UPPER_LIMIT_TRIP_TIME_3OD: return Grid_Voltage_Upper_Limit_Trip_Time_3OD;
        case GRID_VOLTAGE_LOWER_LIMIT_TRIP_TIME_1OD: return Grid_Voltage_Lower_Limit_Trip_Time_1OD;
        case GRID_VOLTAGE_LOWER_LIMIT_TRIP_TIME_2OD: return Grid_Voltage_Lower_Limit_Trip_Time_2OD;
        case GRID_VOLTAGE_LOWER_LIMIT_TRIP_TIME_3OD: return Grid_Voltage_Lower_Limit_Trip_Time_3OD;
        case GRID_FREQ_UPPER_LIMIT_TRIP_TIME_1OD:    return Grid_Freq_Upper_Limit_Trip_Time_1OD;
        case GRID_FREQ_UPPER_LIMIT_TRIP_TIME_2OD:    return Grid_Freq_Upper_Limit_Trip_Time_2OD;
        case GRID_FREQ_UPPER_LIMIT_TRIP_TIME_3OD:    return Grid_Freq_Upper_Limit_Trip_Time_3OD;
        case GRID_FREQ_LOWER_LIMIT_TRIP_TIME_1OD:    return Grid_Freq_Lower_Limit_Trip_Time_1OD;
        case GRID_FREQ_LOWER_LIMIT_TRIP_TIME_2OD:    return Grid_Freq_Lower_Limit_Trip_Time_2OD;
        case GRID_FREQ_LOWER_LIMIT_TRIP_TIME_3OD:    return Grid_Freq_Lower_Limit_Trip_Time_3OD;
        case EXTERNAL_CT_FLAG:                       return External_CT_Flag;
        case WR_DATE_TIME:                           return Write_Date_Time;
        default:                                     return NULL;
    }
}

/* =========================================================================
 * Payload helpers used by the config read/write path.
 * ========================================================================= */
void sanitize_string(char *str)
{
    printf("Sanitizing string...");
    while (*str)
    {
        if (*str == '\r' || *str == '\n')
        {
            *str = '\0';
            break;
        }
        str++;
    }
}

void extract_serial_number(uint8_t *Resp, int length)
{
    if (length < 10)
    {
        printf("Error: Response too short!");
        return;
    }

    memcpy(serial_number, &Resp[3], SERIAL_NUMBER_LENGTH_INV);
    serial_number[SERIAL_NUMBER_LENGTH_INV] = '\0';
    printf("Inverter Serial Number: %s", serial_number);
    memset(Inverter_Comm_Buffer, '\0', INVERTER_COMM_BUFFER_SIZE);
}

void Assign_DateTime_to_Param(char *Val, unsigned char *Param_Name)
{
    int year, month, day, hour, minute, second;

    printf("RECEIVED STRING: %s", Val);
    sanitize_string(Val);

    if (strlen(Val) != 12)
    {
        printf("Invalid DateTime Format");
        return;
    }

    sscanf(Val, "%2d%2d%2d%2d%2d%2d", &year, &month, &day, &hour, &minute, &second);

    Param_Name[0]  = 0x10;
    Param_Name[6]  = year;
    Param_Name[7]  = month;
    Param_Name[8]  = day;
    Param_Name[9]  = hour;
    Param_Name[10] = minute;
    Param_Name[11] = second;

    printf("YEAR[6]:%d MONTH[7]:%d DAY[8]:%d HOUR[9]:%d MIN[10]:%d SEC[11]:%d",
                    Param_Name[6], Param_Name[7], Param_Name[8],
                    Param_Name[9], Param_Name[10], Param_Name[11]);
}

void Assign_Value_to_Param(char *Val, unsigned char *Param_Name)
{
    int HIGHER_DEC = 0;

    printf("RECEIVED STRING:%s", Val);
    HIGHER_DEC = atoi(Val);
    printf("RECEIVED DECIMAL:%d", HIGHER_DEC);

    Param_Name[0] = 0x10;
    Param_Name[6] = (HIGHER_DEC & 0xFF00) >> 8;
    Param_Name[7] = HIGHER_DEC & 0x00FF;
}

void Write_Config_Param_R(unsigned char *Inverter_Cmd)
{
    if (Inverter_Cmd == NULL) return;

    if (Inverter_Cmd[0] != 0x03)
    {
        Inverter_Cmd[0] = 0x03;
    }

    Inverter_Cmd_t temp_cmd;
    temp_cmd.ParamCmd = Inverter_Cmd;
    push_to_ParamCmd_buffer(&temp_cmd);
}

void Write_Config_Param_W(unsigned char *Inverter_Cmd, char *Value)
{
    if (Inverter_Cmd == NULL) return;

    Inverter_Cmd_t temp_cmd;
    if (WRITE_DATE_TIME == 1)
    {
        Assign_DateTime_to_Param(Value, Inverter_Cmd);
    }
    else
    {
        Assign_Value_to_Param(Value, Inverter_Cmd);
    }
    temp_cmd.ParamCmd = Inverter_Cmd;
    push_to_ParamCmd_buffer(&temp_cmd);
}

/* =========================================================================
 * LED / disconnect watchdog.
 * ========================================================================= */
void Controlle_Led(void)
{
    if (Inverter_Disconnect_Count++ > 1500)
    {
        INVERTER_COMMUNICATION = 0;
        Inverter_Disconnect_Count = 0;
    }

    led_config_set_inverter_status(INVERTER_COMMUNICATION,
                                   (Final_Fault != 0),
                                   (InverterData.RunState == INVERTER_RUN_STATE_NORMAL));
}


/* =========================================================================
 * UART_Init — EG800 signature:
 *   ql_uart_open(port, baud, flow)   (baud is an argument, not from dcb)
 *   ql_uart_register_cb(port, cb)    (cb = void(port, void*))
 *   ql_uart_get_dcbconfig            (diagnostic readback)
 *   ql_uart_write                    (initial banner)
 * ========================================================================= */
void UART_Init(void)
{
    int ret = -1;
    ql_uart_config_t dcb = {0};

    ret = ql_uart_open(INVERTER_UART_PORT, INVERTER_UART_BAUD, QL_FC_NONE);
    printf("uart[%d] open ret=%d", INVERTER_UART_PORT, ret);

    if (ret != QL_UART_SUCCESS && ret != QL_UART_ALREAD_OPEN)
    {
        printf("uart open FAILED, ret=%d", ret);
        return;
    }

    ql_uart_register_cb(INVERTER_UART_PORT, ql_uart_notify_cb);

    if (ql_uart_get_dcbconfig(INVERTER_UART_PORT, &dcb) == 0)
    {
        printf("uart[%d] baud=%d data=%d stop=%d parity=%d flow=%d",
                        INVERTER_UART_PORT,
                        dcb.baudrate, dcb.data_bit, dcb.stop_bit,
                        dcb.parity_bit, dcb.flow_ctrl);
    }

    printf("UART INITIALIZE (inverter, Modbus RTU 9600-8-N-1)");

    ql_uart_write(INVERTER_UART_PORT, data, sizeof(data));
    ql_uart_write(INVERTER_UART_PORT, data, sizeof(data));
    ql_uart_write(INVERTER_UART_PORT, data, sizeof(data));


    printf("UART INITIALIZE (inverter, Modbus RTU 9600-8-N-1 data = %s)",data);


}

/* =========================================================================
 * inverter_app_init — entry point called by main.c inverter_launcher.
 * ========================================================================= */
int inverter_app_init(void)
{
    int err=0;
    static ql_task_t inverter_task_t = NULL;
    err = ql_rtos_task_create(&inverter_task_t, 16 * 1024,100,"Inverter_app_comm",inverter_app_thread,NULL, 2);
    if (err != 0)
    {
        printf("inverter_task init failed 0x%x", err);
    }
    return err;
}