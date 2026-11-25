#if defined WIN32 /* Direttiva per compilazione su Windows */
/* Inserimento librerie utili */
#include <winsock2.h>
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
/* Dimensione massima del buffer */
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
    /* Richiesta all'utente di IP e porta del server */
    char ip[32];
    int porta;
    printf("Inserisci l'IP del server: ");
    fflush(stdout);
    scanf("%s", ip);

    printf("Inserisci la porta del server: ");
    fflush(stdout);
    scanf("%d", &porta);

    int sd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sd < 0) {
        printf("Errore creazione socket\n");
#if defined WIN32
        WSACleanup();
#endif
        return -1;
    }
    /* Struttura che contiene l'indirizzo IPv4 e la porta del server */
    struct sockaddr_in sad;
    memset(&sad, 0, sizeof(sad));
    sad.sin_family = AF_INET;
    sad.sin_addr.s_addr = inet_addr(ip);
    sad.sin_port = htons(porta);
    /* Stabilisce la connessione TCP verso il server */
    if (connect(sd, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
        printf("Connessione fallita\n");
        closesocket(sd);
#if defined WIN32
        WSACleanup();
#endif
        return -1;
    }

    char buf[BUFFERSIZE];
    int bytes = recv(sd, buf, BUFFERSIZE - 1, 0);
    if (bytes <= 0) {
        printf("Errore ricezione\n");
        closesocket(sd);
#if defined WIN32
        WSACleanup();
#endif
        return -1;
    }
    buf[bytes] = '\0';
    printf("%s\n", buf);

    int n1, n2, n3;
    /* Ciclo di scambio messaggi: invia i tre interi e legge la risposta finché non arriva "QUIT" */
    while (1) {
        printf("Inserisci tre interi (n1 n2 n3): ");
        fflush(stdout);
        scanf("%d %d %d", &n1, &n2, &n3);

        sprintf(buf, "%d %d %d", n1, n2, n3);
        send(sd, buf, strlen(buf), 0);

        bytes = recv(sd, buf, BUFFERSIZE - 1, 0);
        if (bytes <= 0) {
            break;
        }
        buf[bytes] = '\0';
        buf[strcspn(buf, "\r\n")] = '\0';

        printf("Risposta del server: %s\n", buf);

        if (strcmp(buf, "QUIT") == 0) {
            break;
        }
    }

    closesocket(sd);
#if defined WIN32
    WSACleanup();
#endif
    return 0;
}
