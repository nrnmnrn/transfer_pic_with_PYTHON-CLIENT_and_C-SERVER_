#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define PORT 5022

int recvall(int sock, char* buffer, int n) {
    int total = 0;
    int bytes_received;
    while (total < n) {  // n代表資料大小
        /* recv(,buffer,size,): 會把<=size大小的資料從緩存區取出來放到buffer，回傳取出的資料大小。
        所以有可能不是完整的size大小，才回寫成底下的方式連續去取 */
        bytes_received = recv(sock, buffer + total, n - total, 0);  // buffer + total代表buffer的位置
        if (bytes_received <= 0) {                                  // recv()回傳0: socket關閉。recv()回傳-1: 有錯誤。若緩衝區為空，recv()會阻塞並等待讀取，而不是回傳0
            return 0;
        }
        printf("bytes received: %d\n", bytes_received);
        total += bytes_received;
    }
    // 照理來講，total會跟n相同
    return total;
}

int recv_msg(int sock, char** msg) {
    char raw_msglen[4];  // 傳進來的開頭代表資料大小，為4個bytes。先取出此資訊。
    if (recvall(sock, raw_msglen, 4) != 4) {
        return 0;
    }
    int msglen;                      // 資料大小
    memcpy(&msglen, raw_msglen, 4);  // 先用char接收，因為recvall()函數的預設是char，再copy資訊到int上
    msglen = ntohl(msglen);          // network to host。big-endian轉成little-endian
    *msg = (char*)malloc(msglen);
    if (recvall(sock, *msg, msglen) == 0) {  // 緩衝區沒東西的話
        return 0;
    };

    printf("Image size: %d\n", msglen);
    return msglen;
}

int main(int argc, char const* argv[]) {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int cnt = 1;   // 計算收到幾張圖片
    char* buffer;  // 接收圖片資料用的buffer
    char filename[50];
    int bytes_received;
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);  // host to network。代表從little-endian轉成big-endian

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while (1) {
        // 沒有終止碼的話，server就不會停止接收client
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        while (1) {
            bytes_received = recv_msg(new_socket, &buffer);
            if (*buffer == '0') {
                goto close;  // 收到終止碼，停止server運作
            }
            if (bytes_received == 0) {
                break;  // 暫存區的資料已經全部接收完畢，跳出迴圈
            }

            FILE* fpw = NULL;
            sprintf(filename, "./image/pic_%d.png", cnt);
            printf("Receiving the picture...\n");
            fpw = fopen(filename, "wb");  // 以二進制的方式儲存圖片
            if (fpw == NULL) {
                printf("Error opening file\n");
                close(new_socket);
                continue;
            }
            fwrite(buffer, 1, bytes_received, fpw);
            fclose(fpw);
            printf("Image received and saved as 'pic_%d.png'\n\n", cnt);
            cnt++;
        }
    }
close:
    shutdown(server_fd, SHUT_RDWR);  // server端停止接收
    return 0;
}