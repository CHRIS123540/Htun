#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include "udp_utils.h"
#include <pthread.h>


#define SERVER_IP "192.168.0.42"
#define SERVER_PORT 12345
#define BUFFER_SIZE 4096
#define DELAY_MARK "DELAY"
#define SEQ_OFFSET strlen(DELAY_MARK)

#include <linux/if_tun.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#define TUN_NAME "tun0"
int tun_create(char *dev, int flags) {
    struct ifreq ifr;
    int fd, err;

    if ((fd = open("/dev/net/tun", O_RDWR)) < 0) {
        perror("Opening /dev/net/tun");
        return fd;
    }

    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = flags;

    if (*dev) {
        strncpy(ifr.ifr_name, dev, IFNAMSIZ);
    }

    if ((err = ioctl(fd, TUNSETIFF, (void *)&ifr)) < 0) {
        perror("ioctl(TUNSETIFF)");
        close(fd);
        return err;
    }

    strcpy(dev, ifr.ifr_name);
    return fd;
}

void send_probe_packet(int client_socket, struct sockaddr_in server_addr) {
    char buffer[BUFFER_SIZE];
    struct timespec send_time;

    strcpy(buffer, DELAY_MARK);
    clock_gettime(CLOCK_MONOTONIC, &send_time);
    sprintf(buffer + SEQ_OFFSET, "%ld.%ld", send_time.tv_sec, send_time.tv_nsec);
    sendto(client_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
}

int receive_and_handle_data(int client_socket, struct sockaddr_in server_addr, int tun_fd) {
    char buffer[BUFFER_SIZE];
    struct timespec end;
    int tun_len;
    memset(buffer, 0, BUFFER_SIZE);
    int len = recvfrom(client_socket, buffer, BUFFER_SIZE, 0, NULL, NULL);
    if (len == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return 1;  // 如果没有数据，直接返回
        } else {
            perror("recvfrom");
            return 0;
        }
    } 

    struct timespec received_time;

    
    for(int i = 0;i<5;i++){
	tun_len = read(tun_fd, buffer, sizeof(buffer));
        if (tun_len > 0) {
//          printf("Received data from TUN device:\n");
	    sendto(client_socket, buffer, tun_len, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
	
	} else if (tun_len == -1 && (errno != EAGAIN && errno != EWOULDBLOCK)) {
//            perror("Reading from TUN device");
        }
	}



    // 判断是否为探测响应包
    if (strncmp(buffer, DELAY_MARK, SEQ_OFFSET) == 0) {
        sscanf(buffer + SEQ_OFFSET, "%ld.%ld", &received_time.tv_sec, &received_time.tv_nsec);
        clock_gettime(CLOCK_MONOTONIC, &end);
        double elapsed_time = (end.tv_sec - received_time.tv_sec) * 1e6 + (end.tv_nsec - received_time.tv_nsec) / 1e3; // 微秒为单位
//        printf("Time sent: %ld.%ld\n", received_time.tv_sec, received_time.tv_nsec);
//        printf("RTT: %.2lf µs\n", elapsed_time);

        // 接下来发送数据包
/*        
	for(int i = 0;i<20;i++){ 
	tun_len = read(tun_fd, buffer, sizeof(buffer));
        if (tun_len > 0) {
//触发模型
     	printf("Received data from TUN device:\n");
	    sendto(client_socket, buffer, tun_len, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
	
	} else if (tun_len == -1 && (errno != EAGAIN && errno != EWOULDBLOCK)) {
//            perror("Reading from TUN device");
        }
	}

*/
	return 0;
    } else {
        // 处理非探测包
        int written_len = write(tun_fd, buffer, len);
        if (written_len < 0) {
//            perror("Writing to TUN device");
        } else {
//            printf("Written %d bytes to TUN device.\n", written_len);
        }
    	return 1;
    }
}


struct thread_args {
    int client_socket;
    struct sockaddr_in server_addr;
    int tun_fd;
};

void *send_probe_thread(void *arg) {
    struct thread_args *args = (struct thread_args *)arg;
    int client_socket = args->client_socket;
    struct sockaddr_in server_addr = args->server_addr;

    struct timespec delay;
    delay.tv_sec = 0;
    delay.tv_nsec = 100;  // 100ns

    while (1) {
        send_probe_packet(client_socket, server_addr);
    	nanosleep(&delay, NULL);  // 阻塞 100ns
    }

    return NULL;
}

void *receive_and_handle_thread(void *arg) {
    struct thread_args *args = (struct thread_args *)arg;
    int client_socket = args->client_socket;
    struct sockaddr_in server_addr = args->server_addr;
    int tun_fd = args->tun_fd;

    while (1) {
        receive_and_handle_data(client_socket, server_addr, tun_fd);
    }

    return NULL;
}

int main() {

    int tun_fd;
    char tun_name[IFNAMSIZ];
    strcpy(tun_name, TUN_NAME);
    tun_fd = tun_create(tun_name, IFF_TUN | IFF_NO_PI);
    if (tun_fd < 0) {
        exit(1);
    }
    int flags = fcntl(tun_fd, F_GETFL, 0);
    fcntl(tun_fd, F_SETFL, flags | O_NONBLOCK);
    
    printf("TUN device %s created\n", tun_name);    
    sleep(1);

    struct sockaddr_in server_addr;
    int client_socket;

    
    client_socket = create_udp_socket();
    bind_to_local_addr(client_socket, "192.168.0.41");
    initialize_server_addr(&server_addr, SERVER_IP, SERVER_PORT);
    
    pthread_t send_thread, recv_thread;

    struct thread_args args;
    args.client_socket = client_socket;
    args.server_addr = server_addr;
    args.tun_fd = tun_fd;

    pthread_create(&send_thread, NULL, send_probe_thread, &args);
    pthread_create(&recv_thread, NULL, receive_and_handle_thread, &args);

    pthread_join(send_thread, NULL);
    pthread_join(recv_thread, NULL);

    close(client_socket);
    return 0;
    }


