#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

typedef struct {
    int socket;
} client_t;

void handle_client(int sock) {
    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE * 2]; 
    memset(response, 0, sizeof(response)); 
    snprintf(response, sizeof(response), "Top two CPU-consuming processes:\n");

    FILE *fp = popen("ps -eo pid,comm,%cpu --sort=-%cpu | head -n 3", "r");
    if (fp == NULL) {
        perror("Failed to run command");
        close(sock);
        return;
    }
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        strncat(response, buffer, sizeof(response) - strlen(response) - 1);
    }
    pclose(fp);
    send(sock, response, strlen(response), 0);

    close(sock);
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8002);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port 8002...\n");

    // Server loop: Accept and handle incoming client connections
    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        printf("Client connected\n");

        // Handle the client connection
        handle_client(new_socket);
    }

    // Close the server socket (though this is unreachable in this loop)
    close(server_fd);
    return 0;
}
