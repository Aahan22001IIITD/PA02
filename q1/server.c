#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/socket.h>
#include <dirent.h>
#include <ctype.h>

#define PORT 8000
#define BUFFER_SIZE 1024

typedef struct  {
    int socket;
    struct sockaddr_in address;
}client_t ;

int createsocket() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    return sock;
}

void bindserversocket(int sfd, struct sockaddr_in *address) {
    if (bind(sfd, (struct sockaddr *)address, sizeof(*address)) < 0) {
        perror("Binding failed");
        exit(EXIT_FAILURE);
    }
}

void *handle_client(void *arg) {
    client_t *cli = (client_t *)arg;
    int sock = cli->socket;
    free(cli);

    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE * 2]; 
    memset(response, 0, sizeof(response)); 
    snprintf(response, sizeof(response), "Top two CPU-consuming processes:\n");

    FILE *fp = popen("ps -eo pid,comm,%cpu --sort=-%cpu | head -n 3", "r");
    if (fp == NULL) {
        perror("Failed to run command");
        close(sock);
        return NULL;
    }

    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        strncat(response, buffer, sizeof(response) - strlen(response) - 1);
    }
    pclose(fp);
    send(sock, response, strlen(response), 0);

    close(sock);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <number_of_connections>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int n = atoi(argv[1]);
    int sfd = createsocket();
    struct sockaddr_in address;
    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    // forefully attaching the port to the server once its closed.
    // no bind failed 
    int opt  = 1;
    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
    perror("setsockopt");
    exit(EXIT_FAILURE);
}
    bindserversocket(sfd, &address);
    if (listen(sfd, n) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    printf("Server listening on port %d\n", PORT);

    while (1) {
        client_t *cli = malloc(sizeof(client_t));
        socklen_t addr_len = sizeof(cli->address);
        cli->socket = accept(sfd, (struct sockaddr *)&cli->address, &addr_len);
        if (cli->socket < 0) {
            perror("Accept failed");
            free(cli);
            continue;
        }

        pthread_t thread;
        if (pthread_create(&thread, NULL, handle_client, cli) != 0) {
            perror("Thread creation failed");
            close(cli->socket);
            free(cli);
        }
        pthread_detach(thread); 
    }

    close(sfd);
    return 0;
}
