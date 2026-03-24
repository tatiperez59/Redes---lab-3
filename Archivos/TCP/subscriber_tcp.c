#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char *sub_msg = "SUB EquipoAvsEquipoB"; // Tema de interés
    char buffer[1024] = {0};

    sock = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    
    // Se suscribe
    send(sock, sub_msg, strlen(sub_msg), 0);
    printf("Suscrito a: EquipoAvsEquipoB. Esperando noticias...\n");

    // Loop infinito para recibir actualizaciones en vivo
    while(1) {
        int valread = read(sock, buffer, 1024);
        if(valread > 0) {
            printf("ACTUALIZACIÓN: %s\n", buffer);
            memset(buffer, 0, sizeof(buffer));
        }
    }
    return 0;
}