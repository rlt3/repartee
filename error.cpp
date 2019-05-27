#include <string>
#include <utility>
#include <vector>
#include "error.hpp"

#define BUFFSIZE 1024
#define ERRORMAX 5

static int num_errors = 0;
static std::ostream *out = &std::cerr; /* lmaoing @ C++ */

#define PRINT_FMT_STRING() \
    char buff[BUFFSIZE] = {0}; \
    va_list argp; \
    va_start(argp, fmt); \
    vsnprintf(buff, BUFFSIZE, fmt, argp); \
    va_end(argp); \
    *out << std::string(buff);

void
set_error_output (std::ostream &output)
{
    out = &output;
}

void
panic (const char *fmt, ...)
{
    *out << "Panic: ";
    PRINT_FMT_STRING();
    std::exit(1);
}

void
error (const char *fmt, ...)
{
    PRINT_FMT_STRING();
    num_errors++;
    if (num_errors >= ERRORMAX) {
        *out << "Maximum number of errors reached!\n";
        std::exit(1);
    }
}

void
warning (const char *fmt, ...)
{
    PRINT_FMT_STRING();
}
