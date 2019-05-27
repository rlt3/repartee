#include <stdarg.h>

/* 
 * Print an error message and immediately exit the program.
 */
void panic (const char *fmt, ...);

/* 
 * Add an error message to be printed out. Collects a number of errors before
 * exiting the program.
 */
void error (const char *fmt, ...);

/* 
 * Add a warning to be printed out. No amount of warnings will exit the program.
 */
void warning (const char *fmt, ...);
