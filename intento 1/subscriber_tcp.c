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
int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &serv_addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Conexion fallida");
        return 1;
    }

    char buffer[1024];
    sprintf(buffer, "SUB %s", argv[3]);
    send(sock, buffer, strlen(buffer), 0);
    printf("Suscrito al tema: %s (TCP)\n", argv[3]);
    printf("Esperando notificaciones...\n");

    while (1) {
        memset(buffer, 0, 1024);
        int valread = recv(sock, buffer, 1023, 0);
        if (valread <= 0) {
            printf("Conexion cerrada por el broker\n");
            break;
        }
        printf(" -> NOTIFICACION: %s\n", buffer);
    }

    close(sock);
return 0;
}
