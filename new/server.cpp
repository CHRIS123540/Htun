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

#define PORT 12345
#define TUN_NAME "tun0"

struct sockaddr_in client_addr; // Global variable to store client address
socklen_t client_addr_len = sizeof(client_addr); // Global variable to store client address length

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
    	// ... [No changes here]
}

void *tun_to_client_thread(void *arg) {
    int *fds = (int *)arg;
    int tun_fd = fds[0];
    int server_fd = fds[1];
    char buffer[10000];

    while (1) {
        int len = read(tun_fd, buffer, sizeof(buffer));
        if (len > 0) {
            sendto(server_fd, buffer, len, 0, (struct sockaddr *)&client_addr, client_addr_len);
        }
    }
    return NULL;
}

void *client_to_tun_thread(void *arg) {
    int *fds = (int *)arg;
    int tun_fd = fds[0];
    int server_fd = fds[1];
    char buffer[10000];

    while (1) {
        int len = recvfrom(server_fd, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, &client_addr_len);
        if (len > 0) {
            write(tun_fd, buffer, len);
        }
    }
    return NULL;
}

int main() {
    int server_fd, tun_fd;
    struct sockaddr_in address;
    int opt = 1;
    char buffer[10000];

    // Create TUN device
    char tun_name[IFNAMSIZ];
    strcpy(tun_name, TUN_NAME);
    tun_fd = tun_create(tun_name, IFF_TUN | IFF_NO_PI);
    if (tun_fd < 0) {
        exit(1);
    }
    printf("TUN device %s created\n", tun_name);

    if ((server_fd = socket(AF_INET, SOCK_DGRAM, 0)) == 0) { // Changed to SOCK_DGRAM for UDP
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    int fds[2] = {tun_fd, server_fd};

    pthread_t tun_thread, client_thread;

    // Create threads
    pthread_create(&tun_thread, NULL, tun_to_client_thread, fds);
    pthread_create(&client_thread, NULL, client_to_tun_thread, fds);

    // Wait for threads to finish
    pthread_join(tun_thread, NULL);
    pthread_join(client_thread, NULL);

    close(server_fd);
    close(tun_fd);
    return 0;
}
