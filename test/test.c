#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define MAX_BUFFER_SIZE 999999
#define PORT 5021
int main(int argc, char const* argv[]) {
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    int cnt = 1;
    unsigned char buffer[MAX_BUFFER_SIZE];
    char filename[50];
    int bytes_received;

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET,
                   SO_REUSEADDR | SO_REUSEPORT, &opt,
                   sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

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

        int total_bytes_received = 0;
        while (total_bytes_received < MAX_BUFFER_SIZE) {
            bytes_received = recv(new_socket, buffer + total_bytes_received, MAX_BUFFER_SIZE - total_bytes_received, 0);
            if (bytes_received <= 0) {
                perror("recv");
                close(new_socket);
                continue; // 处理失败时继续下一次循环
            }
            total_bytes_received += bytes_received;
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
