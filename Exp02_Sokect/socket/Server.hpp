#pragma once

#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

// Function {inet_ntoa} is declared in following header
#include <arpa/inet.h>

#include "Account.hpp"

constexpr int PORT = 8080;
constexpr int BACKLOG = 10;
constexpr int SERVER_SHM_KEY = 54321;
constexpr size_t ACCOUNT_NUM = 10;
constexpr const char* LoginSuccessfulM = "Login successful";
constexpr const char* LoginFailedM = "Login failed";

constexpr int SERVER_RUNNING = 0;
constexpr int SERVER_TERMINATED = 1;

constexpr int CLIENT_CONNECTED = 0;
constexpr int CLIENT_DISCONNECTED = 1;


inline auto toSockAddr(sockaddr_in& address) -> sockaddr*
{
    return reinterpret_cast<sockaddr*>(&address);
}

inline auto toSockLen(int& addrlen) -> socklen_t*
{
    return reinterpret_cast<socklen_t*>(&addrlen);
}

inline Account getAccount(const char* buffer)
{
    Account account;
    int i = 0, j = 0;
    while (buffer[i] != ' ') {
        account.name[i] = buffer[i];
        i++;
    }
    i++;
    while (buffer[i] != '\0') {
        account.password[j] = buffer[i];
        i++;
        j++;
    }
    return account;
}
