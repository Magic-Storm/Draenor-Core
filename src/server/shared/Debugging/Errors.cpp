#include "Errors.h"

#include <thread>
#include <chrono>
#include <cstdlib>
#include <cstdarg>

namespace Trinity {

void Assert(char const *file, int line, char const *function, char const *message)
{
    fprintf(stderr, "\n%s:%i in %s ASSERTION FAILED:\n  %s\n",
            file, line, function, message);
    *((volatile int*)NULL) = 0;
    exit(1);
}

void Assert(char const *file, int line, char const *function, char const *message, char const *format, ...)
{
    va_list args;
    va_start(args, format);

    fprintf(stderr, "\n%s:%i in %s ASSERTION FAILED:\n  %s ", file, line, function, message);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    fflush(stderr);

    va_end(args);
    *((volatile int*)NULL) = 0;
    exit(1);
}

void Fatal(char const *file, int line, char const *function, char const *message)
{
    fprintf(stderr, "\n%s:%i in %s FATAL ERROR:\n  %s\n",
                   file, line, function, message);
    std::this_thread::sleep_for(std::chrono::seconds(10));
    *((volatile int*)NULL) = 0;
    exit(1);
}

void Error(char const *file, int line, char const *function, char const *message)
{
    fprintf(stderr, "\n%s:%i in %s ERROR:\n  %s\n",
                   file, line, function, message);
    *((volatile int*)NULL) = 0;
    exit(1);
}

void Warning(char const *file, int line, char const *function, char const *message)
{
    fprintf(stderr, "\n%s:%i in %s WARNING:\n  %s\n",
                   file, line, function, message);
}

void Abort(char const *file, int line, char const *function)
{
    fprintf(stderr, "\n%s:%i in %s ABORTED.\n",
                   file, line, function);
    *((volatile int*)NULL) = 0;
    exit(1);
}

}
