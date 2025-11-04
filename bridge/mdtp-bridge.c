#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BRIDGE_PORT 9999
#define BUFFER_SIZE 8192
#define MDTP_VERSION "MDTP/1.0"



const char* HTML_TEMPLATE_HEADER = 
"HTTP/1.1 200 OK\r\n"
"Content-Type: text/html\r\n"
"Connection: close\r\n\r\n"
"<!DOCTYPE html>\n"
"<html>\n"
"<head>\n"
"  <meta charset=\"UTF-8\">\n"
"  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
"  <title>MDTP Browser</title>\n"
"  <style>\n"
"    body {\n"
"      font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;\n"
"      max-width: 850px;\n"
"      margin: 0 auto;\n"
"      padding: 2rem;\n"
"      line-height: 1.7;\n"
"      color: #2c3e50;\n"
"      background: linear-gradient(135deg, #f5f7fa 0%, #c3cfe2 100%);\n"
"      min-height: 100vh;\n"
"    }\n"
"    .container {\n"
"      background: white;\n"
"      padding: 2.5rem;\n"
"      border-radius: 12px;\n"
"      box-shadow: 0 10px 40px rgba(0,0,0,0.1);\n"
"    }\n"
"    .mdtp-indicator {\n"
"      position: fixed;\n"
"      top: 20px;\n"
"      right: 20px;\n"
"      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);\n"
"      color: white;\n"
"      padding: 10px 20px;\n"
"      border-radius: 25px;\n"
"      font-size: 13px;\n"
"      font-weight: 600;\n"
"      box-shadow: 0 4px 15px rgba(102, 126, 234, 0.4);\n"
"      z-index: 1000;\n"
"    }\n"
"    h1 {\n"
"      font-size: 2.5rem;\n"
"      color: #1a202c;\n"
"      border-bottom: 4px solid #667eea;\n"
"      padding-bottom: 0.5rem;\n"
"      margin-top: 0;\n"
"    }\n"
"    h2 {\n"
"      font-size: 2rem;\n"
"      color: #2d3748;\n"
"      margin-top: 2.5rem;\n"
"      border-bottom: 2px solid #e2e8f0;\n"
"      padding-bottom: 0.4rem;\n"
"    }\n"
"    h3 {\n"
"      font-size: 1.5rem;\n"
"      color: #4a5568;\n"
"      margin-top: 1.8rem;\n"
"    }\n"
"    p { margin: 1.2rem 0; }\n"
"    a {\n"
"      color: #667eea;\n"
"      text-decoration: none;\n"
"      border-bottom: 2px solid transparent;\n"
"      transition: all 0.3s ease;\n"
"      font-weight: 500;\n"
"    }\n"
"    a:hover {\n"
"      border-bottom: 2px solid #667eea;\n"
"      color: #5a67d8;\n"
"    }\n"
"    code {\n"
"      background: #f7fafc;\n"
"      padding: 3px 8px;\n"
"      border-radius: 4px;\n"
"      font-family: 'Courier New', monospace;\n"
"      font-size: 0.9em;\n"
"      color: #e53e3e;\n"
"      border: 1px solid #e2e8f0;\n"
"    }\n"
"    pre {\n"
"      background: #1a202c;\n"
"      color: #f7fafc;\n"
"      padding: 1.5rem;\n"
"      border-radius: 8px;\n"
"      overflow-x: auto;\n"
"      margin: 1.5rem 0;\n"
"      box-shadow: 0 4px 6px rgba(0,0,0,0.1);\n"
"    }\n"
"    pre code {\n"
"      background: none;\n"
"      color: inherit;\n"
"      padding: 0;\n"
"      border: none;\n"
"    }\n"
"    ul, ol {\n"
"      margin: 1rem 0;\n"
"      padding-left: 2rem;\n"
"    }\n"
"    li {\n"
"      margin: 0.6rem 0;\n"
"      line-height: 1.6;\n"
"    }\n"
"    blockquote {\n"
"      border-left: 4px solid #667eea;\n"
"      padding-left: 1.5rem;\n"
"      margin: 1.5rem 0;\n"
"      color: #4a5568;\n"
"      font-style: italic;\n"
"      background: #f7fafc;\n"
"      padding: 1rem 1rem 1rem 1.5rem;\n"
"      border-radius: 0 8px 8px 0;\n"
"    }\n"
"    hr {\n"
"      border: none;\n"
"      border-top: 2px solid #e2e8f0;\n"
"      margin: 2.5rem 0;\n"
"    }\n"
"    strong { color: #1a202c; font-weight: 600; }\n"
"    em { color: #4a5568; }\n"
"  </style>\n"
"</head>\n"
"<body>\n"
"  <div class=\"mdtp-indicator\">MDTP/1.0</div>\n"
"  <div class=\"container\">\n";

