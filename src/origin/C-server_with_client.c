#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define PORT 5022
#define MAX_BUFFER_SIZE 12000

int main(int argc, char const* argv[]) {
    /*有client端的原因是為了在收到圖片後可以回傳給樹莓派訊號表示已收到*/
    // client端的部分------------------------------------------------------
    int status, valread, client_fd;
    struct sockaddr_in serv_addr;
    char* hello = "light5";
    char buffer[1024] = {0};

    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary
    // form
    if (inet_pton(AF_INET, "192.168.1.102", &serv_addr.sin_addr) <= 0) {
        printf(
            "\nInvalid address/ Address not supported \n");
        return -1;
    }

    if ((status = connect(client_fd, (struct sockaddr*)&serv_addr,
                          sizeof(serv_addr))) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }
    send(client_fd, hello, strlen(hello), 0);
    printf("Hello message sent\n");
    valread = read(client_fd, buffer, 1024);
    printf("%s\n", buffer);

    // closing the connected socket
    close(client_fd);

    // server端的部分------------------------------------------------------
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

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
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

        FILE* fpw = NULL;
        sprintf(filename, "pic_%d.jpg", cnt);
        printf("Get the picture!\n");
        fpw = fopen(filename, "wb");
        if (fpw == NULL) {
            printf("Error opening file\n");
            close(new_socket);
            continue;
        }

        while (bytes_left > 0) {
            bytes_received = recv(new_socket, buffer2, MAX_BUFFER_SIZE, 0);
            if (bytes_received <= 0) {
                perror("recv");
                break;
            }

            fwrite(buffer2, 1, bytes_received, fpw);
            bytes_left -= bytes_received;
        }
        fclose(fpw);
        close(new_socket);
        cnt++;
    }

    shutdown(server_fd, SHUT_RDWR);
    return 0;
}
