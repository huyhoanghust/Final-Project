/**
 * @file gnss.c
 * @author hoanghuyhust (hoangnh191855@sis.hust.edu.vn)
 * @brief
 * @version 0.1
 * @date 2023-11-13
 *
 * @copyright Copyright (c) 2023
 *
 */
/*$GPRMC,170047.00,A,4121.23302,N,00201.31654,W,2.703,   ,200417,   , ,A*64
$GPRMC,hhmmss.ss,A,llll.ll,   a,yyyyy.yy,   a,x.x,  x.x,ddmmyy,x.x,a,m*hh
       |         | |          | |           | |     |   |      |   | | \- 13 Checksum
       |         | |          | |           | |     |   |      |   | \--- 12 Mode indicator, (A=Autonomous, D=Differential, E=Estimated, N=Data not valid)
       |         | |          | |           | |     |   |      |   \----- 11 E or W of magnetic variation
       |         | |          | |           | |     |   |      \--------- 10 Magnetic variation degrees (Easterly var. subtracts from true course)
       |         | |          | |           | |     |   \----------------  9 UTC date of fix
       |         | |          | |           | |     \--------------------  8 Track made good in degrees True
       |         | |          | |           | \--------------------------  7 Speed over ground in knots
       |         | |          | |           \----------------------------  6 E or W of longitude
       |         | |          | \----------------------------------------  5 Longitude of fix
       |         | |          \------------------------------------------  4 N or S of longitude
       |         | \-----------------------------------------------------  3 Latitude of fix
       |         \-------------------------------------------------------  2 Data status (A=Valid position, V=navigation receiver warning)
       \-----------------------------------------------------------------  1 UTC time of fix
*/
#include "gnss.h"

l70GPGGAType_t l70GPGGA = {0};
int k = 0;
char l70RxData[100] = {0};
char l70Data = 0;
l70DataRecive_t l70RxState;
l70GPRMCType_t l70GPRMC;
l70Data_t l70DataAfterHadler;
l70Utctime_t l70Utctime;
l70Date_t l70Date;

double speed_knot;
double speed_km;

// Reverses a string 'str' of length 'len'
void reverse(char *str, int len)
{
    int i = 0, j = len - 1, temp;
    while (i < j)
    {
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
        i++;
        j--;
    }
}

// Converts a given integer x to string str[].
// d is the number of digits required in the output.
// If d is more than the number of digits in x,
// then 0s are added at the beginning.
int intToStr(int x, char str[], int d)
{
    int i = 0;
    if (x == 0)
    {
        str[i++] = '0';
    }
    else
    {
        while (x)
        {
            str[i++] = (x % 10) + '0';
            x = x / 10;
        }
    }

    // If number of digits required is more, then
    // add 0s at the beginning
    while (i < d)
        str[i++] = '0';

    reverse(str, i);
    str[i] = '\0';
    return i;
}

// Converts a floating-point/double number to a string.
void ftoa(float n, char *res, int afterpoint)
{
    // Extract integer part
    int ipart = (int)n;
    // Extract floating part
    float fpart = n - (float)ipart;
    // convert integer part to string
    int i = intToStr(ipart, res, 0);
    // check for display option after point
    if (afterpoint != 0)
    {
        res[i] = '.'; // add dot
        fpart = fpart * pow(10, afterpoint);
        intToStr((int)fpart, res + i + 1, afterpoint);
    }
}

/*
init module with  PMTK_API_SET_NMEA_OUTPUT
*/
void l70_init(void)
{
    // Output once every one position fix
    // GGA interval fix data
    HAL_GPIO_WritePin(GNSS_RESET_GPIO_Port, GNSS_CTRL_PWR_Pin, GPIO_PIN_RESET);
    HAL_Delay(100);
    HAL_GPIO_WritePin(GNSS_RESET_GPIO_Port, GNSS_CTRL_PWR_Pin, GPIO_PIN_SET);
    HAL_Delay(100);
    HAL_GPIO_WritePin(GNSS_RESET_GPIO_Port, GNSS_CTRL_PWR_Pin, GPIO_PIN_RESET);
    HAL_Delay(100);
    HAL_UART_Transmit(&huart4, (uint8_t *)"$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29\r\n",
                      strlen("$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29\r\n"), 1000);
}

void l70_callback(void)
{
    // Start receive Data
    if (l70Data == '$' && l70RxState == L70_START_DATA)
    {
        l70RxData[k++] = l70Data;
        l70RxState = L70_DATA;
        HAL_UART_Receive_IT(&huart4, (uint8_t *)&l70Data, 1);
    }
    // End process
    else if (l70Data == '\n' && l70RxState == L70_DATA)
    {
        l70RxData[k++] = l70Data;
        l70RxState = L70_END_DATA;
    }
    // Receving Data
    else if (l70RxState == L70_DATA)
    {
        l70RxData[k++] = l70Data;
        HAL_UART_Receive_IT(&huart4, (uint8_t *)&l70Data, 1);
    }
    else
    {
        HAL_UART_Receive_IT(&huart4, (uint8_t *)&l70Data, 1);
    }
}

