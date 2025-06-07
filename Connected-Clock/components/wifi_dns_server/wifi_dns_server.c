#include "wifi_dns_server.h"

static int getQuestionLength(uint8_t *buffer, int maxLen)
{
    int i = 0;
    while (buffer[i] != 0 && i <= maxLen)
        i += buffer[i] + 1; // Label length + 1

    return i + 1 + 4; // +1 for the final 0, +4 for QTYPE and QCLASS
}

/// @brief The main task for the captive portal, normally it must be called by the DNSserver_StartSocket() function
/// @param taskParam A DNSserver struct representing the server status and informations
void DNSserverTask(void *taskParam)
{
    DNSserver *server = (DNSserver *)taskParam;
    if (!server)
    {
        ESP_LOGE(TAG_DNS, "DNSserverTask : ERROR : server passed to function but is NULL");
        vTaskDelete(NULL);
    }

    // The sender's address
    sockaddr_in_t recv_addr;
    socklen_t size = sizeof(recv_addr);
    uint8_t request[512];
    uint8_t answer[512];

    // The header of the request
    DNSheader *reqHeader;

    // The header of the answer
    DNSheader ansHeader = {
        .param = htons(MY_DNS_ANSPARAM),
        .questionCount = htons(1),
        .answerCount = htons(1),
        .additionalCount = 0,
        .authorityCount = 0,
    };

    // The body of the answer
    DNSanswer ans = {
        .name = htons(0xC00C), // pointer to the name (offset 12)
        .type = htons(0x0001), // A
        .clas = htons(0x0001), // IN
        .ttl = htonl(0),       // 5 minutes test avec 0
        .len = htons(4),       // IPv4 = 4 bytes
    };

    // The IPv4 address of the HTTP server
    ans.addr = inet_addr("192.168.4.1");

    int offset = DNS_HEADER_SIZE;

    // Main loop of the task
    // It sends to every request the same IPv4 address
    while (server->started)
    {
        offset = DNS_HEADER_SIZE;
        size = sizeof(recv_addr);

        int len = recvfrom(server->sock, request, sizeof(request), 0, (struct sockaddr *)&recv_addr, &size);
        if (len < 2) // ensuring the request is valid
            continue;

        ESP_LOGI(TAG_DNS, "%d bytes received", len);

        reqHeader = (DNSheader *)request;
        ansHeader.id = reqHeader->id;
        int questionLen = getQuestionLength(request + offset, len - offset);
        ESP_LOGI(TAG_DNS, "%d question(s), %d bytes", htons(reqHeader->questionCount), questionLen);

        memcpy(answer, &ansHeader, sizeof(ansHeader));
        memcpy(answer + offset, request + offset, questionLen);

        offset += questionLen;
        memcpy(answer + offset, &ans, sizeof(ans));
        offset += sizeof(ans);

        ESP_LOGI(TAG_DNS, "About to send %d bytes of data", offset);

        int ret = sendto(server->sock, answer, offset, 0, (struct sockaddr *)&recv_addr, size);
        if (ret < 0)
            ESP_LOGE(TAG_DNS, "sendto error : %d bytes transmitted", ret);
        else
            ESP_LOGI(TAG_DNS, "%d bytes sent !", ret);
    }
}

DNSserver *DNSserver_StartSocket()
{
    DNSserver *server = malloc(sizeof(DNSserver));
    if (!server)
    {
        ESP_LOGE(TAG_DNS, "DNSserver_StartSocket, impossible to allocate memory for the server structure\n");
        return NULL;
    }
    server->started = true;

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(53),
        .sin_addr.s_addr = htonl(INADDR_ANY),
    };
    bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));

    server->sock = sock;

    xTaskCreate(DNSserverTask, "Captive Portal", 4096, server, 5, &server->task);
    printf("Task_created\n");

    return NULL;
}