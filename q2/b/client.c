#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8000
#define BUFFER_SIZE 1024

// Mutex for synchronized access to shared resources
pthread_mutex_t lock;

int createsocket() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    return sock;
}

void *connect_to_server(void *arg) {
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr);

    int sock = createsocket();
    if (connect(sock, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Connection failed");
        close(sock);
        pthread_exit(NULL);
    }

    char buffer[BUFFER_SIZE] = {0};
    read(sock, buffer, sizeof(buffer));

    // Use mutex lock to ensure safe access to stdout
    pthread_mutex_lock(&lock);
    printf("Response from server: %s\n", buffer);
    pthread_mutex_unlock(&lock);

    close(sock);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <number_of_connections>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int n = atoi(argv[1]);

    // Initialize mutex
    pthread_mutex_init(&lock, NULL);

    // Create an array of threads
    pthread_t threads[n];
    for (int i = 0; i < n; i++) {
        // Create a new thread for each connection
        if (pthread_create(&threads[i], NULL, connect_to_server, NULL) != 0) {
            perror("Thread creation failed");
            exit(EXIT_FAILURE);
        }
    }

    // Wait for all threads to finish
    for (int i = 0; i < n; i++) {
        pthread_join(threads[i], NULL);
    }

    // Destroy the mutex
    pthread_mutex_destroy(&lock);

    return 0;
}
