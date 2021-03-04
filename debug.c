#include "debug.h"

#include <inttypes.h>
#include <stdio.h>
#include <time.h>

//log level
static int g_log_level = LL_INFO;

//start of exe tickcount
static uint32_t g_start_tick = 0;

//debug file
static FILE *debug_file;

//debug flags
// use
#ifdef _WIN32
static int g_use_ods = 0;
#endif

static int g_use_stderr = 1;
static int g_show_pid = 0;
static int g_show_tid = 0;
static int g_show_time = 0;
static int g_show_ticks = 0;
static int g_use_file = 0;

static volatile int32_t g_last_time_update = 0xFFFFFFFF;
static char g_time[32];
static size_t g_time_size;

static volatile uint64_t g_counter = 0x0;
static volatile uint64_t g_loop_time = 0x0;

#ifdef _WIN32
//#include <windef.h>
#include <windows.h>
typedef DWORD ticks_t;
typedef DWORD thread_id_t;
typedef DWORD pid_t;

ticks_t getTick() {
    return GetTickCount();
}

pid_t getpid() {
    return GetCurrentProcessId();
}

pid_t gettid() {
    return GetCurrentThreadId();
}

#else

ticks_t getTick() {
    struct timespec ts;
    unsigned theTick = 0U;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    theTick = ts.tv_nsec / 1000000;
    theTick += ts.tv_sec * 1000;
    return theTick;
}

#endif

void do_update_time(DWORD ticks, int counter) {
    int r;
    time_t dest;
    struct tm tmdata;

    //char tmp[128];

    /*
    if(g_use_ods || g_use_stderr){
        sprintf(tmp, __FUNCTION__ "[%.*s][%8d] [%8d] [%8d]", g_time_size, g_time, GetCurrentThreadId(), ticks, counter);
        if(g_use_ods){
           OutputDebugStringA(tmp);
        }
        if(g_use_stderr){
            fprintf(stderr, __FUNCTION__ "[%.*s][%8d] [%8d] [%8d]\n", g_time_size, g_time, GetCurrentThreadId(), ticks, counter);
        }
    }
    */

    time(&dest);

    r = gmtime_s(&tmdata, &dest);
    if (r != 0) {
        //OutputDebugString(__FUNCTIONW__ L" gmtime_s error");
        goto error;
    }

    r = strftime(g_time, sizeof(g_time), "%Y-%m-%dT%H:%M:%S", &tmdata);
    if (r <= 0) {
        if (g_use_stderr) {
            fprintf(stderr, "%s strftime error %d %s\n ", __func__, errno, strerror(errno));
        }
        //OutputDebugString(__FUNCTIONW__ L" strftime error");
        goto error;
    }
    g_time_size = r;
    return;
error:
    memset(g_time, '-', sizeof(g_time));
    g_time_size = sizeof(g_time) - 1;
    g_time[g_time_size] = '\0';
}

int debug_update_time_ticks(DWORD ticks) {
    DWORD current, seconds;
    int counter = 0;

    seconds = ticks / 100;  //tenths of seconds;

    do {
        //read current value;
        current = InterlockedCompareExchange(&g_last_time_update, 0, 0);
        //current = g_last_time_update;
        counter++;
        if (seconds == current) {
            return 0;  //do nothing
        }

    } while (InterlockedCompareExchange(&g_last_time_update, seconds, current) != current);

    do_update_time(ticks, counter);
    return 1;
}

int debug_update_time() {
    return debug_update_time_ticks(GetTickCount() - g_start_tick);
}

int debug_set_counters(uint64_t counter, uint64_t time) {
    if (g_counter != counter && g_log_level < LL_DEBUG) {
        if (g_use_stderr) {
            fprintf(stderr, "--------------------------------------------------------------------------------------------------------------\n");
        }
        if (g_use_file && debug_file) {
            fprintf(debug_file, "--------------------------------------------------------------------------------------------------------------\n");
        }
    }
    g_counter = counter;
    g_loop_time = time;

    return 0;
}

//void debug_init_file();

void debug_set_level(int level, int log_options) {
    int use_stderr = (log_options & WMQ_LOG_OPTION_USE_STDERR) != 0;
    g_log_level = level;
    g_use_ods = (log_options & WMQ_LOG_OPTION_USE_ODS) != 0;

    g_show_pid = (log_options & WMQ_LOG_OPTION_SHOW_PID) != 0;
    g_show_tid = (log_options & WMQ_LOG_OPTION_SHOW_TID) != 0;

    g_show_time = (log_options & WMQ_LOG_OPTION_SHOW_TIME) != 0;
    g_show_ticks = (log_options & WMQ_LOG_OPTION_SHOW_TICKS) != 0;

    g_use_file = (log_options & WMQ_LOG_OPTION_USE_FILE) != 0;

    memset(g_time, '-', sizeof(g_time));
    g_time_size = sizeof(g_time) - 1;
    g_time[g_time_size] = '\0';

    // if (g_use_file) {
    //     debug_init_file();
    // }

    g_start_tick = GetTickCount();

    g_use_stderr = 0;

    debug_print(LL_INFO, "debug_set_level: %8d %d ODS:%d stderr:%d %s", getpid(), level, g_use_ods, use_stderr, GetCommandLineA());
    g_use_stderr = use_stderr;

    if (!(g_use_ods || g_use_stderr || g_use_file)) {
        g_log_level = LL_NO_LOG;
    }
}

