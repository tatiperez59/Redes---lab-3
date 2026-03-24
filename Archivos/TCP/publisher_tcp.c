#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    // El formato es "PUB <partido> <mensaje>"
    char *mensaje = "PUB EquipoAvsEquipoB Gol de Equipo A al minuto 32";

    sock = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    // connect(): Establece el canal confiable con el broker (3-way handshake)
    connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    
    // send(): Envía el mensaje
    send(sock, mensaje, strlen(mensaje), 0);
    printf("Mensaje publicado.\n");
    
    close(sock);
    return 0;
}