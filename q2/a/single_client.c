#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <number_of_times>\n", argv[0]);
        return -1;
    }

    int n = atoi(argv[1]);  // Convert command-line argument to integer
    if (n <= 0) {
        printf("Please provide a positive integer for the number of times to run the client.\n");
        return -1;
    }

    int sock = 0;
    struct sockaddr_in serv_addr;
    const char *hello = "Hello from client";
    char buffer[1024] = {0};

    while (n--) {
        // Create socket
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("Socket creation error");
            return -1;
        }

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(8002);

        // Convert IPv4 address from text to binary form
        if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
            perror("Invalid address/Address not supported");
            return -1;
        }

        // Connect to server
        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            perror("Connection Failed");
            close(sock);
            continue;
        }

        // Send message to server
        send(sock, hello, strlen(hello), 0);
        printf("Hello message sent\n");

        // Read response from server
        read(sock, buffer, 1024);
        printf("Message received: %s\n", buffer);

        // Close socket
        close(sock);
    }

    return 0;
}