const char* HTML_TEMPLATE_FOOTER = 
"  </div>\n"
"</body>\n"
"</html>\n";


void markdown_to_html(const char *markdown, char *html, size_t html_size) {
    const char *src = markdown;
    char *dst = html;
    size_t remaining = html_size - 1;
    int in_code_block = 0;
    int in_list = 0;
    
    while (*src && remaining > 100) {
        if (strncmp(src, "```", 3) == 0) {
            in_code_block = !in_code_block;
            if (in_code_block) {
                int written = snprintf(dst, remaining, "<pre><code>");
                dst += written;
                remaining -= written;
                src += 3;
                while (*src && *src != '\n') src++; 
                if (*src) src++;
            } else {
                int written = snprintf(dst, remaining, "</code></pre>\n");
                dst += written;
                remaining -= written;
                src += 3;
            }
            continue;
        }
        
        if (in_code_block) {
            *dst++ = *src++;
            remaining--;
            continue;
        }
        
        if (*src == '#' && (src == markdown || *(src-1) == '\n')) {
            int level = 0;
            while (*src == '#' && level < 6) {
                level++;
                src++;
            }
            while (*src == ' ') src++;
            
            int written = snprintf(dst, remaining, "<h%d>", level);
            dst += written;
            remaining -= written;
            
            while (*src && *src != '\n') {
                *dst++ = *src++;
                remaining--;
            }
            
            written = snprintf(dst, remaining, "</h%d>\n", level);
            dst += written;
            remaining -= written;
            continue;
        }
        
        if (strncmp(src, "**", 2) == 0) {
            src += 2;
            int written = snprintf(dst, remaining, "<strong>");
            dst += written;
            remaining -= written;
            
            while (*src && strncmp(src, "**", 2) != 0) {
                *dst++ = *src++;
                remaining--;
            }
            if (*src) src += 2;
            
            written = snprintf(dst, remaining, "</strong>");
            dst += written;
            remaining -= written;
            continue;
        }

        if (*src == '*' && *(src+1) != '*') {
            src++;
            int written = snprintf(dst, remaining, "<em>");
            dst += written;
            remaining -= written;
            
            while (*src && *src != '*') {
                *dst++ = *src++;
                remaining--;
            }
            if (*src) src++;
            
            written = snprintf(dst, remaining, "</em>");
            dst += written;
            remaining -= written;
            continue;
        }
        
        if (*src == '`') {
            src++;
            int written = snprintf(dst, remaining, "<code>");
            dst += written;
            remaining -= written;
            
            while (*src && *src != '`') {
                *dst++ = *src++;
                remaining--;
            }
            if (*src) src++;
            
            written = snprintf(dst, remaining, "</code>");
            dst += written;
            remaining -= written;
            continue;
        }
    
        if (*src == '[') {
            src++;
            char link_text[256] = {0};
            char link_url[512] = {0};
            int i = 0;
            
            while (*src && *src != ']' && i < 255) {
                link_text[i++] = *src++;
            }
            if (*src == ']') src++;
            if (*src == '(') {
                src++;
                i = 0;
                while (*src && *src != ')' && i < 511) {
                    link_url[i++] = *src++;
                }
                if (*src == ')') src++;
                
                if (strncmp(link_url, "mdtp://", 7) == 0) {
                    int written = snprintf(dst, remaining, 
                        "<a href=\"http://127.0.0.1:9999/%s\">%s</a>",
                        link_url + 7, link_text);
                    dst += written;
                    remaining -= written;
                } else {
                    int written = snprintf(dst, remaining, 
                        "<a href=\"%s\">%s</a>", link_url, link_text);
                    dst += written;
                    remaining -= written;
                }
            }
            continue;
        }
        
        if ((*src == '-' || *src == '*') && (src == markdown || *(src-1) == '\n')) {
            if (!in_list) {
                int written = snprintf(dst, remaining, "<ul>\n");
                dst += written;
                remaining -= written;
                in_list = 1;
            }
            src++;
            while (*src == ' ') src++;
            
            int written = snprintf(dst, remaining, "<li>");
            dst += written;
            remaining -= written;
            
            while (*src && *src != '\n') {
                *dst++ = *src++;
                remaining--;
            }
            
            written = snprintf(dst, remaining, "</li>\n");
            dst += written;
            remaining -= written;
            continue;
        }
        
        if (in_list && *src == '\n' && *(src+1) != '-' && *(src+1) != '*') {
            int written = snprintf(dst, remaining, "</ul>\n");
            dst += written;
            remaining -= written;
            in_list = 0;
        }
        
        if (strncmp(src, "---", 3) == 0) {
            int written = snprintf(dst, remaining, "<hr>\n");
            dst += written;
            remaining -= written;
            src += 3;
            while (*src == '-') src++;
            continue;
        }
        
        if (*src == '\n') {
            if (*(src+1) == '\n') {
                int written = snprintf(dst, remaining, "</p>\n<p>");
                dst += written;
                remaining -= written;
                src += 2;
                continue;
            } else {
                int written = snprintf(dst, remaining, "<br>\n");
                dst += written;
                remaining -= written;
                src++;
                continue;
            }
        }
        
        *dst++ = *src++;
        remaining--;
    }
    
    if (in_list) {
        snprintf(dst, remaining, "</ul>\n");
    }
    
    *dst = '\0';
}

