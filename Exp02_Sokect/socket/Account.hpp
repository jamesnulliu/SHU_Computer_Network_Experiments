#pragma once
#include <cstdio>

struct Account
{
    inline void to_cstr(char* result) const noexcept
    {
        sprintf(result, "%s %s", name, password);
    }

    char name[1024]{0};
    char password[1024]{0};
};