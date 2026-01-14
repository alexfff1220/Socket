/*
 * server_duplex.c - 双工通信服务端
 */
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h> // 用于多线程

#pragma comment(lib, "Ws2_32.lib")

#define BUFFER_SIZE 4096

// 接收线程函数：专门负责从客户端收消息并打印
DWORD WINAPI ReceiveThread(LPVOID lpParam) {
    SOCKET clientSocket = (SOCKET)lpParam;
    char buf[BUFFER_SIZE];
    int len;

    while ((len = recv(clientSocket, buf, sizeof(buf) - 1, 0)) > 0) {
        buf[len] = '\0';
        printf("\n[Client]: %sServer> ", buf); // 打印收到的消息，并恢复提示符
    }
    return 0;
}

int main(int argc, char *argv[]) {
    WSADATA wsaData;
    struct sockaddr_in sin, client_addr;
    int client_len;
    SOCKET s, new_s;
    char sendBuf[BUFFER_SIZE];
    int port = 8888; // 默认端口

    if (argc == 2) port = atoi(argv[1]);

    WSAStartup(MAKEWORD(2, 2), &wsaData);

    s = socket(PF_INET, SOCK_STREAM, 0);
    
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(port);

    bind(s, (struct sockaddr *)&sin, sizeof(sin));
    listen(s, 1);

    printf("=== Duplex Server (Chat Mode) ===\n");
    printf("Listening on port %d...\n", port);

    client_len = sizeof(client_addr);
    new_s = accept(s, (struct sockaddr *)&client_addr, &client_len);
    printf("Client connected! You can type now.\nServer> ");

    // 创建接收线程
    CreateThread(NULL, 0, ReceiveThread, (LPVOID)new_s, 0, NULL);

    // 主线程：负责输入并发送
    while (fgets(sendBuf, sizeof(sendBuf), stdin)) {
        send(new_s, sendBuf, strlen(sendBuf), 0);
        printf("Server> ");
    }

    closesocket(new_s);
    closesocket(s);
    WSACleanup();
    return 0;
}
