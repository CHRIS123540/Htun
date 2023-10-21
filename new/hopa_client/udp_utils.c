#include "udp_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

int create_udp_socket() {
    int client_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (client_socket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // 设置非阻塞模式
    int flags = fcntl(client_socket, F_GETFL, 0);
    fcntl(client_socket, F_SETFL, flags | O_NONBLOCK);
    return client_socket;
}

void bind_to_local_addr(int socket, const char* local_ip) {
    struct sockaddr_in local_addr;
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = inet_addr(local_ip);
    local_addr.sin_port = htons(0); // 任意端口

    if (bind(socket, (struct sockaddr*)&local_addr, sizeof(local_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
}

void initialize_server_addr(struct sockaddr_in* server_addr, const char* ip, int port) {
    memset(server_addr, 0, sizeof(*server_addr));
    server_addr->sin_family = AF_INET;
    server_addr->sin_addr.s_addr = inet_addr(ip);
    server_addr->sin_port = htons(port);
}