char *l70_receiveGPS(void)
{
    k = 0;
    memset(l70RxData, '\0', sizeof(l70RxData));
    l70RxState = L70_START_DATA;
    HAL_UART_Receive_IT(&huart4, (uint8_t *)&l70Data, 1);
    while (!(l70RxState == L70_END_DATA))
        ;
    return l70RxData;
}

double minuteTodegree(char *data)
{
    int temp = 0;
    double minutePart = 0;
    double degreePart = 0;
    double data_float = 0;
    double data_degree;
    // convert str to double
    data_float = atof(data);
    // convert ddmm.mmmm to dddd.dddd (d: degree; m: minute)
    temp = (int)data_float;
    degreePart = (double)(temp / 100);
    minutePart = (double)(temp % 100) + (data_float - (double)temp);
    data_degree = degreePart + minutePart / 60.0;
    return data_degree;
}

void l70_handleGPS(char *GPSResponse)
{
    // GPSResponse = strstr(GPSResponse, "$GPGGA");
    // sscanf(GPSResponse, "$GPGGA,%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,]",
    //  l70GPGGA.utcTime, l70GPGGA.latitude, &l70GPGGA.NorS, l70GPGGA.longitude,&l70GPGGA.EorW,
    //  &l70GPGGA.fixStatus, l70GPGGA.numberOfSatellites,l70GPGGA.HDOP,l70GPGGA.altitude);
    long int utctime;
    long int date;
    log_data("\nMessage GPRMC:\n");
    log_data(GPSResponse);
    GPSResponse = strstr(GPSResponse, "$GPRMC");
    // log_data("\nMessage GPRMC:\n");
    sscanf(GPSResponse, "$GPRMC,%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,]",\
           l70GPRMC.utcTime, &l70GPRMC.dataValid, l70GPRMC.latitude, &l70GPRMC.NorS, l70GPRMC.longitude,\
           &l70GPRMC.EorW, l70GPRMC.Speed, l70GPRMC.COGdirect, l70GPRMC.Date);

    // halder lat and long
    ftoa(minuteTodegree(l70GPRMC.latitude), l70DataAfterHadler.latitude, 6);
    ftoa(minuteTodegree(l70GPRMC.longitude), l70DataAfterHadler.longitude, 6);

    /// halder speed
    speed_knot = atof(l70GPRMC.Speed);
    speed_km = KNOT_TO_KM * speed_knot;
    ftoa(speed_km, l70DataAfterHadler.Speed, 1);

    // halder date and time  time: "hhmmss.00"   datte: "dmy"
    utctime = (long int)(atof(l70GPRMC.utcTime));
    l70Utctime.sencond = utctime % 100;
    l70Utctime.minute = (utctime % 10000) / 100;
    l70Utctime.hour = GMT + utctime / 10000;
    intToStr(l70Utctime.hour, l70DataAfterHadler.utcTime, 0);
    l70DataAfterHadler.utcTime[2] = ':';
    intToStr(l70Utctime.minute, l70DataAfterHadler.utcTime + 3, 0);
    l70DataAfterHadler.utcTime[5] = ':';
    intToStr(l70Utctime.sencond, l70DataAfterHadler.utcTime + 6, 0);

    date = (long int)(atof(l70GPRMC.Date));
    l70Date.year = 2000 + date % 100;
    l70Date.month = (date % 10000) / 100;
    l70Date.day = date / 10000;
    intToStr(l70Date.day, l70DataAfterHadler.Date, 0);
    l70DataAfterHadler.Date[2] = '/';
    intToStr(l70Date.month, l70DataAfterHadler.Date + 3, 0);
    l70DataAfterHadler.Date[5] = '/';
    intToStr(l70Date.year, l70DataAfterHadler.Date + 6, 0);

    log_data(l70DataAfterHadler.latitude);
    log_data("\n");
    log_data(l70DataAfterHadler.longitude);
    log_data("\n");
    log_data(l70DataAfterHadler.utcTime);
    log_data("\n");
    log_data(l70DataAfterHadler.Date);
    log_data("\n");
    log_data(l70DataAfterHadler.Speed);
    log_data("'\n");
}

void l70_checkSpeed()
{
    if (speed_km > 40.0)
    {
        log_data("Warning over speed!\n");
        //gui thong tin qua toc do len server va luu vao flash va sdcard
    }
}

void l70_sleep(void)
{
    HAL_UART_Transmit_IT(&huart4, (uint8_t *)"$PMTK161,0*28\r\n", strlen("$PMTK161,0*28\r\n"));
}