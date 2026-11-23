#if defined WIN32
    #include <winsock.h>
    typedef int socklen_t;
#else
    #define closesocket close
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <string.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#define PROTOPORT 5376 
#define QLEN 2
#define BUFFERSIZE 512

void ErrorHandler(char *errorMessage) {
    printf("%s", errorMessage);
}

void ClearWinSock() {
#if defined WIN32
    WSACleanup();
#endif
}

void HandleClient(int clientSocket) {
    char buf[BUFFERSIZE];

    char *welcomeMsg = "connessione avvenuta";
    int msgLen = strlen(welcomeMsg) + 1;
    if (send(clientSocket, welcomeMsg, msgLen, 0) != msgLen) {
        ErrorHandler("Errore nell'invio del messaggio di benvenuto\n");
        closesocket(clientSocket);
        return;
    }

    while (1) {
        memset(buf, 0, BUFFERSIZE);
        int bytesRcvd = recv(clientSocket, buf, BUFFERSIZE - 1, 0);

        if (bytesRcvd <= 0) {
            printf("Client disconnesso.\n\n");
            break;
        }

        int n1, n2, n3;
        if (sscanf(buf, "%d %d %d", &n1, &n2, &n3) == 3) {
            int res = n1 + n2 - n3;
            printf("Risultato: %d + %d - %d = %d\n", n1, n2, n3, res);

            char response[BUFFERSIZE];
            if (res == 100) {
                strcpy(response, "QUIT");
            } else {
                sprintf(response, "%d", res);
            }

            int respLen = strlen(response) + 1;
            if (send(clientSocket, response, respLen, 0) != respLen) {
                ErrorHandler("Errore nell'invio della risposta\n");
                break;
            }

            if (res == 100) {
                printf("Inviato QUIT al client, chiusura sessione\n\n");
                break;
            }
        }
    }

    closesocket(clientSocket);
}

int main(int argc, char *argv[]) {
    int port;

	printf("--- SERVER ---\n\n");

    if (argc > 1) {
        port = atoi(argv[1]);
    } else {
        port = PROTOPORT;
    }

    if (port < 0) {
        printf("Numero di porta non valido: %s\n", argv[1]);
        return 0;
    }

#if defined WIN32
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        ErrorHandler("Errore in WSAStartup()\n");
        return 0;
    }
#endif

    int MySocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (MySocket < 0) {
        ErrorHandler("Creazione socket fallita\n");
        ClearWinSock();
        return -1;
    }

    struct sockaddr_in sad;
    memset(&sad, 0, sizeof(sad));
    sad.sin_family = AF_INET;
    sad.sin_addr.s_addr = htonl(INADDR_ANY);
    sad.sin_port = htons(port);

    if (bind(MySocket, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
        ErrorHandler("bind() fallita\n");
        closesocket(MySocket);
        ClearWinSock();
        return -1;
    }

    if (listen(MySocket, QLEN) < 0) {
        ErrorHandler("listen() fallita\n");
        closesocket(MySocket);
        ClearWinSock();
        return -1;
    }

    struct sockaddr_in cad;
    int clientSocket;
    socklen_t clientLen;

    printf("Server avviato sulla porta %d. In attesa di connessioni...\n", port);

    while (1) {
        clientLen = sizeof(cad);
        clientSocket = accept(MySocket, (struct sockaddr *)&cad, &clientLen);
        if (clientSocket < 0) {
            ErrorHandler("accept() fallita\n");
            closesocket(MySocket);
            ClearWinSock();
            return 0;
        }

        printf("Connesso con il client il cui indirizzo IP: %s\n", inet_ntoa(cad.sin_addr));

        HandleClient(clientSocket);
    }

    closesocket(MySocket);
    ClearWinSock();
    return 0;
}