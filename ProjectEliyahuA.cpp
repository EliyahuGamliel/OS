//Eliyahu Gamliel - Project part A - Client

#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

char resolveOption(char input[]) {
    if (!strcmp(input, "Ping")) return 'p';
    if (!strcmp(input, "Run")) return 'r';
    if (!strcmp(input, "Update")) return 'u';
    if (!strcmp(input, "Version")) return 'v';
    return 'e';
}

int __cdecl main(int argc, char** argv)
{
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo* result = NULL,
        * ptr = NULL,
        hints;
    char recvbuf[DEFAULT_BUFLEN];
    int iResult;
    int recvbuflen = DEFAULT_BUFLEN;

    //// Validate the parameters
    //if (argc != 2) {
    //    printf("usage: %s server-name\n", argv[0]);
    //    return 1;
    //}

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo("127.0.0.1", DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Attempt to connect to an address until one succeeds
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
            ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        // Connect to server.
        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

    char choice[8];
    // Receive until the peer shuts down the connection
    do {
        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
        scanf("%s", &choice);
        if (iResult > 0) {
            switch (resolveOption(choice))
            {
            ///////////
            //LEVEL 2//
            ///////////
            case 'p':
            {
                send(ConnectSocket, choice, strlen(choice) + 1, 0);
                recv(ConnectSocket, recvbuf, recvbuflen, 0);
                printf("%s", recvbuf);
                break;
            }
            ///////////
            //LEVEL 3//
            ///////////
            case 'r':
            {
                PCHAR path = (PCHAR)malloc(sizeof(CHAR) * 256);
                fgets(path, 2, stdin);
                fgets(path, 256, stdin);
                path[strlen(path) - 1] = '\0';
                send(ConnectSocket, choice, strlen(choice) + 1, 0);
                send(ConnectSocket, path, strlen(path) + 1, 0);
                break;
            }
            ///////////
            //LEVEL 4//
            ///////////
            case 'u':
            {
                PCHAR path = (PCHAR)malloc(sizeof(CHAR) * 256);
                fgets(path, 2, stdin);
                scanf_s("%s",path, 256);
                send(ConnectSocket, choice, strlen(choice) + 1, 0);
                char buf[65536];
                HANDLE file = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                DWORD fS = GetLastError();
                DWORD fSize = 0;
                ReadFile(file, buf, 65536, &fSize, NULL);
                char File_Size[1024];
                sprintf_s(File_Size, 1024, "%d", fSize);
                send(ConnectSocket, File_Size, 1025, 0);
                send(ConnectSocket, buf, 65536, 0);
                break;
            }
            case 'v':
            {
                send(ConnectSocket, choice, strlen(choice) + 1, 0);
                recv(ConnectSocket, recvbuf, recvbuflen, 0);
                printf("%s", recvbuf);
                break;
            }
            default:
                send(ConnectSocket, "Error", strlen("Error") + 1, 0);
                recv(ConnectSocket, recvbuf, recvbuflen, 0);
                printf("%s", recvbuf);
                break;
            }
        }
        else if (iResult == 0)
            printf("Connection closing...\n");
        else {
            printf("recv failed with error: %d\n", WSAGetLastError());
            closesocket(ConnectSocket);
            WSACleanup();
            return 1;
        }

    } while (iResult > 0);

    // shutdown the connection since no more data will be sent
    iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    // cleanup
    closesocket(ConnectSocket);
    WSACleanup();

    return 0;
}