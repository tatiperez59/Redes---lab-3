#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>


// Estructura del "Paquete QUIC Simulado"
typedef struct {
    int seq_num;        
    int is_ack;         
    char topic[50];     
    char payload[256];  
} QuicPacket;

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Uso: %s <ip_broker> <puerto> <tema>\n", argv[0]);
        return 1;
    }
int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &serv_addr.sin_addr);

    // 1. Suscribirse utilizando handshake simple
    QuicPacket sub_packet;
    sub_packet.seq_num = 0;
    sub_packet.is_ack = 0;
    strncpy(sub_packet.topic, argv[3], 49);
    strcpy(sub_packet.payload, "SUB");

    sendto(sockfd, (char *)&sub_packet, sizeof(QuicPacket), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    printf("Suscripcion enviada al broker. Tema: %s\n", argv[3]);
    printf("Esperando notificaciones...\n");

    struct sockaddr_in from_addr;
socklen_t addr_len = sizeof(from_addr);

    QuicPacket recv_packet;
    int expected_seq = 1; // Para llevar la cuenta si hay desorden 

    while (1) {
        memset(&recv_packet, 0, sizeof(QuicPacket));
        int valread = recvfrom(sockfd, (char *)&recv_packet, sizeof(QuicPacket), 0, (struct sockaddr *)&from_addr, &addr_len);
        
        if (valread == sizeof(QuicPacket)) {
            // Ignorar ACKs puros, solo queremos datos
            if (recv_packet.is_ack == 0) {
                printf("[SUB] -> Notificacion | Seq %d: %s\n", recv_packet.seq_num, recv_packet.payload);
                
                // Muestra de un potencial desorden (UDP lo permite, esto te ayuda a responder las preguntas del taller)
                if (recv_packet.seq_num > expected_seq) {
                     printf("  [!] (Aviso: un paquete llego tarde o se perdio antes que este. Esperaba %d, llego %d)\n", expected_seq, recv_packet.seq_num);
                     expected_seq = recv_packet.seq_num + 1;
                } else if (recv_packet.seq_num == expected_seq) {
                     expected_seq++;
                }
            } else if (recv_packet.is_ack == 1 && strcmp(recv_packet.payload, "ACK_SUB") == 0) {
                 printf("[SUB] Conexion de suscripcion confirmada (Recibido ACK del broker)\n");
            }
        }
    }

    close(sockfd);
return 0;
}
