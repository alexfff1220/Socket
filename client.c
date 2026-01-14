/*
 * client.c - TCP客户端
*/
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

#define INITIAL_BUF_SIZE 1024 

int main(int argc, char *argv[]) {
    WSADATA wsaData;
    struct hostent *hp;
    struct sockaddr_in sin;
    char *host;
    int server_port;
    SOCKET s;
    // 动态缓冲区变量，支持 >20KB 
    char *message_buffer;
    int buffer_capacity = INITIAL_BUF_SIZE;
    int buffer_len = 0;
    int ch;
    int newline_count = 0;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <host> <port>\n", argv[0]);
        exit(1);
    }

    host = argv[1];
    server_port = atoi(argv[2]);

    // 0. Winsock 初始化
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed.\n");
        exit(1);
    }

    // 1. 解析主机名
    hp = gethostbyname(host);
    if (!hp) {
        fprintf(stderr, "client: unknown host: %s\n", host);
        WSACleanup();
        exit(1);
    }

    // 2. 构建地址结构
    memset((char *)&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    memcpy((char *)&sin.sin_addr, hp->h_addr, hp->h_length);
    sin.sin_port = htons(server_port);

    // 3. 创建 Socket
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        fprintf(stderr, "client: socket error code %d\n", WSAGetLastError());
        WSACleanup();
        exit(1);
    }

    // 4. 连接 Server
    if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) == SOCKET_ERROR) {
        fprintf(stderr, "client: connect error (check if server is running)\n");
        closesocket(s);
        WSACleanup();
        exit(1);
    }

    // 5. 分配初始内存
    message_buffer = (char *)malloc(buffer_capacity);
    if (!message_buffer) {
        perror("client: malloc error");
        exit(1);
    }

    // 6. 读取输入 (处理双回车和长文本)
    while ((ch = getchar()) != EOF) {
        if (buffer_len >= buffer_capacity - 1) {
            buffer_capacity *= 2;
            message_buffer = (char *)realloc(message_buffer, buffer_capacity);
            if (!message_buffer) {
                perror("client: realloc error");
                exit(1);
            }
        }

        // 双回车检测逻辑 
        if (ch == '\n') {
            newline_count++;
            if (newline_count == 2) {
                // 移除前一个回车，不发送这两个回车
                if (buffer_len > 0 && message_buffer[buffer_len-1] == '\n') {
                    buffer_len--; 
                }
                break; 
            }
        } else if (ch != '\r') { 
             newline_count = 0;
        }

        if (ch != '\r') { 
             message_buffer[buffer_len++] = (char)ch;
        }
    }

    // 7. 发送数据
    int sent = 0;
    int n;
    while (sent < buffer_len) {
        n = send(s, message_buffer + sent, buffer_len - sent, 0);
        if (n == SOCKET_ERROR) {
            fprintf(stderr, "client: send error code %d\n", WSAGetLastError());
            break;
        }
        sent += n;
    }

    // 8. 清理并退出
    free(message_buffer);
    closesocket(s);
    WSACleanup();
    return 0;
}
