#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CONFIG_FILE "./mdtp.conf"

typedef struct {
    char root_dir[512];
    int port;
    int max_connections;
    int timeout_seconds;
    int enable_logging;
    int log_level;
    char index_file[256];
    int enable_stats;
    int stats_interval;
    int enable_cache;
    long max_file_size;
} mdtp_config_t;

static mdtp_config_t g_config = {
    .root_dir = "./content",
    .port = 8585,
    .max_connections = 100,
    .timeout_seconds = 30,
    .enable_logging = 1,
    .log_level = LOG_INFO,
    .index_file = "index.md",
    .enable_stats = 1,
    .stats_interval = 300,
    .enable_cache = 1,
    .max_file_size = 10485760
};

int load_config(const char *config_file) {
    FILE *f = fopen(config_file ? config_file : CONFIG_FILE, "r");
    if (!f) {
        log_message(LOG_WARNING, "Config file not found, using defaults");
        return 0;
    }
    
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        if (line[0] == '#' || line[0] == '\n') continue;
        
        char key[128], value[384];
        if (sscanf(line, "%127s = %383[^\n]", key, value) == 2) {
            char *v = value;
            if (v[0] == '"') {
                v++;
                char *end = strchr(v, '"');
                if (end) *end = '\0';
            }
            
            if (strcmp(key, "root_dir") == 0) {
                strncpy(g_config.root_dir, v, sizeof(g_config.root_dir) - 1);
            } else if (strcmp(key, "port") == 0) {
                g_config.port = atoi(v);
            } else if (strcmp(key, "max_connections") == 0) {
                g_config.max_connections = atoi(v);
            } else if (strcmp(key, "timeout") == 0) {
                g_config.timeout_seconds = atoi(v);
            } else if (strcmp(key, "enable_logging") == 0) {
                g_config.enable_logging = atoi(v);
            } else if (strcmp(key, "log_level") == 0) {
                if (strcmp(v, "DEBUG") == 0) g_config.log_level = LOG_DEBUG;
                else if (strcmp(v, "INFO") == 0) g_config.log_level = LOG_INFO;
                else if (strcmp(v, "WARNING") == 0) g_config.log_level = LOG_WARNING;
                else if (strcmp(v, "ERROR") == 0) g_config.log_level = LOG_ERROR;
            } else if (strcmp(key, "index_file") == 0) {
                strncpy(g_config.index_file, v, sizeof(g_config.index_file) - 1);
            } else if (strcmp(key, "enable_stats") == 0) {
                g_config.enable_stats = atoi(v);
            } else if (strcmp(key, "max_file_size") == 0) {
                g_config.max_file_size = atol(v);
            }
        }
    }
    
    fclose(f);
    log_message(LOG_INFO, "Configuration loaded from %s", config_file ? config_file : CONFIG_FILE);
    return 1;
}

void save_default_config() {
    FILE *f = fopen(CONFIG_FILE, "w");
    if (!f) {
        log_message(LOG_ERROR, "Failed to create default config");
        return;
    }
    
    fprintf(f, "# MDTP Server Configuration\n\n");
    fprintf(f, "# Server settings\n");
    fprintf(f, "port = 8585\n");
    fprintf(f, "root_dir = \"./content\"\n");
    fprintf(f, "index_file = \"index.md\"\n");
    fprintf(f, "max_connections = 100\n");
    fprintf(f, "timeout = 30\n");
    fprintf(f, "max_file_size = 10485760\n\n");
    fprintf(f, "# Logging\n");
    fprintf(f, "enable_logging = 1\n");
    fprintf(f, "log_level = \"INFO\"  # DEBUG, INFO, WARNING, ERROR\n\n");
    fprintf(f, "# Statistics\n");
    fprintf(f, "enable_stats = 1\n");
    fprintf(f, "stats_interval = 300\n\n");
    fprintf(f, "# Performance\n");
    fprintf(f, "enable_cache = 1\n");
    
    fclose(f);
    log_message(LOG_INFO, "Default configuration created: %s", CONFIG_FILE);
}

void print_config() {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║             MDTP SERVER CONFIGURATION                      ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║ Port:              %-10d                              ║\n", g_config.port);
    printf("║ Root Directory:    %-40s ║\n", g_config.root_dir);
    printf("║ Index File:        %-40s ║\n", g_config.index_file);
    printf("║ Max Connections:   %-10d                              ║\n", g_config.max_connections);
    printf("║ Timeout:           %-10d seconds                       ║\n", g_config.timeout_seconds);
    printf("║ Max File Size:     %.2f MB                               ║\n", 
           (float)g_config.max_file_size / 1024 / 1024);
    printf("║ Logging:           %s                                     ║\n", 
           g_config.enable_logging ? "Enabled" : "Disabled");
    printf("║ Statistics:        %s                                     ║\n",
           g_config.enable_stats ? "Enabled" : "Disabled");
    printf("║ Cache:             %s                                     ║\n",
           g_config.enable_cache ? "Enabled" : "Disabled");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\n");
}

mdtp_config_t* get_config() {
    return &g_config;
}
