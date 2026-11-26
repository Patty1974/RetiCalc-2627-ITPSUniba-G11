#if defined WIN32 /* Direttiva per compilazione su Windows */
/* Inserimento librerie utili, definizione tipo per la lunghezza */
/* dell'indirizzo socket */
#include <winsock2.h>
typedef int socklen_t;
#else
#define closesocket close
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* Definizione della porta di ascolto del server e */
/* della dimensione massima del buffer */
#define PORTA 5000
#define BUFFERSIZE 512

int main(void) {
#if defined WIN32
	/* Inizializzazione della libreria Winsock */
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Errore WSAStartup\n");
        return -1;
    }
#endif

    int sd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sd < 0) {
        printf("Errore creazione socket\n");
#if defined WIN32
        WSACleanup();
#endif
        return -1;
    }
    /* Struttura che contiene l'indirizzo IPv4 e la porta della socket */
    struct sockaddr_in sad;
    memset(&sad, 0, sizeof(sad));
    sad.sin_family = AF_INET;
    sad.sin_addr.s_addr = htonl(INADDR_ANY);
    sad.sin_port = htons(PORTA);
    /* Associa la socket all'indirizzo e alla porta del server; segnala */
    /* errore se fallisce */
    if (bind(sd, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
        printf("Errore bind\n");
        closesocket(sd);
#if defined WIN32
        WSACleanup();
#endif
        return -1;
    }
    /* Mette la socket in ascolto per le connessioni in ingresso; segnala */
    /* errore se fallisce */
    if (listen(sd, 5) < 0) {
        printf("Errore listen\n");
        closesocket(sd);
#if defined WIN32
        WSACleanup();
#endif
        return -1;
    }

    printf("Server in ascolto sulla porta %d...\n", PORTA);
    fflush(stdout);
    /* Ciclo infinito: accetta un client alla volta, mostra il suo IP e conferma */
    /* la connessione */
    while (1) {
        struct sockaddr_in cad;
        socklen_t cad_len = sizeof(cad);
        int client_sd = accept(sd, (struct sockaddr *)&cad, &cad_len);
        if (client_sd < 0) {
            continue;
        }

        printf("connesso con il client il cui nome IP: %s\n", inet_ntoa(cad.sin_addr));

        char *msg = "connessione avvenuta";
        send(client_sd, msg, strlen(msg), 0);

        char buf[BUFFERSIZE];
        int n1, n2, n3, res;
        /* Ciclo di ricezione ed elaborazione dei messaggi finché il client invia dati */
        while (1) {
            int bytes = recv(client_sd, buf, BUFFERSIZE - 1, 0);
            if (bytes <= 0) {
                break;
            }
            buf[bytes] = '\0';

            if (sscanf(buf, "%d %d %d", &n1, &n2, &n3) == 3) {
                res = n1 + n2 - n3;
                printf("la somma di %d+%d-%d=%d\n", n1, n2, n3, res);

                if (res == 100) {
                    strcpy(buf, "QUIT");
                    send(client_sd, buf, strlen(buf), 0);
                    break;
                } else {
                    sprintf(buf, "%d", res);
                    send(client_sd, buf, strlen(buf), 0);
                }
            }
        }

        closesocket(client_sd);
    }

    closesocket(sd);
#if defined WIN32
    WSACleanup();
#endif
    return 0;
}
