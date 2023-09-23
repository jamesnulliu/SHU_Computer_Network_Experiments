#include "Server.hpp"
#include "Account.hpp"
#include "Utils.hpp"

int KEY = 0;
Account ValidAccounts[10] = {{"Tom", "123"}, {"Jack", "456"}};

void initServer(int& server_fd, sockaddr_in& address, int& opt, int*& serverStatus)
{
    // Creating socket file descriptor
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    // Forcefully attaching socket to the port 8080
    bind(server_fd, toSockAddr(address), sizeof(address));
    listen(server_fd, BACKLOG);
    serverStatus = createSharedMemory(SERVER_SHM_KEY, SERVER_RUNNING);
}

void handleConnection(int client_fd, int*& serverStatus)
{
    char buffer[1024] = {0};
    // -------------------------------------------------------------------------------------------
    int* clientConnectionStatus = createSharedMemory(KEY, CLIENT_DISCONNECTED);
    int valread = read(client_fd, buffer, 1024);
    Account account = getAccount(buffer);
    memset(buffer, 0, 1024);
    // Check if it is a valid login
    bool loginState = checkValidLogin(ValidAccounts, ACCOUNT_NUM, account.name, account.password);
    // If it is a valid login, send {LoginSuccessfulM} to client, end connection
    if (loginState == false) {
        // Send {LoginFailedM} to client
        send(client_fd, LoginFailedM, strlen(LoginFailedM), 0);
        printf("[INFO] User login failed\n");
        // Close connection
        return;
    }
    // Valid login, send {LoginSuccessfulM} to client
    printf("[INFO] Client login successful\n");
    *clientConnectionStatus = CLIENT_CONNECTED;
    // Send {LoginSuccessfulM} to client
    size_t length = send(client_fd, LoginSuccessfulM, strlen(LoginSuccessfulM), 0);
    // -------------------------------------------------------------------------------------------
    // Create child process to handle message from client
    int pid1 = fork();
    if (pid1 == 0) {
        while (true) {
            // Check if user pressed 'q', which means to stop connecting to current client
            if (*clientConnectionStatus == 1) {
                return;
            }
            // Read message from client, and store into {buffer}
            valread = read(client_fd, buffer, 1024);
            // Check is client disconnected
            if (valread == 0) {
                *clientConnectionStatus = CLIENT_DISCONNECTED;
                return;
            }

            printf("client >>> %s", buffer);

            // Set some friendly response
            if (is_same(buffer, "Who are you?")) {
                size_t length = send(client_fd, "I'm your father.\n", strlen("I'm your father.\n"), 0);
                if (length == -1) {
                    *clientConnectionStatus = CLIENT_DISCONNECTED;
                    return;
                }
            } else if (is_same(buffer, "Test")) {
                for (int i = 0; i < 10; ++i) {
                    char testMessage[10]{0};
                    strcat(testMessage, "Test-");
                    strcat(testMessage, std::to_string(i).c_str());
                    strcat(testMessage, "\n");
                    size_t length = send(client_fd, testMessage, strlen(testMessage), 0);
                    if (length == -1) {
                        *clientConnectionStatus = CLIENT_DISCONNECTED;
                        return;
                    }
                    // Delay for 2 seconds
                    sleep(2);
                }
            }

            memset(buffer, 0, 1024);
        }
    }
    // Following parent process will handle user input
    else {
        int pid2 = fork();
        if (pid2 == 0) {
            while (true) {
                char ch = getchar();
                if (ch == 'q') {
                    *serverStatus = SERVER_TERMINATED;
                    return;
                }
            }
        } else {
            while (true) {
                if(*serverStatus == SERVER_TERMINATED)
                {
                    puts("[TEST] Server terminated");
                    kill(pid1, SIGKILL);
                    kill(pid2, SIGKILL);
                    return;
                }
                if (*clientConnectionStatus == 2) {
                    printf("[INFO] Client %d disconnected\n", client_fd);
                    kill(pid1, SIGKILL);
                    kill(pid2, SIGKILL);
                    return;
                }
            }
        }
    }
}


int main(int argc, char const* argv[])
{
    int server_fd,  // listening socket
        new_socket, // connected socket
        valread;    // number of bytes read

    sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    int* serverStatus = nullptr;

    initServer(server_fd, address, opt, serverStatus);

    int pid = 0;

    while (true) {
        // Accept connection from client
        new_socket = accept(server_fd, toSockAddr(address), toSockLen(addrlen));
        // Log new connection
        printf("[INFO] New connection from %s:%d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));

        pid = fork();
        ++KEY;

        if (pid == 0) { // Child process
            // Close listening socket
            close(server_fd);
            handleConnection(new_socket, serverStatus);
            if(*serverStatus == SERVER_TERMINATED){
                printf("[INFO] Server terminated\n");
                kill(pid, SIGKILL);
                return 0;
            }
            exit(0);
        } else { // Parent process
            // Close connected socket
            close(new_socket);
        }
    }

    return 0;
}