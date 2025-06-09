// Samuel CHEVIGNARD 2A
// wifi_dns_server

#pragma once

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "sys/socket.h"

#define TAG_DNS "DNS_server"

#define UDP_PORT 53
#define DNS_HEADER_SIZE 12

#define MY_IP "192.168.4.1"
#define MY_DNS_ANSPARAM 0x8180

typedef struct sockaddr_in sockaddr_in_t;
typedef struct sockaddr sockaddr_t;

typedef struct DNSserver
{
    bool started;
    int sock;
    sockaddr_t addr;
    TaskHandle_t task;
} DNSserver;

#pragma pack(push, 1)
typedef struct DNSheader
{
    uint16_t id;
    uint16_t param;
    uint16_t questionCount;
    uint16_t answerCount;
    uint16_t authorityCount;
    uint16_t additionalCount;
} DNSheader;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct DNSanswer
{
    uint16_t name;
    uint16_t type;
    uint16_t clas;
    uint32_t ttl;
    uint16_t len;
    uint32_t addr;
} DNSanswer;
#pragma pack(pop)

/// @brief Start the DNS server to do a captive portal
/// @return The server's name
DNSserver *DNSserver_StartSocket();
