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

    char topic[50];
    strncpy(topic, argv[3], 49);
    topic[49] = '\0';

    printf("Iniciando publicacion en el tema: %s (UDP)\n", topic);
    for (int i = 1; i <= 10; i++) {
        char buffer[1024];
        char msg[256];
        sprintf(msg, "Gol del evento %s (Mensaje UDP #%d)", topic, i);
        sprintf(buffer, "PUB %s %s", topic, msg);
        sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
        printf("Enviado: %s\n", msg);
sleep(1);
    }

    close(sockfd);
return 0;
}