char* fetch_mdtp(const char *host, int port, const char *path) {
    int sock;
    struct sockaddr_in server_addr;
    char request[1024];
    char response[BUFFER_SIZE * 2];
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return NULL;
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, host, &server_addr.sin_addr);
    
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        close(sock);
        return NULL;
    }
    
    snprintf(request, sizeof(request),
        "GET %s %s\r\n"
        "Host: %s\r\n"
        "User-Agent: MDTP-Bridge/1.0\r\n"
        "\r\n",
        path, MDTP_VERSION, host
    );
    
    send(sock, request, strlen(request), 0);
    ssize_t bytes = recv(sock, response, sizeof(response) - 1, 0);
    close(sock);
    if (bytes <= 0) return NULL;
    response[bytes] = '\0';
    char *body = strstr(response, "\r\n\r\n");
    if (body) {
        body += 4;
        return strdup(body);
    }
    
    return NULL;
}

void handle_http_request(int client_sock) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
    
    if (bytes <= 0) {
        close(client_sock);
        return;
    }
    
    buffer[bytes] = '\0';
    char method[16], path[512];
    sscanf(buffer, "%s %s", method, path);
    printf("[HTTP Bridge] %s %s\n", method, path);

    if (strcmp(path, "/") == 0) {
        const char *home = 
            "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
            "<html><head><title>MDTP Bridge</title></head>"
            "<body style='font-family:sans-serif;max-width:600px;margin:50px auto'>"
            "<h1>🌐 MDTP HTTP Bridge</h1>"
            "<p>Access MDTP sites through your browser!</p>"
            "<h2>Try these:</h2><ul>"
            "<li><a href='/127.0.0.1:8585/index.md'>index.md</a></li>"
            "<li><a href='/127.0.0.1:8585/about.md'>about.md</a></li>"
            "</ul></body></html>";
        send(client_sock, home, strlen(home), 0);
        close(client_sock);
        return;
    }

    char host[256] = "127.0.0.1";
    int port = 8585;
    char mdtp_path[512] = "/index.md";
    
    if (path[0] == '/') {
        char *p = path + 1;
        char *colon = strchr(p, ':');
        char *slash = strchr(p, '/');
        
        if (colon && slash) {
            int host_len = colon - p;
            strncpy(host, p, host_len);
            host[host_len] = '\0';
            port = atoi(colon + 1);
            strcpy(mdtp_path, slash);
        } else if (slash) {
            strcpy(mdtp_path, slash);
        }
    }
    
    char *markdown = fetch_mdtp(host, port, mdtp_path);
    
    if (markdown) {
        char html_content[BUFFER_SIZE * 2];
        markdown_to_html(markdown, html_content, sizeof(html_content));
        
        send(client_sock, HTML_TEMPLATE_HEADER, strlen(HTML_TEMPLATE_HEADER), 0);
        send(client_sock, html_content, strlen(html_content), 0);
        send(client_sock, HTML_TEMPLATE_FOOTER, strlen(HTML_TEMPLATE_FOOTER), 0);
        
        free(markdown);
    } else {
        const char *error = 
            "HTTP/1.1 500 Error\r\nContent-Type: text/html\r\n\r\n"
            "<html><body><h1>MDTP Error</h1>"
            "<p>Failed to fetch from MDTP server</p></body></html>";
        send(client_sock, error, strlen(error), 0);
    }
    
    close(client_sock);
}


int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("Socket failed");
        return 1;
    }
    
    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(BRIDGE_PORT);
    
    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        return 1;
    }
    
    listen(server_sock, 10);
    
    printf("╔═══════════════════════════════════════════════════════╗\n");
    printf("║       MDTP HTTP Bridge Server - Running!            ║\n");
    printf("╠═══════════════════════════════════════════════════════╣\n");
    printf("║  Browse MDTP sites in your regular web browser!      ║\n");
    printf("║                                                       ║\n");
    printf("║  Open: http://127.0.0.1:9999/                        ║\n");
    printf("║                                                       ║\n");
    printf("║  Direct access:                                       ║\n");
    printf("║  http://127.0.0.1:9999/127.0.0.1:8585/index.md       ║\n");
    printf("╚═══════════════════════════════════════════════════════╝\n\n");
    
    while (1) {
        client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_len);
        if (client_sock >= 0) {
            handle_http_request(client_sock);
        }
    }
    
    close(server_sock);
    return 0;
} 
