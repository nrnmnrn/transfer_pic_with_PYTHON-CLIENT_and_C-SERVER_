#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define PORT 5021
#define MAX_BUFFER_SIZE 99999
int main(int argc, char const* argv[]) {
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    int cnt = 1;
    unsigned char buffer[MAX_BUFFER_SIZE];
    char filename[50];
    int bytes_received;

    // 创建socket文件描述符和其他设置...

    if (bind(server_fd, (struct sockaddr*)&address,
             sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while (1) {
        // 接受新连接并创建新的套接字
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            continue;  // 处理失败时继续下一次循环
        }

        bytes_received = recv(new_socket, buffer, MAX_BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            perror("recv");
            close(new_socket);
            continue;  // 处理失败时继续下一次循环
        }

        sprintf(filename, "pic_%d.bmp", cnt);
        printf("Get the picture!\n");
        FILE* fpw = fopen(filename, "wb");
        if (fpw == NULL) {
            printf("Error opening file\n");
            close(new_socket);
            continue;  // 处理失败时继续下一次循环
        }

        // 将接收到的数据写入文件
        fwrite(buffer, 1, bytes_received, fpw);
        fclose(fpw);

        // 关闭与客户端的连接
        close(new_socket);
        cnt++;
    }
    // closing the listening socket
    shutdown(server_fd, SHUT_RDWR);
    return 0;
}
