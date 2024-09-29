#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8000
#define BUFFER_SIZE 1024
int createsocket() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    return sock;
}
void connect_to_server(int n) {
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr);

    for (int i = 0; i < n; i++) {
        int sock = createsocket();
        if (connect(sock, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
            perror("Connection failed");
            close(sock);
            continue;
        }

        char buffer[BUFFER_SIZE] = {0};
        read(sock, buffer, sizeof(buffer));
        printf("Response from server: %s\n", buffer);

        close(sock);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <number_of_connections>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int n = atoi(argv[1]);
    connect_to_server(n);

    return 0;
}
