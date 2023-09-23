#include "Utils.hpp"
#include "Account.hpp"

bool checkValidLogin(const Account* ValidAccounts, uint64_t size, const char* username, const char* password)
{
    puts("[INFO] Checking login");
    for (uint64_t i = 0; i < size; i++) {
        if (strcmp(username, ValidAccounts[i].name) == 0 &&
            strcmp(password, ValidAccounts[i].password) == 0) {
            return true;
        }
    }
    return false;
}

bool is_same(const char* str, const char* pattern)
{
    for (int i = 0; i < strlen(pattern);++i){
        if(str[i]!=pattern[i])
        {
            return false;
        }
    }
    return true;
}

int* createSharedMemory(int KEY, int initVal)
{
    // Create shared memory
    int shmid = shmget(KEY, sizeof(int), IPC_CREAT | 0666);
    // Attach shared memory
    int* sharedMemory = (int*)shmat(shmid, NULL, 0);
    *sharedMemory = initVal;
    return sharedMemory;
}