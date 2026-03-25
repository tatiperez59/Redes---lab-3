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
    if (sock < 0) {
        perror("Socket fallback");
        return 1;
    }
    
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &serv_addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Conexion fallida");
        return 1;
    }

    char topic[50];
    strncpy(topic, argv[3], 49);
    topic[49] = '\0';

    printf("Iniciando publicacion en el tema: %s\n", topic);
    for (int i = 1; i <= 10; i++) {
        char buffer[1024];
        char msg[256];
        sprintf(msg, "Gol del evento %s (Mensaje TCP #%d)", topic, i);
        sprintf(buffer, "PUB %s %s", topic, msg);
        send(sock, buffer, strlen(buffer), 0);
        printf("Enviado: %s\n", msg);
sleep(1);
    }

    close(sock);
return 0;
}
