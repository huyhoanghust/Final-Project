#include "gnss.h"

l70GPGGAType_t l70GPGGA = {0};
int k = 0;
char l70RxData[100] = {0};
char l70Data = 0;
l70Data_t l70RxState;


// Reverses a string 'str' of length 'len'
void reverse(char* str, int len)
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
void ftoa(float n, char* res, int afterpoint)
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

void l70_callback(void)
{
    //Start receive Data
    if(l70Data == '$' && l70RxState == L70_START_DATA)
    {
        l70RxData[k++] = l70Data;
        l70RxState = L70_DATA;
        HAL_UART_Receive_IT(&huart4, (uint8_t*)&l70Data, 1);
    }
    //End process
    else if(l70Data == '\n' && l70RxState == L70_DATA)
    {
        l70RxData[k++] = l70Data;
        l70RxState = L70_END_DATA;
    }
    //Receving Data
    else if(l70RxState == L70_DATA)
    {
        l70RxData[k++] = l70Data;
        HAL_UART_Receive_IT(&huart4, (uint8_t*)&l70Data, 1);
    }
    else 
    {
        HAL_UART_Receive_IT(&huart4, (uint8_t*)&l70Data, 1);
    }
}

char *l70_receiveGPS(void)
{
    k = 0;
    memset(l70RxData, '\0', sizeof(l70RxData));
    l70RxState = L70_START_DATA;
    HAL_UART_Receive_IT(&huart4, (uint8_t*)&l70Data, 1);
    while(!(l70RxState == L70_END_DATA));
    return l70RxData;
}

void l70_init(void)
{
    //Output once every one position fix
    //GGA interval fix data
    HAL_UART_Transmit_IT(&huart4 ,(uint8_t*)"$PMTK314,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29\r\n", strlen("$PMTK314,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29\r\n"));
}

void l70_handleGPS(char *latData, char *longData, char *GPSResponse)
{
    int temp = 0;
    double decPart = 0;
    double intPart = 0;
    double l70latitude = 0;
    double l70longitude = 0;
    GPSResponse = strstr(GPSResponse, "$GPGGA");
    sscanf(GPSResponse, "$GPGGA,%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,]",\
     l70GPGGA.utcTime, l70GPGGA.latitude, &l70GPGGA.NorS, l70GPGGA.longitude,\
     &l70GPGGA.EorW, &l70GPGGA.fixStatus, l70GPGGA.numberOfSatellites);
    //convert str to double
    l70latitude = atof(l70GPGGA.latitude);
   //convert ddmm.mmmm to dddd.dddd (d: degree; m: minute)
    temp = (int)l70latitude;
    intPart = (double)(temp / 100);
    decPart = (double)(temp % 100) + l70latitude - (double)temp;
    l70latitude = intPart + decPart / 60;
    l70longitude = atof(l70GPGGA.longitude);
    temp = (int)l70longitude;
    intPart = (double)(temp / 100);
    decPart = (double)(temp % 100) + l70longitude - (double)temp;
    l70longitude = intPart + decPart / 60;
    ftoa(l70latitude, latData, 6);
    ftoa(l70longitude, longData, 6);
}

void l70_sleep(void)
{
    HAL_UART_Transmit_IT(&huart4 ,(uint8_t*)"$PMTK161,0*28\r\n", strlen("$PMTK161,0*28\r\n"));
}