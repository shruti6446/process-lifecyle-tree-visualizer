#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static const char *log_path = "process_events.csv";
static int log_fd = -1;

static void log_event(const char *name, pid_t child) {
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) == -1) { perror("clock_gettime"); _exit(2); }
    char line[256];
    int len = snprintf(line, sizeof line, "%lld.%09ld,%ld,%ld,%s,%ld\n",
        (long long)ts.tv_sec, ts.tv_nsec, (long)getpid(),
        (long)getppid(), name, (long)child);
    if (len < 0 || (size_t)len >= sizeof line) _exit(2);
    size_t offset = 0;
    while (offset < (size_t)len) {
        ssize_t n = write(log_fd, line + offset, (size_t)len - offset);
        if (n < 0 && errno == EINTR) continue;
        if (n <= 0) { perror("write event"); _exit(2); }
        offset += (size_t)n;
    }
}

static void pause_ms(int ms) {
    struct timespec req = {ms / 1000, (long)(ms % 1000) * 1000000L};
    while (nanosleep(&req, &req) == -1) {
        if (errno != EINTR) { perror("nanosleep"); _exit(2); }
    }
}

static int reap(pid_t pid) {
    int status;
    pid_t result;
    do { result = waitpid(pid, &status, 0); } while (result == -1 && errno == EINTR);
    if (result == -1) { perror("waitpid"); return -1; }
    log_event("REAP", pid);
    return WIFEXITED(status) && WEXITSTATUS(status) == 0 ? 0 : -1;
}

static void work(int duration_ms, int grandchildren) {
    pid_t nested[4];
    int made = 0, failed = 0;
    log_event("START", 0);
    for (int i = 0; i < grandchildren; ++i) {
        pid_t pid = fork();
        if (pid < 0) { perror("grandchild fork"); failed = 1; break; }
        if (pid == 0) {
            log_event("START", 0);
            log_event("SLEEP", 0);
            pause_ms(180 + i * 60);
            log_event("WAKE", 0);
            log_event("EXIT", 0);
            _exit(0);
        }
        nested[made++] = pid;
        log_event("FORK", pid);
    }
    log_event("SLEEP", 0);
    pause_ms(duration_ms);
    log_event("WAKE", 0);
    for (int i = 0; i < made; ++i) if (reap(nested[i]) != 0) failed = 1;
    log_event("EXIT", 0);
    _exit(failed ? 1 : 0);
}

int main(int argc, char **argv) {
    if (argc > 3) { fprintf(stderr, "Usage: %s [scenario 1-7] [output.csv]\n", argv[0]); return 1; }
    char *end = NULL;
    errno = 0;
    long mode = argc > 1 ? strtol(argv[1], &end, 10) : 1;
    if (errno || mode < 1 || mode > 7 || (argc > 1 && (!end || *end))) {
        fprintf(stderr, "Scenario must be an integer from 1 to 7\n"); return 1;
    }
    if (argc == 3) log_path = argv[2];
    log_fd = open(log_path, O_WRONLY | O_CREAT | O_TRUNC | O_APPEND, 0644);
    if (log_fd == -1) { perror("open log"); return 1; }
    const char *header = "timestamp,pid,ppid,event,child_pid\n";
    if (write(log_fd, header, strlen(header)) != (ssize_t)strlen(header)) { perror("write header"); close(log_fd); return 1; }
    static const int counts[7] = {3,3,4,6,4,3,5};
    static const int durations[7][6] = {
        {200,350,500}, {120,120,120}, {400,200,350,250},
        {150,220,290,360,430,500}, {100,700,250,550},
        {500,400,300}, {120,180,240,300,360}
    };
    pid_t children[6];
    int made = 0, failed = 0, count = counts[mode - 1];
    log_event("START", 0);
    printf("Scenario %ld: %d direct children\n", mode, count);
    fflush(stdout);
    for (int i = 0; i < count; ++i) {
        pid_t pid = fork();
        if (pid == -1) { perror("fork"); failed = 1; break; }
        if (pid == 0) {
            int grandkids = (mode == 3 && i == 0) ? 2 : ((mode == 6 && i < 2) ? 2 : 0);
            work(durations[mode - 1][i], grandkids);
        }
        children[made++] = pid;
        log_event("FORK", pid);
        if (mode == 2 || mode == 7) pause_ms(130);
    }
    for (int i = 0; i < made; ++i) if (reap(children[i]) != 0) failed = 1;
    log_event("EXIT", 0);
    if (close(log_fd) == -1) { perror("close log"); failed = 1; }
    printf("Scenario %ld %s: %s\n", mode, failed ? "failed" : "complete", log_path);
    return failed ? 1 : 0;
}
