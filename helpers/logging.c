#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <unistd.h>

#define LOG_DIR "./logs"
#define MAX_LOG_SIZE 10485760 // 10 MB 
#define LOG_BUFFER_SIZE 8192

typedef enum {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_CRITICAL
} log_level_t;

typedef struct {
    FILE *file;
    log_level_t min_level;
    int enable_colors;
    int enable_timestamps;
    long max_size;
    int rotate_count;
} logger_t;


static logger_t g_logger = {NULL, LOG_INFO, 1, 1, MAX_LOG_SIZE, 5};

const char* log_level_string(log_level_t level) {
    switch(level) {
        case LOG_DEBUG: return "DEBUG";
        case LOG_INFO: return "INFO";
        case LOG_WARNING: return "WARN";
        case LOG_ERROR: return "ERROR";
        case LOG_CRITICAL: return "CRIT";
        default: return "UNKNOWN";
    }
}

const char* log_level_color(log_level_t level) {
    if (!g_logger.enable_colors) return "";
    
    switch(level) {
        case LOG_DEBUG: return "\033[36m";    // Cyan
        case LOG_INFO: return "\033[32m";     // Green
        case LOG_WARNING: return "\033[33m";  // Yellow
        case LOG_ERROR: return "\033[31m";    // Red
        case LOG_CRITICAL: return "\033[35m"; // Magenta
        default: return "\033[0m";
    }
}

void rotate_logs(const char *log_file) {
    char old_name[512], new_name[512];
    
    for (int i = g_logger.rotate_count - 1; i > 0; i--) {
        snprintf(old_name, sizeof(old_name), "%s.%d", log_file, i);
        snprintf(new_name, sizeof(new_name), "%s.%d", log_file, i + 1);
        rename(old_name, new_name);
    }
    
    snprintf(new_name, sizeof(new_name), "%s.1", log_file);
    rename(log_file, new_name);
}

int init_logger(const char *log_file, log_level_t min_level) {
    mkdir(LOG_DIR, 0755);
    
    char full_path[512];
    snprintf(full_path, sizeof(full_path), "%s/%s", LOG_DIR, log_file);
    
    struct stat st;
    if (stat(full_path, &st) == 0 && st.st_size > g_logger.max_size) {
        rotate_logs(full_path);
    }
    
    g_logger.file = fopen(full_path, "a");
    if (!g_logger.file) {
        perror("Failed to open log file");
        return -1;
    }
    
    g_logger.min_level = min_level;
    
    log_message(LOG_INFO, "Logger initialized");
    return 0;
}

void log_message(log_level_t level, const char *format, ...) {
    if (level < g_logger.min_level) return;
    
    char message[LOG_BUFFER_SIZE];
    char timestamp[64];
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);
    
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    
    printf("%s[%s]%s [%s] %s\n", log_level_color(level), log_level_string(level), "\033[0m", timestamp, message);
    

    if (g_logger.file) {
        fprintf(g_logger.file, "[%s] [%s] %s\n", log_level_string(level), timestamp, message);
        fflush(g_logger.file);
    }
}

void close_logger() {
    if (g_logger.file) {
        log_message(LOG_INFO, "Logger shutting down");
        fclose(g_logger.file);
        g_logger.file = NULL;
    }
}

