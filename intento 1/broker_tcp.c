#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>


#define MAX_CLIENTS 100
#define BUFFER_SIZE 1024

typedef struct {
    int fd;
    int is_sub;
    char topic[50];
} Client;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Uso: %s <puerto>\n", argv[0]);
        return 1;
    }
int port = atoi(argv[1]);
    int server_fd;
    struct sockaddr_in server_addr;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Error creando socket");
        return 1;
    }
    
    // Configurar socket (SO_REUSEADDR)
int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind fallido");
        return 1;
    }
    
    if (listen(server_fd, 10) < 0) {
        perror("Listen fallido");
        return 1;
    }
    printf("Broker TCP escuchando en puerto %d...\n", port);

    Client clients[MAX_CLIENTS];
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].fd = 0;
        clients[i].is_sub = 0;
        memset(clients[i].topic, 0, 50);
    }

    fd_set readfds;
    int max_sd, sd;

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        max_sd = server_fd;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            sd = clients[i].fd;
            if (sd > 0) FD_SET(sd, &readfds);
            if (sd > max_sd) max_sd = sd;
        }

        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if (activity < 0) {
            printf("Error en select\n");
            continue;
        }

        // Nueva conexion
        if (FD_ISSET(server_fd, &readfds)) {
            struct sockaddr_in client_addr;
socklen_t addrlen = sizeof(client_addr);
            int new_socket = accept(server_fd, (struct sockaddr *)&client_addr, &addrlen);
            if (new_socket >= 0) {
                for (int i = 0; i < MAX_CLIENTS; i++) {
                    if (clients[i].fd == 0) {
                        clients[i].fd = new_socket;
                        clients[i].is_sub = 0;
                        memset(clients[i].topic, 0, 50);
                        break;
                    }
                }
            }
        }

        // Leer datos
        for (int i = 0; i < MAX_CLIENTS; i++) {
            sd = clients[i].fd;
            if (sd > 0 && FD_ISSET(sd, &readfds)) {
                char buffer[BUFFER_SIZE];
                memset(buffer, 0, BUFFER_SIZE);
                int valread = recv(sd, buffer, BUFFER_SIZE - 1, 0);
                if (valread <= 0) {
                    close(sd);
                    clients[i].fd = 0;
                } else {
                    if (strncmp(buffer, "SUB ", 4) == 0) {
                        clients[i].is_sub = 1;
                        sscanf(buffer + 4, "%49s", clients[i].topic);
                        printf("Nuevo suscriptor para el tema: %s\n", clients[i].topic);
                    } else if (strncmp(buffer, "PUB ", 4) == 0) {
                        char topic[50];
                        char msg[BUFFER_SIZE];
                        memset(topic, 0, 50);
                        memset(msg, 0, BUFFER_SIZE);
                        sscanf(buffer + 4, "%49s %[^\n]", topic, msg);
                        printf("Mensaje recibido para %s: %s\n", topic, msg);
                        
                        // Reenviar a suscriptores
                        for (int j = 0; j < MAX_CLIENTS; j++) {
                            if (clients[j].fd > 0 && clients[j].is_sub && strcmp(clients[j].topic, topic) == 0) {
                                send(clients[j].fd, msg, strlen(msg), 0);
                            }
                        }
                    }
                }
            }
        }
    }
return 0;
}
