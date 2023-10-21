#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define SERVER_PORT 12345
#define BUFFER_SIZE 4096
#define DELAY_MARK "DELAY"
#define SEQ_OFFSET strlen(DELAY_MARK)
#define REPORT_INTERVAL 1 // 每1秒报告一次
#define SPECIFIC_IP "192.168.0.42" // 要绑定的特定IP地址

int main() {
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFFER_SIZE] = {0};
    int server_socket;
    socklen_t client_addr_len = sizeof(client_addr);
    time_t start_time, current_time;

    unsigned long long total_bytes_received = 0;
    unsigned long long seq_number;

    server_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (server_socket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SPECIFIC_IP);
    server_addr.sin_port = htons(SERVER_PORT);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    printf("Server bound to IP %s and listening on port %d...\n", SPECIFIC_IP, SERVER_PORT);

    time(&start_time);

    while (1) {

    int bytes_read = recvfrom(server_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &client_addr_len);
    
    if(bytes_read < 0) {
        perror("Receive failed");
        continue;
    }

    total_bytes_received += bytes_read;

    // 打印接收到的数据的前缀
    char prefix[10] = {0};
    strncpy(prefix, buffer, SEQ_OFFSET);
    printf("Received prefix: %s\n", prefix);

    // 判断是否为探测包
    if (strncmp(buffer, DELAY_MARK, SEQ_OFFSET) == 0) {
        sscanf(buffer + SEQ_OFFSET, "%llu", &seq_number);
        printf("Received Probe: Sequence number: %llu\n", seq_number);
        sendto(server_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, client_addr_len);
    } else {
        // 数据包的处理逻辑
        printf("Received Data Packet.\n");
    }

        time(&current_time);
        if (difftime(current_time, start_time) >= REPORT_INTERVAL) {
            double recv_rate_mbps = (total_bytes_received * 8.0) / (1024 * 1024 * difftime(current_time, start_time));
            printf("Receive Rate: %.2lf Mbps\n", recv_rate_mbps);
            time(&start_time);
            total_bytes_received = 0;
        }
    }

    close(server_socket);
    return 0;
}

