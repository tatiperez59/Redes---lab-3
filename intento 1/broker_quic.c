#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>


// Estructura del "Paquete QUIC Simulado"
typedef struct {
    int seq_num;        // Numero de secuencia
    int is_ack;         // 1 si es ACK, 0 si son datos
    char topic[50];     // Tema (suscripcion/publicacion)
    char payload[256];  // Mensaje
} QuicPacket;

#define MAX_SUBS 100

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
        perror("Error creando socket");
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
    printf("Broker QUIC-Simulado ejecutandose en puerto %d...\n", port);

    Subscriber subs[MAX_SUBS];
    int num_subs = 0;

    QuicPacket recv_packet;
    struct sockaddr_in client_addr;
socklen_t addr_len = sizeof(client_addr);

    while (1) {
        memset(&recv_packet, 0, sizeof(QuicPacket));
        int n = recvfrom(sockfd, (char *)&recv_packet, sizeof(QuicPacket), 0, (struct sockaddr *)&client_addr, &addr_len);
        
        if (n == sizeof(QuicPacket)) {
            // Manejamos Suscripciones
            if (strcmp(recv_packet.payload, "SUB") == 0) {
                if (num_subs < MAX_SUBS) {
                    subs[num_subs].addr = client_addr;
                    strcpy(subs[num_subs].topic, recv_packet.topic);
                    num_subs++;
                    printf("[BROKER] Nuevo suscriptor QUIC al tema: %s\n", recv_packet.topic);
                    
                    // Enviar ACK al suscriptor para confirmar recepcion
                    QuicPacket ack;
                    ack.seq_num = recv_packet.seq_num;
                    ack.is_ack = 1;
                    strcpy(ack.topic, recv_packet.topic);
                    strcpy(ack.payload, "ACK_SUB");
                    sendto(sockfd, (char *)&ack, sizeof(QuicPacket), 0, (struct sockaddr *)&client_addr, addr_len);
                }
            } 
            // Manejamos Publicaciones (Datos)
            else if (recv_packet.is_ack == 0) {
                printf("[BROKER] Recibido de %s | Seq: %d | Mensaje: %s\n", recv_packet.topic, recv_packet.seq_num, recv_packet.payload);
                
                // 1. Enviar Acuse de Recibo (ACK) inmediatamente al publicador
                QuicPacket ack;
                ack.seq_num = recv_packet.seq_num;
                ack.is_ack = 1;
                strcpy(ack.topic, recv_packet.topic);
                strcpy(ack.payload, "ACK");
                sendto(sockfd, (char *)&ack, sizeof(QuicPacket), 0, (struct sockaddr *)&client_addr, addr_len);
                printf("  -> (Enviado ACK %d al Publicador)\n", ack.seq_num);

                // 2. Reenviar (Broadcast) a los Suscriptores interesados
                for (int i = 0; i < num_subs; i++) {
                    if (strcmp(subs[i].topic, recv_packet.topic) == 0) {
                        // Enviamos a cada suscriptor como paquete de datos
                        sendto(sockfd, (char *)&recv_packet, sizeof(QuicPacket), 0, (struct sockaddr *)&subs[i].addr, sizeof(subs[i].addr));
                    }
                }
            }
        }
    }
close(sockfd);
return 0;
}
