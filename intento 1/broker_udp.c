#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>


#define MAX_SUBS 100
#define BUFFER_SIZE 1024

typedef struct {
    struct sockaddr_in addr;
    char topic[50];
} Subscriber;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Uso: %s <puerto>\n", argv[0]);
        return 1;
    }
int port = atoi(argv[1]);
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        return 1;
    }
    
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind fallido");
        return 1;
    }
    printf("Broker UDP escuchando en puerto %d...\n", port);

    Subscriber subs[MAX_SUBS];
    int num_subs = 0;

    char buffer[BUFFER_SIZE];
    struct sockaddr_in client_addr;
socklen_t addr_len = sizeof(client_addr);

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0, (struct sockaddr *)&client_addr, &addr_len);
        if (n > 0) {
            if (strncmp(buffer, "SUB ", 4) == 0) {
                if (num_subs < MAX_SUBS) {
                    char topic[50];
                    sscanf(buffer + 4, "%49s", topic);
                    subs[num_subs].addr = client_addr;
                    strcpy(subs[num_subs].topic, topic);
                    num_subs++;
                    printf("Nuevo suscriptor UDP para el tema: %s\n", topic);
                }
            } else if (strncmp(buffer, "PUB ", 4) == 0) {
                char topic[50];
                char msg[BUFFER_SIZE];
                sscanf(buffer + 4, "%49s %[^\n]", topic, msg);
                printf("Mensaje UDP recibido para %s: %s\n", topic, msg);
                
                // Reenviar datagrama a interesados
                for (int i = 0; i < num_subs; i++) {
                    if (strcmp(subs[i].topic, topic) == 0) {
                        sendto(sockfd, msg, strlen(msg), 0, (struct sockaddr *)&subs[i].addr, sizeof(subs[i].addr));
                    }
                }
            }
        }
    }
close(sockfd);
return 0;
}
