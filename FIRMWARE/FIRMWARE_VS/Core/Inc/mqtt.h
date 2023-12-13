
#ifndef _MQTT_H__
#define _MQTT_H__

#include "sim4g_lte.h"
#include "string.h"

#define TOPIC               "v1/devices/me/telemetry" //topic use for all device
#define DEVICE_USERNAME     "pmaaTVZMDASsADuo1wBG"
#define DEVICE_PASSWORD     "3b50e5e0-998d-11ee-bf05-ef1fe94b65d9"
// #define DEVICE_TRACKING_USERNAME    "Xo96QR3fcjoU9RdByS9K"
// #define DEVICE_TRACKING_PASSWORD    "f4baea20-9457-11ee-ad71-4d1e7bacfffa"

// #define DEVICE_SENSORS_USERNAME     "IypR3HiB3jw34hfJpVN1"
// #define DEVICE_SENSORS_PASSWORD     "98965ef0-9583-11ee-b2a0-f9cb933ed214"

// #define DEVICE_STATUS_USERNAME      "TBaH0gM38qEv1h10yde0"
// #define DEVICE_STATUS_PASSWORD      "21e0e730-9461-11ee-ad71-4d1e7bacfffa"

// #define DEVICE_RFID_USERNAME        "E40hrkVcQmydWNH8R8e1"
// #define DEVICE_RFID_PASSWORD        "703c7370-957b-11ee-b2a0-f9cb933ed214"

// #define DEVICE_FUEL_USERNAME        "RA5F4WLDc6NqMh5MyrEr"
// #define DEVICE_FUEL_PASSWORD        "e09d98e0-9460-11ee-ad71-4d1e7bacfffa"
//#define TOPIC_TEST  "DATN_topicTest"
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