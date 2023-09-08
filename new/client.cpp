#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <pthread.h>

#define SERVER_IP "192.168.0.213"
#define PORT 12345
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

void *tun_to_sock_thread(void *arg) {
    int *fds = (int *)arg;
    int tun_fd = fds[0];
    int sock = fds[1];
    char buffer[10000];
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr);

    while (1) {
        int len = read(tun_fd, buffer, sizeof(buffer));
//	printf("len %d\n",len);
	if (len > 0) {
            sendto(sock, buffer, len, 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
        }
    }
    return NULL;
}

void *sock_to_tun_thread(void *arg) {
    int *fds = (int *)arg;
    int tun_fd = fds[0];
    int sock = fds[1];
    char buffer[10000];

    while (1) {
        int len = recv(sock, buffer, sizeof(buffer), 0);
        if (len > 0) {
            write(tun_fd, buffer, len);
        }
    }
    return NULL;
}

int main() {
    int sock = 0, tun_fd;
    struct sockaddr_in serv_addr;
    char buffer[10000];

    // Create TUN device
    char tun_name[IFNAMSIZ];
    strcpy(tun_name, TUN_NAME);
    tun_fd = tun_create(tun_name, IFF_TUN | IFF_NO_PI);
    if (tun_fd < 0) {
        exit(1);
    }
    printf("TUN device %s created\n", tun_name);
    sleep(5);

    // Create UDP socket
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    // Main loop: read data from TUN and server, then forward
    int fds[2] = {tun_fd, sock};

    pthread_t tun_thread, sock_thread;

    // Create threads
    pthread_create(&tun_thread, NULL, tun_to_sock_thread, fds);
    pthread_create(&sock_thread, NULL, sock_to_tun_thread, fds);

    // Wait for threads to finish
    pthread_join(tun_thread, NULL);
    pthread_join(sock_thread, NULL);

    close(sock);
    close(tun_fd);
    return 0;
}
