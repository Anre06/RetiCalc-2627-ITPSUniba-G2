#if defined WIN32
    #include <winsock.h>
#else
    #define closesocket close
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFERSIZE 512

void ErrorHandler(char *errorMessage) {
    printf("%s", errorMessage);
}

void ClearWinSock() {
#if defined WIN32
    WSACleanup();
#endif
}

int main(void) {
	printf("--- CLIENT ---\n\n");
	
#if defined WIN32
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("Error at WSAStartup\n");
        return -1;
    }
#endif

    char serverIP[32];
    int serverPort;
    printf("Inserisci l'indirizzo IP del server (indirizzo IP del computer o 127.0.0.1 ): ");
    scanf("%31s", serverIP);
    printf("Inserisci il numero di porta del server (5376): ");
    scanf("%d", &serverPort);

    int Csocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (Csocket < 0) {
        ErrorHandler("Creazione socket fallita\n");
        ClearWinSock();
        return -1;
    }

    struct sockaddr_in sad;
    memset(&sad, 0, sizeof(sad));
    sad.sin_family = AF_INET;
    sad.sin_addr.s_addr = inet_addr(serverIP);
    sad.sin_port = htons(serverPort);

    if (connect(Csocket, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
        ErrorHandler("Connessione fallita\n");
        closesocket(Csocket);
        ClearWinSock();
        return -1;
    }

    char buf[BUFFERSIZE];
    memset(buf, 0, BUFFERSIZE);
    int bytesRcvd = recv(Csocket, buf, BUFFERSIZE - 1, 0);
    if (bytesRcvd <= 0) {
        ErrorHandler("recv() fallita o connessione chiusa prematuramente\n");
        closesocket(Csocket);
        ClearWinSock();
        return -1;
    }
    printf("Messaggio dal server: %s\n", buf);

    int n1, n2, n3;
    char sendBuf[BUFFERSIZE];

    while (1) {
        printf("\nInserisci il primo numero intero: ");
        if (scanf("%d", &n1) != 1) {
            printf("Input non valido.\n");
            break;
        }

        printf("Inserisci il secondo numero intero: ");
        if (scanf("%d", &n2) != 1) {
            printf("Input non valido.\n");
            break;
        }

        printf("Inserisci il terzo numero intero: ");
        if (scanf("%d", &n3) != 1) {
            printf("Input non valido.\n");
            break;
        }

        sprintf(sendBuf, "%d %d %d", n1, n2, n3);
        int sendLen = strlen(sendBuf) + 1;

        if (send(Csocket, sendBuf, sendLen, 0) != sendLen) {
            ErrorHandler("send() ha inviato un numero di byte diverso da quello previsto\n");
            break;
        }

        memset(buf, 0, BUFFERSIZE);
        bytesRcvd = recv(Csocket, buf, BUFFERSIZE - 1, 0);
        if (bytesRcvd <= 0) {
            ErrorHandler("recv() fallita o connessione chiusa prematuramente\n");
            break;
        }

        printf("Risposta del server: %s\n", buf);

        if (strcmp(buf, "QUIT") == 0) {
            break;
        }
    }

    closesocket(Csocket);
    ClearWinSock();

#if defined WIN32
    system("pause");
#endif

    return 0;
}