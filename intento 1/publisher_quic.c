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

    // CONFIGURAR TIMEOUT PARA EL UDP (Simulando mecanismo QUIC de retransmision)
    // Usaremos un timeout MUY CORTO (500 ms) para reaccionar rapido ante la perdida de paquetes.
struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 500000; // 500 milisegundos en Linux/Mac
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));

    char topic[50];
    strncpy(topic, argv[3], 49);
    topic[49] = '\0';

    printf("Iniciando Publicador QUIC para: %s\n", topic);
    
    // Nuestro state principal
    int sequence_number = 1;
    QuicPacket ack_packet;
    
socklen_t addr_len = sizeof(serv_addr);

    // Enviar 10 mensajes segun indicaciones del LAB
    for (int i = 1; i <= 10; i++) {
        QuicPacket send_packet;
        send_packet.seq_num = sequence_number;
        send_packet.is_ack = 0; // Son datos
        strcpy(send_packet.topic, topic);
        sprintf(send_packet.payload, "Gol del evento %s (Mensaje QUIC #%d)", topic, i);
        
        int ack_received = 0;
        int retries = 0;
        
        while (!ack_received && retries < 5) { // Maximo 5 reintentos
            // Mandamos el paquete
            sendto(sockfd, (char *)&send_packet, sizeof(QuicPacket), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
            printf("[PUB] Enviado Seq %d: %s | Esperando ACK...\n", send_packet.seq_num, send_packet.payload);
            
            // Esperamos el ACK (el recvfrom esta bloqueado por maximo el timeout configurado arriba)
            memset(&ack_packet, 0, sizeof(QuicPacket));
            int n = recvfrom(sockfd, (char *)&ack_packet, sizeof(QuicPacket), 0, (struct sockaddr *)&serv_addr, &addr_len);
            
            if (n < 0 || n != sizeof(QuicPacket)) {
                // El socket arrojara este error debido al timeout estricto
                printf("  [!] TIMEOUT: Perdimos paquete o ACK. Retransmitiendo (Seq: %d)...\n", sequence_number);
                retries++;
            } else if (ack_packet.is_ack == 1 && ack_packet.seq_num == sequence_number) {
                // Llego el ACK correcto
                printf("  [OK] ACK recibido %d!\n", ack_packet.seq_num);
                ack_received = 1;
            } else {
                 printf("  [?] Se recibio paquete desconocido. Esperando ACK real.\n");
            }
        }
        
        if (retries >= 5) {
            printf("[FATAL] Fallo al entregar mensaje Seq %d tras 5 reintentos. Broker caido?\n", sequence_number);
        }

        sequence_number++; // Incremento para el proximo paquete

#ifdef _WIN32
        Sleep(1000); // 1 segundo entre envios exitosos
#else
        sleep(1);
#endif
    }

    close(sockfd);
return 0;
}
