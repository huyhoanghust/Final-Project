/**
 * @file mqtt.c
 * @author hoanghuyhust (hoangnh191855@sis.hust.edu.vn)
 * @brief 
 * @version 0.1
 * @date 2023-12-01
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "mqtt.h"

mqttServer_t mqttServer ={
    .host = "demo.thingsboard.io",
    .port = 1883,
    .serverType=0,  //TCP
};

mqttClient_t mqttClient={
    .clientID="DATN_Tracking_20191855",
    .keepAliveInterval = 90,
    .username="",
    .pass="",
    .clientIndex=0,
    .QoS=0,
};

mqttData_t mqttData;

char ATcommand[200]={0};

extern char simRxResponse[100];

void mqtt_disconnectServer()
{
    memset(ATcommand,'\0',sizeof(ATcommand));
    sprintf(ATcommand,"AT+CMQTTDISC=%d,%d\r\n",mqttClient.clientIndex, mqttClient.keepAliveInterval);
    if(sim7672_send_command(ATcommand,"+CMQTTDISC: 0,0\r\n",TIME_VERY_SHORT))
    {
        sim7672_errorHandle();
    }
    HAL_Delay(100);
    memset(ATcommand,'\0',sizeof(ATcommand));
    sprintf(ATcommand,"AT+CMQTTREL=%d\r\n",mqttClient.clientIndex);
    if(sim7672_send_command(ATcommand, "+CMQTTDISC: 0,0\r\n",TIME_VERY_SHORT))
    {
        sim7672_errorHandle();
    }
    HAL_Delay(100);
    if(sim7672_send_command("AT+CMQTTSTOP\r\n","+CMQTTSTOP: 0\r\n",TIME_VERY_SHORT))
    {
        sim7672_errorHandle();
    }
    HAL_Delay(100);
}

void mqtt_connectServer()
{
    memset(ATcommand,'\0',sizeof(ATcommand));
    //start MQTT
    if(sim7672_send_command("AT+CMQTTSTART\r\n","+CMQTTSTART: 0\r\n",TIME_VERY_SHORT)!=SIM_TRUE)
    {
        sim7672_errorHandle();
    }
    HAL_Delay(100);
    //accquire MQTT client
    sprintf(ATcommand,"AT+CMQTTACCQ=%d,\"%s\",%d\r\n",mqttClient.clientIndex,mqttClient.clientID,mqttServer.serverType);
    if(sim7672_send_command(ATcommand,"OK\r\n",TIME_VERY_SHORT)!=SIM_TRUE)
    {
        sim7672_errorHandle();
    }
    HAL_Delay(100);
    memset(ATcommand,'\0',sizeof(ATcommand));
    //connect to MQTT server
    //AT+CMQTTCONNECT=0,"tcp://demo.thingsboard.io:1883",90,1,"57OrAgUzdWLWjWE760Ir","5437ac10-9460-11ee-ad71-4d1e7bacfffa"
    sprintf(ATcommand,"AT+CMQTTCONNECT=%d,\"tcp://%s:%d\",%d,1,\"%s\",\"%s\"\r\n",\
    mqttClient.clientIndex,mqttServer.host,mqttServer.port,mqttClient.keepAliveInterval,DEVICE_USERNAME,DEVICE_PASSWORD);
    if(sim7672_send_command(ATcommand,"+CMQTTCONNECT: 0,0\r\n",TIME_LONG)!=SIM_TRUE)
    {
        sim7672_errorHandle();
    }
    HAL_Delay(100);
}

void mqtt_pub(char *topic, char *payload)
{
    int len_topic = strlen(topic);
    int len_payload=strlen(payload);

    memset(ATcommand,'\0',sizeof(ATcommand));
    sprintf(ATcommand,"AT+CMQTTTOPIC=%d,%d\r\n",mqttClient.clientIndex,len_topic);
    if(sim7672_send_command(ATcommand,">",TIME_VERY_SHORT)!=SIM_TRUE)
    {
        //sim7672_errorHandle();
    }
    //HAL_Delay(100);
    //sim7672_sendcmd_notresp(topic);
    if(sim7672_send_command(topic,"OK\r\n",TIME_VERY_SHORT)!=SIM_TRUE)
    {
        //sim7672_errorHandle();
    }
    //HAL_Delay(100);

    memset(ATcommand,'\0',sizeof(ATcommand));
    sprintf(ATcommand,"AT+CMQTTPAYLOAD=%d,%d\r\n",mqttClient.clientIndex,len_payload);
    if(sim7672_send_command(ATcommand,">",TIME_VERY_SHORT)!=SIM_TRUE)
    {
        //sim7672_errorHandle();
    }
    //HAL_Delay(100);
    if(sim7672_send_command(payload,"OK\r\n",TIME_VERY_SHORT)!=SIM_TRUE)
    {
        //sim7672_errorHandle();
    }
    //HAL_Delay(100);

    memset(ATcommand,'\0',sizeof(ATcommand));
    sprintf(ATcommand,"AT+CMQTTPUB=%d,%d,%d\r\n",mqttClient.clientIndex,mqttClient.QoS,mqttClient.keepAliveInterval);
    if(sim7672_send_command(ATcommand,"+CMQTTPUB: 0,0\r\n",TIME_VERY_SHORT)!=SIM_TRUE)
    {
        //sim7672_errorHandle();
    }
    //HAL_Delay(100);
}
/*
AT+CMQTTSUB=0
OK

+CMQTTRXSTART: 0,6,18

+CMQTTRXTOPIC: 0,6
test1z

+CMQTTRXPAYLOAD: 0,18
payload

+CMQTTRXEND: 0

+CMQTTSUB: 0,0
*/
void mqtt_sub(char *topic)
{
    int len_topic = strlen(topic);
    memset(ATcommand,'\0',sizeof(ATcommand));
    sprintf(ATcommand,"AT+CMQTTSUBTOPIC=%d,%d,%d\r\n",mqttClient.clientIndex,len_topic,mqttClient.QoS);
    if(sim7672_send_command(ATcommand,">",TIME_VERY_SHORT)!=SIM_TRUE)
    {
        sim7672_errorHandle();
    }
    HAL_Delay(100);
    //sim7672_sendcmd_notresp(topic);
    if(sim7672_send_command(topic,"OK\r\n",TIME_VERY_SHORT)!=SIM_TRUE)
    {
        sim7672_errorHandle();
    }
    HAL_Delay(100);
    memset(ATcommand,'\0',sizeof(ATcommand));
    sprintf(ATcommand,"AT+CMQTTSUB=%d\r\n",mqttClient.clientIndex);
    if(sim7672_send_command(ATcommand,"+CMQTTSUB: 0,0\r\n",TIME_LONG)!=SIM_TRUE)
    {
        sim7672_errorHandle();
    }
    HAL_Delay(100);
    //FInd Topic

    //Find Payload
    char *payloadStart = strstr(simRxResponse,"+CMQTTRXPAYLOAD:");
    if(payloadStart!=NULL)
    {
        // Di chuyển con trỏ đến sau dấu xuống dòng đầu tiên
        payloadStart=strchr(payloadStart,'\n');
        if (payloadStart != NULL) {
            // Tìm vị trí bắt đầu thực sự của payload
            payloadStart++;
            
            // Tìm vị trí kết thúc của payload
            char *payloadEnd = strstr(payloadStart, "\n");
            
            if (payloadEnd != NULL) {
                // Copy payload vào một buffer mới để xử lý
                strncpy(mqttData.payload, payloadStart, payloadEnd - payloadStart);
                mqttData.payload[payloadEnd - payloadStart] = '\0';
                log_data("payload:");
                log_data(mqttData.payload);
            }
        }
    }
}