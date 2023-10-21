#ifndef UDP_UTILS_H
#define UDP_UTILS_H

#include <arpa/inet.h>

int create_udp_socket();
void bind_to_local_addr(int socket, const char* local_ip);
void initialize_server_addr(struct sockaddr_in* server_addr, const char* ip, int port);

#endif // UDP_UTILS_H

