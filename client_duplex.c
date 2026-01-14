/*
 * client_duplex.c - Windows版 双工通信客户端
 */
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#pragma comment(lib, "Ws2_32.lib")

#define BUFFER_SIZE 4096

// 接收线程函数：专门负责从服务端收消息并打印
DWORD WINAPI ReceiveThread(LPVOID lpParam) {
    SOCKET serverSocket = (SOCKET)lpParam;
    char buf[BUFFER_SIZE];
    int len;

    while ((len = recv(serverSocket, buf, sizeof(buf) - 1, 0)) > 0) {
        buf[len] = '\0';
        printf("\n[Server]: %sClient> ", buf);
    }
    return 0;
}

int main(int argc, char *argv[]) {
    WSADATA wsaData;
    struct sockaddr_in sin;
    SOCKET s;
    char sendBuf[BUFFER_SIZE];
    char *host = "127.0.0.1";
    int port = 8888;

    if (argc == 3) {
        host = argv[1];
        port = atoi(argv[2]);
    }

    WSAStartup(MAKEWORD(2, 2), &wsaData);

    s = socket(PF_INET, SOCK_STREAM, 0);
    
    struct hostent *hp = gethostbyname(host);
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    memcpy(&sin.sin_addr, hp->h_addr, hp->h_length);
    sin.sin_port = htons(port);

    if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
        printf("Connect failed.\n");
        return 1;
    }

    printf("=== Duplex Client (Chat Mode) ===\n");
    printf("Connected to server! You can type now.\nClient> ");

    // 创建接收线程
    CreateThread(NULL, 0, ReceiveThread, (LPVOID)s, 0, NULL);

    // 主线程：负责输入并发送
    while (fgets(sendBuf, sizeof(sendBuf), stdin)) {
        send(s, sendBuf, strlen(sendBuf), 0);
        printf("Client> ");
    }

    closesocket(s);
    WSACleanup();
    return 0;
}
