#pragma once
#include <cstdint>
#include <cstring>
#include <string>
#include <cstdio>
#include <sys/shm.h>
#include "Account.hpp"

bool checkValidLogin(const Account* ValidAccounts, uint64_t size, const char* username, const char* password);

bool is_same(const char* str, const char* pattern);

int* createSharedMemory(int KEY, int initVal);