char *str_level(int level) {
    if (level < LL_TRACE) {
        return "VERBOS";
    }
    if (level < LL_DEBUG) {
        return "TRACE ";
    }
    if (level < LL_DETAIL) {
        return "DEBUG ";
    }
    if (level < LL_INFO) {
        return "DETAIL";
    }
    if (level == LL_INFO) {
        return "INFO  ";
    }
    if (level <= LL_WARN) {
        return "WARN  ";
    }
    if (level <= LL_ERROR) {
        return "ERROR ";
    }

    return "FATAL ";
}

void print_to_file(FILE *file, DWORD ticks, DWORD pid, DWORD tid, char *level_s, char *buffer) {
    int c;

    if (g_show_time) {
        fprintf(file, "[%8" PRId64 "][%8" PRId64 "]", g_counter, g_loop_time);
    }

    c = ((g_show_time) ? 8 : 0) + (g_show_ticks ? 4 : 0) + (g_show_pid ? 2 : 0) + (g_show_tid ? 1 : 0);
    switch (c) {
        case 0:
            fprintf(file, "[%s] %s\n", level_s, buffer);
            break;
        case 1:
            fprintf(file, "[%8d][%s] %s\n", tid, level_s, buffer);
            break;
        case 2:
            fprintf(file, "[%8d][%s] %s\n", pid, level_s, buffer);
            break;
        case 3:
            fprintf(file, "[%8d][%8d][%s] %s\n", pid, tid, level_s, buffer);
            break;
        case 4:
            fprintf(file, "[%8d][%s] %s\n", ticks, level_s, buffer);
            break;
        case 5:
            fprintf(file, "[%8d][%8d][%s] %s\n", ticks, tid, level_s, buffer);
            break;
        case 6:
            fprintf(file, "[%8d][%8d][%s] %s\n", ticks, pid, level_s, buffer);
            break;
        case 7:
            fprintf(file, "[%8d][%8d][%8d][%s] %s\n", ticks, pid, tid, level_s, buffer);
            break;
        case 8:
            fprintf(file, "[%.*s][%s] %s\n", g_time_size, g_time, level_s, buffer);
            break;
        case 9:
            fprintf(file, "[%.*s][%8d][%s] %s\n", g_time_size, g_time, tid, level_s, buffer);
            break;
        case 10:
            fprintf(file, "[%.*s][%8d][%s] %s\n", g_time_size, g_time, pid, level_s, buffer);
            break;
        case 11:
            fprintf(file, "[%.*s][%8d][%8d][%s] %s\n", g_time_size, g_time, pid, tid, level_s, buffer);
            break;
        case 12:
            fprintf(file, "[%.*s][%8d][%s] %s\n", g_time_size, g_time, ticks, level_s, buffer);
            break;
        case 13:
            fprintf(file, "[%.*s][%8d][%8d][%s] %s\n", g_time_size, g_time, ticks, tid, level_s, buffer);
            break;
        case 14:
            fprintf(file, "[%.*s][%8d][%8d][%s] %s\n", g_time_size, g_time, ticks, pid, level_s, buffer);
            break;
        case 15:
            fprintf(file, "[%.*s][%8d][%8d][%8d][%s] %s\n", g_time_size, g_time, ticks, pid, tid, level_s, buffer);
            break;
    }
}

void debug_print(int level, const char *fmt, ...) {
    int n, size = 100;
    va_list ap;
    char *buffer = NULL;
    //char* stderr_fmt;
    char *level_s = str_level(level);

    if (level < g_log_level) {
        return;
    }

    while (1) {
        char *newbuffer = (char *)realloc(buffer, size);
        if (newbuffer == NULL) {
            free(buffer);
            return;
        }
        buffer = newbuffer;
        va_start(ap, fmt);
        n = vsnprintf_s(buffer, size, _TRUNCATE, fmt, ap);
        //n = _vsn
        va_end(ap);
        if (n > -1 && n < size) {
            break;
        }
        if (n > -1)
            size = n + 1;
        else
            size *= 2;
    }
#ifdef _WIN32
    if (g_use_ods) {
        char *new_buffer = (char *)malloc(size + 20);  //[12345678][DETAIL] .... - 10+8+1+size+zero
        thread_id_t tid = gettid();
        if (new_buffer == NULL) {
            debug_print_no_memory(buffer);
            return;
        }
        memmove(new_buffer, buffer, size);
        sprintf(new_buffer, "[%8d][%s] %s", tid, level_s, buffer);
        OutputDebugStringA(new_buffer);
        free(new_buffer);
    }
#endif

    if (g_use_stderr | g_use_file) {
        ticks_t ticks = GetTickCount() - g_start_tick;
        pid_t pid = g_show_pid ? getpid() : 0;
        pid_t tid = g_show_tid ? gettid() : 0;
        if (g_show_time) {
            debug_update_time_ticks(ticks);
        }
        if (g_use_stderr) {
            print_to_file(stderr, ticks, pid, tid, level_s, buffer);
        }
        if (g_use_file && debug_file) {
            print_to_file(debug_file, ticks, pid, tid, level_s, buffer);
        }
    }

    free(buffer);
}

const char no_memory[] = "no memory:";

void debug_print_no_memory(const char *msg) {
#ifdef _WIN32
    OutputDebugStringA("No memory");
    OutputDebugStringA(msg);
#endif
    if (g_use_stderr) {
        fwrite(no_memory, 1, strlen(no_memory), stderr);
        fwrite(msg, 1, strlen(msg), stderr);
        fwrite("\n", 1, 1, stderr);
        //fprintf(stderr, "%s\n", msg);
    }
}