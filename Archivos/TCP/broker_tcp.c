#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define MAX_CLIENTS 10

// Estructura para guardar a los suscriptores
struct {
    int socket;
    char topic[50];
} subscribers[MAX_CLIENTS];

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};

    // 1. socket(): Crea el punto de comunicación
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // 2. bind(): Asigna la IP y el puerto al socket
    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    
    // 3. listen(): Pone al socket a escuchar conexiones entrantes TCP
    listen(server_fd, 3);
    printf("Broker TCP escuchando en el puerto %d...\n", PORT);

    int sub_count = 0;

    while(1) {
        // 4. accept(): Acepta una nueva conexión de un Publisher o Subscriber
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        
        read(new_socket, buffer, 1024);
        
        // Lógica simple: si el mensaje empieza con "SUB", es un suscriptor
        if (strncmp(buffer, "SUB", 3) == 0) {
            subscribers[sub_count].socket = new_socket;
            strcpy(subscribers[sub_count].topic, buffer + 4); // Guarda el tema (ej. partido)
            printf("Nuevo suscriptor para el tema: %s\n", subscribers[sub_count].topic);
            sub_count++;
        } 
        // Si no, es un publicador enviando un mensaje ("PUB topic mensaje")
        else if (strncmp(buffer, "PUB", 3) == 0) {
            char topic[50], msg[200];
            sscanf(buffer + 4, "%s %[^\n]", topic, msg);
            printf("Mensaje recibido: [%s] %s\n", topic, msg);
            
            // Reenviar a los suscriptores interesados
            for (int i = 0; i < sub_count; i++) {
                if (strcmp(subscribers[i].topic, topic) == 0) {
                    send(subscribers[i].socket, msg, strlen(msg), 0);
                }
            }
            close(new_socket); // El publicador se desconecta tras enviar
        }
        memset(buffer, 0, sizeof(buffer));
    }
    return 0;
}