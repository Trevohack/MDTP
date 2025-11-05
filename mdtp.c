
/****************************************************************************** 
 * MDTP (Markdown Transfer Protocol) 
 * Version: 1.0
 * 
 * This is a full protocol implementation including:
 * - MDTP Server  
 * - MDTP Client
 * - Protocol parser
 * - Request/Response handling
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define MDTP_VERSION "MDTP/1.0"
#define DEFAULT_PORT 8585
#define BUFFER_SIZE 8192
#define MAX_PATH 256
#define MAX_HEADER 1024


typedef enum {
    MDTP_OK = 200,
    MDTP_BAD_REQUEST = 400,
    MDTP_NOT_FOUND = 404,
    MDTP_INTERNAL_ERROR = 500
} mdtp_status_t;


typedef struct {
    char method[16];
    char path[MAX_PATH];
    char version[16];
    char host[256];
    char user_agent[256];
} mdtp_request_t;


typedef struct {
    mdtp_status_t status;
    char content_type[64];
    size_t content_length;
    char *body;
} mdtp_response_t;



void get_timestamp(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *tm_info = gmtime(&now);
    strftime(buffer, size, "%a, %d %b %Y %H:%M:%S GMT", tm_info);
}


const char* get_status_message(mdtp_status_t status) {
    switch(status) {
        case MDTP_OK: return "OK";
        case MDTP_BAD_REQUEST: return "Bad Request";
        case MDTP_NOT_FOUND: return "Not Found";
        case MDTP_INTERNAL_ERROR: return "Internal Server Error";
        default: return "Unknown";
    }
}


char* read_file(const char *path, size_t *length) {
    FILE *file = fopen(path, "r");
    if (!file) return NULL;
    
    fseek(file, 0, SEEK_END);
    *length = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char *content = malloc(*length + 1);
    if (!content) {
        fclose(file);
        return NULL;
    }
    
    fread(content, 1, *length, file);
    content[*length] = '\0';
    fclose(file);
    
    return content;
}


int parse_request(const char *raw_request, mdtp_request_t *req) {
    memset(req, 0, sizeof(mdtp_request_t));
    

    char *line = strdup(raw_request);
    char *saveptr;
    char *token = strtok_r(line, " ", &saveptr);
    
    if (!token) {
        free(line);
        return -1;
    }
    strncpy(req->method, token, sizeof(req->method) - 1);
    
    token = strtok_r(NULL, " ", &saveptr);
    if (!token) {
        free(line);
        return -1;
    }
    strncpy(req->path, token, sizeof(req->path) - 1);
    
    token = strtok_r(NULL, "\r\n", &saveptr);
    if (!token) {
        free(line);
        return -1;
    }
    strncpy(req->version, token, sizeof(req->version) - 1);
    
    free(line);
    

    const char *host_header = strstr(raw_request, "Host: ");
    if (host_header) {
        sscanf(host_header, "Host: %255s", req->host);
    }
    
    const char *ua_header = strstr(raw_request, "User-Agent: ");
    if (ua_header) {
        sscanf(ua_header, "User-Agent: %255[^\r\n]", req->user_agent);
    }
    
    return 0;
}


char* build_response(mdtp_response_t *resp, size_t *total_length) {
    char timestamp[64];
    get_timestamp(timestamp, sizeof(timestamp));
    

    char header[MAX_HEADER];
    int header_len = snprintf(header, sizeof(header),
        "%s %d %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "Date: %s\r\n"
        "Server: MDTP-Server/1.0\r\n"
        "\r\n",
        MDTP_VERSION, resp->status, get_status_message(resp->status),
        resp->content_type,
        resp->content_length,
        timestamp
    );
    
    *total_length = header_len + resp->content_length;
    char *full_response = malloc(*total_length);
    if (!full_response) return NULL;
    
    memcpy(full_response, header, header_len);
    if (resp->body && resp->content_length > 0) {
        memcpy(full_response + header_len, resp->body, resp->content_length);
    }
    
    return full_response;
}


void handle_client(int client_sock) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
    
    if (bytes_read <= 0) {
        close(client_sock);
        return;
    }
    
    buffer[bytes_read] = '\0';
    
    mdtp_request_t req;
    if (parse_request(buffer, &req) < 0) {
        const char *error_resp = 
            "MDTP/1.0 400 Bad Request\r\n"
            "Content-Type: text/markdown\r\n\r\n"
            "# 400 - Bad Request\n\nInvalid MDTP request.";
        send(client_sock, error_resp, strlen(error_resp), 0);
        close(client_sock);
        return;
    }
    
    printf("[%s] %s %s\n", req.host, req.method, req.path);

    char filepath[MAX_PATH];
    snprintf(filepath, sizeof(filepath), ".%s", req.path);
    
    if (strcmp(req.path, "/") == 0) {
        strcpy(filepath, "./index.md");
    }
    
    mdtp_response_t resp;
    size_t content_length;
    char *content = read_file(filepath, &content_length);
    
    if (content) {
        resp.status = MDTP_OK;
        strcpy(resp.content_type, "text/markdown");
        resp.content_length = content_length;
        resp.body = content;
    } else {
        const char *not_found_body = "# 404 - Not Found\n\nThe requested document was not found on this server.";
        resp.status = MDTP_NOT_FOUND;
        strcpy(resp.content_type, "text/markdown");
        resp.content_length = strlen(not_found_body);
        resp.body = strdup(not_found_body);
    }
    
    size_t response_length;
    char *response = build_response(&resp, &response_length);
    
    if (response) {
        send(client_sock, response, response_length, 0);
        free(response);
    }
    
    free(resp.body);
    close(client_sock);
}

void start_server(int port) {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("Socket creation failed");
        exit(1);
    }
    
    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    // Bind
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    
    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(1);
    }
    
    if (listen(server_sock, 10) < 0) {
        perror("Listen failed");
        exit(1);
    }
    
    printf("[MDTP] MDTP Server running on port %d\n", port);
    printf("[MDTP] Protocol: %s\n", MDTP_VERSION);
    printf("[MDTP] Serving Markdown documents from current directory\n\n");
    
    while (1) {
        client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_len);
        if (client_sock < 0) {
            perror("Accept failed");
            continue;
        }
        
        handle_client(client_sock);
    }
    
    close(server_sock);
}


char* mdtp_fetch(const char *host, int port, const char *path) {
    int sock;
    struct sockaddr_in server_addr;
    char request[BUFFER_SIZE];
    char response[BUFFER_SIZE * 4];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return NULL;
    }
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, host, &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        close(sock);
        return NULL;
    }
    
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sock);
        return NULL;
    }
    
    snprintf(request, sizeof(request),
        "GET %s %s\r\n"
        "Host: %s\r\n"
        "User-Agent: MDTP-Client/1.0\r\n"
        "Accept: text/markdown\r\n"
        "\r\n",
        path, MDTP_VERSION, host
    );
    
    send(sock, request, strlen(request), 0);

    ssize_t bytes_read = recv(sock, response, sizeof(response) - 1, 0);
    close(sock);
    
    if (bytes_read <= 0) {
        return NULL;
    }
    
    response[bytes_read] = '\0';

    char *body = strstr(response, "\r\n\r\n");
    if (body) {
        body += 4;
        return strdup(body);
    }
    
    return NULL;
}


void print_usage(const char *prog) {
    printf("MDTP - Markdown Transfer Protocol v1.0\n\n");
    printf("Usage:\n");
    printf("  %s server [port]           Start MDTP server (default port: 8585)\n", prog);
    printf("  %s client <host> <path>    Fetch document via MDTP\n", prog);
    printf("\nExamples:\n");
    printf("  %s server 8585\n", prog);
    printf("  %s client 127.0.0.1 /index.md\n", prog);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    if (strcmp(argv[1], "server") == 0) {
        int port = DEFAULT_PORT;
        if (argc > 2) {
            port = atoi(argv[2]);
        }
        start_server(port);
    }
    else if (strcmp(argv[1], "client") == 0) {
        if (argc < 4) {
            printf("Usage: %s client <host> <path>\n", argv[0]);
            return 1;
        }
        
        char *content = mdtp_fetch(argv[2], DEFAULT_PORT, argv[3]);
        if (content) {
            printf("%s\n", content);
            free(content);
        } else {
            printf("Failed to fetch document\n");
            return 1;
        }
    }
    else {
        print_usage(argv[0]);
        return 1;
    }
    
    return 0;
}
