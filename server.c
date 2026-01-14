/*
 * server.c - TCP服务端
 */
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib") // 如果使用 Visual Studio，这行会自动链接库

#define MAX_PENDING 10      // 允许排队的客户端数量 [cite: 790]
#define RECV_BUFFER_SIZE 4096 // 接收缓冲区大小

int main(int argc, char *argv[]) {
    WSADATA wsaData;
    struct sockaddr_in sin;
    struct sockaddr_in client_addr;
    int client_len;
    SOCKET s, new_s; // Windows下Socket类型为 SOCKET
    char buf[RECV_BUFFER_SIZE];
    int len;
    int server_port;

    // 检查参数
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    server_port = atoi(argv[1]);

    // 0. Winsock 初始化 (Windows特有)
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed.\n");
        exit(1);
    }

    // 1. 构建地址数据结构
    memset((char *)&sin, 0, sizeof(sin)); // 使用 memset 替代 bzero
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(server_port);

    // 2. 创建 Socket
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        fprintf(stderr, "server: socket error code %d\n", WSAGetLastError());
        WSACleanup();
        exit(1);
    }

    // 3. Bind 绑定端口
    if ((bind(s, (struct sockaddr *)&sin, sizeof(sin))) == SOCKET_ERROR) {
        fprintf(stderr, "server: bind error code %d\n", WSAGetLastError());
        closesocket(s);
        WSACleanup();
        exit(1);
    }

    // 4. Listen 监听
    listen(s, MAX_PENDING);
    printf("Server is listening on port %d...\n", server_port);

    // 5. 主循环：等待连接
    while (1) {
        client_len = sizeof(client_addr);
        // Accept 连接
        if ((new_s = accept(s, (struct sockaddr *)&client_addr, &client_len)) == INVALID_SOCKET) {
            fprintf(stderr, "server: accept error code %d\n", WSAGetLastError());
            continue;
        }

        // 6. 接收数据循环 (支持 >20KB 长文本)
        // Windows下 recv 返回值 > 0 表示有数据
        while ((len = recv(new_s, buf, sizeof(buf) - 1, 0)) > 0) {
            buf[len] = '\0'; 
            fputs(buf, stdout);// 打印到标准输出 [cite: 745]
        }
        
        // 7. 关闭当前客户端连接
        closesocket(new_s); // Windows使用 closesocket
    }
    
    closesocket(s);
    WSACleanup(); // 清理 Winsock
    return 0;
}
