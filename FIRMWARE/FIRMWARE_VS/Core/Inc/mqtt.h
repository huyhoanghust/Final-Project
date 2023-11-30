
#ifndef _MQTT_H__
#define _MQTT_H__

#include "sim4g_lte.h"
#include "string.h"

typedef struct
{
    char *host;
    uint16_t port;
    int serverType;
} mqttServer_t;

typedef struct
{
    char *username;
    char *pass;
    char *clientID;
    unsigned short keepAliveInterval;
    int clientIndex;
    int QoS;
} mqttClient_t;

typedef struct
{
    char topic[20];
    char payload[100];
} mqttData_t;
// typedef struct
// {
//     mqttServer_t mqttServer;
//     mqttClient_t mqttClient;
// } MQTT_t;
void mqtt_disconnectServer();
void mqtt_connectServer();
void mqtt_pub(char *topic, char *payload);
void mqtt_sub(char *topic);

#endif