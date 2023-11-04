#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define PORT 5022
#define MAX_BUFFER_SIZE 12000

int main(int argc, char const* argv[]) {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    int cnt = 1;
    unsigned char buffer2[MAX_BUFFER_SIZE];
    char filename[50];
    int bytes_received;
    int image_size = 0;
    int bytes_left = 0;
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    int new_buffer_size = 18192000;  // 這裡以8192位元組（8KB）為例，您可以根據需要調整大小

    // 設定輸出緩衝區的大小
    if (setsockopt(server_fd, SOL_SOCKET, SO_SNDBUF, &new_buffer_size, sizeof(new_buffer_size)) == -1) {
        perror("設定緩衝區大小失敗");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            continue;
        }

        char size_buffer[16];
        bytes_received = recv(new_socket, size_buffer, sizeof(size_buffer) - 1, 0);
        if (bytes_received > 0) {
            size_buffer[bytes_received] = '\0';
            image_size = atoi(size_buffer);
            bytes_left = image_size;
        }
        printf("%d bytes size\n", bytes_left);
        FILE* fpw = NULL;
        sprintf(filename, "./image/pic_%d.png", cnt);
        printf("Receiving the picture...\n");
        fpw = fopen(filename, "wb");
        if (fpw == NULL) {
            printf("Error opening file\n");
            close(new_socket);
            continue;
        }
        bytes_received = 1;
        while (bytes_received) {
            bytes_received = recv(new_socket, buffer2, MAX_BUFFER_SIZE, 0);
            if (bytes_received <= 0) {
                perror("recv");
                break;
            }

            fwrite(buffer2, 1, bytes_received, fpw);
            bytes_left -= bytes_received;
        }
        if (bytes_left != 0) {
            printf("---------------------\n");
            printf("%d bytes left\n", bytes_left);
            printf("---------------------\n");
        } else
            printf("%d  bytes left\n", bytes_left);
        fclose(fpw);
        printf("Image received and saved as 'pic_%d.png'\n\n", cnt);

        // 关闭与客户端的连接
        close(new_socket);
        cnt++;
    }

    // 关闭监听套接字
    shutdown(server_fd, SHUT_RDWR);
    return 0;
}