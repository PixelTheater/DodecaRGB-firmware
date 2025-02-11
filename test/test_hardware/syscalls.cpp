#include <errno.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/unistd.h>

extern "C" {

int _getpid(void) {
    return 1;
}

int _kill(int pid, int sig) {
    errno = EINVAL;
    return -1;
}

int _open(const char *name, int flags, int mode) {
    return -1;
}

// Add other syscalls if needed
} 