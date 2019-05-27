#include <iostream>
#include <string>
#include <vector>
#include "error.hpp"

#define BUFFSIZE 1024
#define ERRORMAX 5

#define PRINT_FMT_STRING() \
    char buff[BUFFSIZE] = {0}; \
    va_list argp; \
    va_start(argp, fmt); \
    vsnprintf(buff, BUFFSIZE, fmt, argp); \
    va_end(argp); \
    std::cerr << std::string(buff);

static int num_errors = 0;

void
panic (const char *fmt, ...)
{
    std::cerr << "Panic: ";
    PRINT_FMT_STRING();
    std::exit(1);
}

void
error (const char *fmt, ...)
{
    PRINT_FMT_STRING();
    num_errors++;
    if (num_errors >= ERRORMAX) {
        std::cerr << "Maximum number of errors reached!\n";
        std::exit(1);
    }
}

void
warning (const char *fmt, ...)
{
    PRINT_FMT_STRING();
}
