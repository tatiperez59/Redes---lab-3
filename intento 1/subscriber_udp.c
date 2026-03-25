#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>


int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Uso: %s <ip> <puerto> <tema>\n", argv[0]);
        return 1;
    }
int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &serv_addr.sin_addr);

    char buffer[1024];
    sprintf(buffer, "SUB %s", argv[3]);
    sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    printf("Suscrito al tema mediante UDP: %s\n", argv[3]);
    printf("Esperando notificaciones...\n");

    struct sockaddr_in from_addr;
socklen_t addr_len = sizeof(from_addr);

    while (1) {
        memset(buffer, 0, 1024);
        int valread = recvfrom(sockfd, buffer, 1023, 0, (struct sockaddr *)&from_addr, &addr_len);
        if (valread > 0) {
            printf(" -> NOTIFICACION UDP: %s\n", buffer);
        }
    }

    close(sockfd);
return 0;
}
