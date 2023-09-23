// client.cpp
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

#define PORT 8080
#define SHM_KEY 1234

int main(int argc, char const* argv[])
{
    if (argc != 3) {
        printf("Usage: %s <client_name> <client_password>\n", argv[0]);
        return -1;
    }

    int status, valread, client_fd;
    struct sockaddr_in serv_addr;

    char hello[1024] = {0};
    strcat(hello, "Hello from client ");
    strcat(hello, argv[1]);

    char loginRequest[1024] = {0};
    strcat(loginRequest, argv[1]);
    strcat(loginRequest, " ");
    strcat(loginRequest, argv[2]);

    char buffer[1024] = {0};
    client_fd = socket(AF_INET, SOCK_STREAM, 0);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    inet_pton(AF_INET, "127.0.1.1", &serv_addr.sin_addr);

    status = connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    // Send username and password
    send(client_fd, loginRequest, strlen(loginRequest), 0);
    printf("[INFO] Login request sent\n");

    // Receive response
    valread = read(client_fd, buffer, 1024);

    if (strcmp(buffer, "Login successful") != 0) {
        puts(buffer);
        return -1;
    }

    memset(buffer, 0, 1024);

    printf("[INFO] Login successful\n");

// Create shared memory
int shmid = shmget(SHM_KEY, sizeof(int), IPC_CREAT | 0666);

// Attach shared memory
int* inputState = (int*)shmat(shmid, NULL, 0);

    // Split two process to recieve and send messages
    // Child process:
    if (fork() == 0) {
        while (true) {
            // Check if pressed 'i'
            char c = getchar();
            // If pressed 'i', enter input mode, set {inputState} to 1
            if (c == 'i') {
                // Clear stdin
                while (getchar() != '\n')
                    ;
                *inputState = 1;
                // Input a line of message
                printf("[Input] <<< ");
                char message[1024] = {0};
                fgets(message, 1024, stdin);
                // Send message to server
                send(client_fd, message, strlen(message), 0);
                // Set {inputState} to 0
                *inputState = 0;
            }
        }
    }
    // Parent process:
    else {
valread = 0;
char realBuffer[1024] = {0};
while (true) {
    if (strlen(realBuffer) != 0 && *inputState == 0) {
        printf("server >>> %s", realBuffer);
        memset(realBuffer, 0, 1024);
        valread = 0;
    }
    // Read message from server
    valread = read(client_fd, buffer, 1024);
    if (valread == 0) {
        printf("[INFO] Server disconnected\n");
        return 0;
    }
    strcat(realBuffer, buffer);
    memset(buffer, 0, 1024);
}
    }

    close(client_fd);
    return 0;
}
