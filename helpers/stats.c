#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#define STATS_FILE "./logs/mdtp_stats.json"
#define MAX_URLS 1000
#define MAX_IPS 1000

typedef struct {
    char url[256];
    long count;
    long total_time_ms;
} url_stat_t;

typedef struct {
    char ip[64];
    long requests;
    time_t first_seen;
    time_t last_seen;
} ip_stat_t;

typedef struct {
    long total_requests;
    long successful_requests;
    long failed_requests;
    long requests_200;
    long requests_404;
    long requests_500;

    long total_response_time_ms;
    long min_response_time_ms;
    long max_response_time_ms;
    
    long bytes_sent;
    long bytes_received;
    
    url_stat_t *top_urls;
    int url_count;
    
    ip_stat_t *ips;
    int ip_count;

    time_t start_time;
    time_t last_request_time;
    
    pthread_mutex_t lock;
} server_stats_t;

static server_stats_t g_stats = {0};

void init_stats() {
    memset(&g_stats, 0, sizeof(g_stats));
    g_stats.start_time = time(NULL);
    g_stats.min_response_time_ms = LONG_MAX;
    g_stats.top_urls = calloc(MAX_URLS, sizeof(url_stat_t));
    g_stats.ips = calloc(MAX_IPS, sizeof(ip_stat_t));
    pthread_mutex_init(&g_stats.lock, NULL);
    
    log_message(LOG_INFO, "Statistics system initialized");
}

void record_request(const char *url, const char *ip, int status_code, 
                   long response_time_ms, long bytes_sent) {
    pthread_mutex_lock(&g_stats.lock);
    
    g_stats.total_requests++;
    g_stats.last_request_time = time(NULL);
    g_stats.bytes_sent += bytes_sent;
    g_stats.total_response_time_ms += response_time_ms;
    
    if (response_time_ms < g_stats.min_response_time_ms)
        g_stats.min_response_time_ms = response_time_ms;
    if (response_time_ms > g_stats.max_response_time_ms)
        g_stats.max_response_time_ms = response_time_ms;
   
    if (status_code == 200) {
        g_stats.requests_200++;
        g_stats.successful_requests++;
    } else if (status_code == 404) {
        g_stats.requests_404++;
        g_stats.failed_requests++;
    } else if (status_code >= 500) {
        g_stats.requests_500++;
        g_stats.failed_requests++;
    }
    
    int found = 0;
    for (int i = 0; i < g_stats.url_count; i++) {
        if (strcmp(g_stats.top_urls[i].url, url) == 0) {
            g_stats.top_urls[i].count++;
            g_stats.top_urls[i].total_time_ms += response_time_ms;
            found = 1;
            break;
        }
    }
    if (!found && g_stats.url_count < MAX_URLS) {
        strncpy(g_stats.top_urls[g_stats.url_count].url, url, 255);
        g_stats.top_urls[g_stats.url_count].count = 1;
        g_stats.top_urls[g_stats.url_count].total_time_ms = response_time_ms;
        g_stats.url_count++;
    }
    
    found = 0;
    for (int i = 0; i < g_stats.ip_count; i++) {
        if (strcmp(g_stats.ips[i].ip, ip) == 0) {
            g_stats.ips[i].requests++;
            g_stats.ips[i].last_seen = time(NULL);
            found = 1;
            break;
        }
    }
    if (!found && g_stats.ip_count < MAX_IPS) {
        strncpy(g_stats.ips[g_stats.ip_count].ip, ip, 63);
        g_stats.ips[g_stats.ip_count].requests = 1;
        g_stats.ips[g_stats.ip_count].first_seen = time(NULL);
        g_stats.ips[g_stats.ip_count].last_seen = time(NULL);
        g_stats.ip_count++;
    }
    
    pthread_mutex_unlock(&g_stats.lock);
}

void print_stats() {
    pthread_mutex_lock(&g_stats.lock);
    
    time_t now = time(NULL);
    long uptime = now - g_stats.start_time;
    long avg_response_time = g_stats.total_requests > 0 ? 
        g_stats.total_response_time_ms / g_stats.total_requests : 0;
    
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║             MDTP SERVER STATISTICS                         ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║ Uptime: %ld seconds (%ld min)                            \n", uptime, uptime/60);
    printf("║                                                            ║\n");
    printf("║ REQUESTS:                                                  ║\n");
    printf("║   Total:      %-10ld  Success: %-10ld            ║\n", 
           g_stats.total_requests, g_stats.successful_requests);
    printf("║   Failed:     %-10ld  Success Rate: %.1f%%         ║\n",
           g_stats.failed_requests,
           g_stats.total_requests > 0 ? 
               (float)g_stats.successful_requests / g_stats.total_requests * 100 : 0);
    printf("║                                                            ║\n");
    printf("║ STATUS CODES:                                              ║\n");
    printf("║   200 OK:     %-10ld  404 Not Found: %-10ld   ║\n",
           g_stats.requests_200, g_stats.requests_404);
    printf("║   500 Error:  %-10ld                                 ║\n",
           g_stats.requests_500);
    printf("║                                                            ║\n");
    printf("║ PERFORMANCE:                                               ║\n");
    printf("║   Avg Response: %ld ms                                     ║\n", avg_response_time);
    printf("║   Min Response: %ld ms                                     ║\n", 
           g_stats.min_response_time_ms == LONG_MAX ? 0 : g_stats.min_response_time_ms);
    printf("║   Max Response: %ld ms                                     ║\n", 
           g_stats.max_response_time_ms);
    printf("║                                                            ║\n");
    printf("║ TRAFFIC:                                                   ║\n");
    printf("║   Bytes Sent:     %.2f MB                                  ║\n",
           (float)g_stats.bytes_sent / 1024 / 1024);
    printf("║   Requests/min:   %.1f                                     ║\n",
           uptime > 0 ? (float)g_stats.total_requests / (uptime / 60.0) : 0);
    printf("║                                                            ║\n");
    printf("║ TOP 5 URLS:                                                ║\n");
    
    // Sort URLs by count
    for (int i = 0; i < g_stats.url_count && i < 5; i++) {
        for (int j = i + 1; j < g_stats.url_count; j++) {
            if (g_stats.top_urls[j].count > g_stats.top_urls[i].count) {
                url_stat_t temp = g_stats.top_urls[i];
                g_stats.top_urls[i] = g_stats.top_urls[j];
                g_stats.top_urls[j] = temp;
            }
        }
        printf("║   %d. %-40s (%ld hits)   ║\n", 
               i+1, g_stats.top_urls[i].url, g_stats.top_urls[i].count);
    }
    
    printf("║                                                            ║\n");
    printf("║ UNIQUE VISITORS: %-10d                               ║\n", g_stats.ip_count);
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    pthread_mutex_unlock(&g_stats.lock);
}

void save_stats() {
    pthread_mutex_lock(&g_stats.lock);
    
    FILE *f = fopen(STATS_FILE, "w");
    if (!f) {
        log_message(LOG_ERROR, "Failed to save statistics");
        pthread_mutex_unlock(&g_stats.lock);
        return;
    }
    
    fprintf(f, "{\n");
    fprintf(f, "  \"total_requests\": %ld,\n", g_stats.total_requests);
    fprintf(f, "  \"successful_requests\": %ld,\n", g_stats.successful_requests);
    fprintf(f, "  \"failed_requests\": %ld,\n", g_stats.failed_requests);
    fprintf(f, "  \"requests_200\": %ld,\n", g_stats.requests_200);
    fprintf(f, "  \"requests_404\": %ld,\n", g_stats.requests_404);
    fprintf(f, "  \"requests_500\": %ld,\n", g_stats.requests_500);
    fprintf(f, "  \"avg_response_time_ms\": %ld,\n", 
            g_stats.total_requests > 0 ? g_stats.total_response_time_ms / g_stats.total_requests : 0);
    fprintf(f, "  \"bytes_sent\": %ld,\n", g_stats.bytes_sent);
    fprintf(f, "  \"uptime\": %ld,\n", time(NULL) - g_stats.start_time);
    fprintf(f, "  \"unique_visitors\": %d\n", g_stats.ip_count);
    fprintf(f, "}\n");
    
    fclose(f);
    log_message(LOG_INFO, "Statistics saved to %s", STATS_FILE);
    
    pthread_mutex_unlock(&g_stats.lock);
}
